// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// ebgstudio's writer: the viewer's edits of a domain's surface groups, written into the model's files as text.
//
// A surface the user clicked is written by its label: label("x"), narrowed to a piece with between = (a, b). A shape
// (or a part of one) without a label of its own gets one first: the writer asks for a name, through the viewer, and
// writes .label("name") (or .label(part = "name")) after the shape's call in the file that makes it. Only the edited
// domain's block is rewritten: its subdomain lines and unchanged groups are kept as written, comments included; a
// changed or new group is written in canonical form, keeping the properties it had. A model without the domain gets
// it appended to its setup file. Every write keeps the files' previous text, for undo and redo, no write goes over a
// file that changed on disk since the model was read, and a write changes all its files or none.

#ifndef EBGEOMETRY_STUDIO_WRITER_HPP
#define EBGEOMETRY_STUDIO_WRITER_HPP

#include "EBGeometry.hpp"
#include "Source/EBGeometry_Language.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace Studio {

using namespace EBGeometry;

// One replacement of a byte range of a file.
struct Edit
{
  std::string m_file;
  uint32_t    m_begin = 0;
  uint32_t    m_end   = 0;
  std::string m_text;
};

// Files' texts, nothing for a file that does not exist.
using Texts = std::vector<std::pair<std::string, std::optional<std::string>>>;

// A write: each file's text before and after (a file the write made has none before: undone, it is removed).
struct Write
{
  Texts m_before;
  Texts m_after;
};

// A file's text on disk, or nothing.
inline std::optional<std::string>
readFile(const std::string& a_path)
{
  std::ifstream in(a_path, std::ios::binary);

  if (!in) {
    return std::nullopt;
  }

  std::stringstream text;

  text << in.rdbuf();

  return text.str();
}

// A name made valid in the language: letters, digits and underscores, not starting with a digit, not reserved.
inline std::string
validName(const std::string& a_name)
{
  std::string name;

  for (const char c : a_name) {
    name += (std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_') ? c : '_';
  }

  if (name.empty() || std::isdigit(static_cast<unsigned char>(name[0])) != 0) {
    name = "_" + name;
  }

  return Language::Syntax::isReserved(name) ? name + "_" : name;
}

template <class T>
class Writer
{
public:
  // The writer of a model read from a path (a .ebg file or a model folder).
  explicit Writer(std::string a_path) : m_path(std::move(a_path))
  {}

  // The model as last read: what edits are written against.
  void
  attach(const Language::Model<T>& a_model)
  {
    m_model = &a_model;
  }

  // What the title shows ("" for nothing).
  const std::string&
  note() const noexcept
  {
    return m_note;
  }

  // The viewer's edit of the active domain (or of surface groups with no domain yet): written, or a question.
  std::string
  edited(const RegionNames& a_names, const DomainSetup& a_setup)
  {
    m_pending = Pending{};

    m_pending.m_setup = a_setup;
    m_pending.m_names.clear();

    if (a_names.getNumDomains() > 0U) {
      m_pending.m_domain = a_names.domainName(a_names.getActiveDomain());
    }

    // The groups' names, made names of the language and kept apart: from each other and from the subdomains'.
    std::vector<std::string> taken;

    for (uint32_t d = 0; d < m_model->getNumDomains(); d++) {
      const Language::ModelDomain& domain = m_model->domain(d);

      if (domain.name() == m_pending.m_domain) {
        for (uint32_t s = 0; s < domain.getNumSubdomains(); s++) {
          taken.push_back(domain.subdomainName(s));
        }
      }
    }

    for (uint32_t g = 0; g < a_setup.getNumSurfaceGroups(); g++) {
      std::string name = validName(a_names.surfaceGroup(g, false));

      while (std::find(taken.begin(), taken.end(), name) != taken.end()) {
        name += "_2";
      }

      taken.push_back(name);
      m_pending.m_names.push_back(name);
    }

    // Every clicked surface that has no label to write it by needs a name.
    for (uint32_t i = 0; i < a_setup.getNumSelectors(); i++) {
      const SurfaceSelector& s = a_setup.getSelector(i);

      if (s.m_surface != SurfaceSelector::Any && this->labelFor(s).empty()) {
        const uint32_t leaf = m_model->index().getLeafOfSurface(s.m_surface);
        const uint32_t part = s.m_part;

        const bool known = std::any_of(m_pending.m_needs.begin(), m_pending.m_needs.end(), [&](const Need& n) {
          return n.m_leaf == leaf && n.m_part == part;
        });

        if (!known) {
          m_pending.m_needs.push_back(Need{leaf, part, ""});
        }
      }
    }

    m_pending.m_active = true;

    return this->next();
  }

  // The name typed for the last question ("" cancels the edit, which then stays in the viewer only).
  std::string
  named(const std::string& a_name)
  {
    if (!m_pending.m_active) {
      return "";
    }

    if (a_name.empty()) {
      m_pending = Pending{};
      m_note    = "the edit was not written (a name was cancelled)";

      return "";
    }

    if (m_pending.m_domain.empty()) {
      m_pending.m_domain = validName(a_name);
    }
    else {
      m_pending.m_needs[m_pending.m_asked].m_name = validName(a_name);
    }

    return this->next();
  }

  // Undo the last write; false when there is none, or a file changed since it.
  bool
  undo()
  {
    return this->step(m_undo, m_redo, true);
  }

  // Redo the last write undone.
  bool
  redo()
  {
    return this->step(m_redo, m_undo, false);
  }

private:
  // A shape or part that needs a name, and the name given.
  struct Need
  {
    uint32_t    m_leaf = 0;
    uint32_t    m_part = SurfaceSelector::Any;
    std::string m_name;
  };

  // An edit waiting for names.
  struct Pending
  {
    bool                     m_active = false;
    std::string              m_domain;
    DomainSetup              m_setup;
    std::vector<std::string> m_names;
    std::vector<Need>        m_needs;
    size_t                   m_asked = 0;
  };

  std::string               m_path;
  const Language::Model<T>* m_model = nullptr;
  std::string               m_note;
  Pending                   m_pending;
  std::vector<Write>        m_undo;
  std::vector<Write>        m_redo;

  // The next question, or "" once the edit is written.
  std::string
  next()
  {
    if (m_pending.m_domain.empty()) {
      return "Name the new domain";
    }

    for (size_t n = 0; n < m_pending.m_needs.size(); n++) {
      const Need& need = m_pending.m_needs[n];

      if (need.m_name.empty()) {
        m_pending.m_asked = n;

        const Language::ShapeSite* site = m_model->siteOfLeaf(need.m_leaf);

        const std::string refusal = this->refusal(need, site);

        if (!refusal.empty()) {
          m_pending = Pending{};
          m_note    = "not written: " + refusal;

          return "";
        }

        const std::string part = (need.m_part != SurfaceSelector::Any && need.m_part < site->m_parts.size())
                                   ? " (its part '" + site->m_parts[need.m_part] + "')"
                                   : "";

        return "Name the " + site->m_shape + " at " + site->m_span.m_file + ":" + std::to_string(site->m_span.m_line) +
               part;
      }
    }

    try {
      this->write();
    } catch (const std::exception& e) {
      m_note = std::string("not written: ") + e.what();
    }

    m_pending = Pending{};

    return "";
  }

  // Why a shape (or part) cannot be given a name where it is made, or "" when it can.
  std::string
  refusal(const Need& a_need, const Language::ShapeSite* a_site) const
  {
    if (a_site == nullptr) {
      return "a picked surface is not a shape the model's text makes";
    }
    if (this->source(a_site->m_span.m_file) == nullptr) {
      return "the " + a_site->m_shape + " picked is made outside the model's files (" + a_site->m_span.m_file + ")";
    }

    const std::string where =
      "the " + a_site->m_shape + " at " + a_site->m_span.m_file + ":" + std::to_string(a_site->m_span.m_line);

    // A value used in several places: a label where it is made would name every one of them.
    const uint32_t site  = m_model->siteIndexOfLeaf(a_need.m_leaf);
    uint32_t       count = 0;

    for (uint32_t leaf = 0; leaf < m_model->index().getNumLeaves(); leaf++) {
      count += m_model->siteIndexOfLeaf(leaf) == site ? 1U : 0U;
    }

    if (count > 1U) {
      return where + " is used " + std::to_string(count) + " times; give the one picked a value of its own";
    }

    // A label the text gives it already, which names other surfaces too.
    if (a_need.m_part == SurfaceSelector::Any && !a_site->m_label.empty()) {
      return where + " is labeled \"" + a_site->m_label +
             "\" with other shapes; give it a label of its own in the file";
    }
    if (a_need.m_part != SurfaceSelector::Any && a_need.m_part < a_site->m_partLabeled.size() &&
        a_site->m_partLabeled[a_need.m_part]) {
      return where + ": its part '" + a_site->m_parts[a_need.m_part] + "' is labeled \"" +
             a_site->m_partLabels[a_need.m_part] + "\" with others; give it a label of its own in the file";
    }

    return "";
  }

  // The leaves a label names (as their own label or a part's).
  std::vector<std::pair<uint32_t, uint32_t>>
  usesOf(const SurfaceLabelId a_label) const
  {
    const ExprIndex<T>& index = m_model->index();

    std::vector<std::pair<uint32_t, uint32_t>> uses;

    for (uint32_t leaf = 0; leaf < index.getNumLeaves(); leaf++) {
      if (index.getSurfaceLabel(leaf) == a_label) {
        uses.emplace_back(leaf, SurfaceSelector::Any);
      }

      for (uint32_t part = 0; part < index.getNumPartsOfLeaf(leaf); part++) {
        if (index.getPartSurfaceLabel(leaf, part) == a_label) {
          uses.emplace_back(leaf, part);
        }
      }
    }

    return uses;
  }

  // The label a clicked surface (or part) is written by: one that names it alone; "" for none.
  std::string
  labelFor(const SurfaceSelector& a_selector) const
  {
    const ExprIndex<T>& index = m_model->index();
    const uint32_t      leaf  = index.getLeafOfSurface(a_selector.m_surface);

    // A name given in this edit.
    for (const Need& need : m_pending.m_needs) {
      if (need.m_leaf == leaf && need.m_part == a_selector.m_part && !need.m_name.empty()) {
        return need.m_name;
      }
    }

    // A selector picking a label (a mesh's face group, see RegionNames::setPickedByLabel()) is written by it, when only
    // this shape carries it.
    if (a_selector.m_label != SurfaceSelector::Any) {
      const SurfaceLabelId picked(a_selector.m_label);

      for (const auto& use : this->usesOf(picked)) {
        if (use.first != leaf) {
          return "";
        }
      }

      return index.getSurfaceLabelName(picked);
    }

    const SurfaceLabelId label = (a_selector.m_part != SurfaceSelector::Any)
                                   ? index.getPartSurfaceLabel(leaf, a_selector.m_part)
                                   : index.getSurfaceLabel(leaf);

    if (label.isNone()) {
      return "";
    }

    for (const auto& [l, p] : this->usesOf(label)) {
      if (l != leaf || (a_selector.m_part != SurfaceSelector::Any && p != a_selector.m_part)) {
        return "";
      }
    }

    return index.getSurfaceLabelName(label);
  }

  // A region's name in the text.
  std::string
  regionName(const uint32_t a_region) const
  {
    return a_region == 0 ? "outside" : m_model->index().getRegion(a_region).m_name;
  }

  // A selector as text; "" when the language cannot write it.
  std::string
  selectorText(const SurfaceSelector& a_s) const
  {
    const ExprIndex<T>& index = m_model->index();

    // A region with no name (an object that is not an overlay) cannot be written.
    for (const uint32_t region : {a_s.m_regionA, a_s.m_regionB}) {
      if (region != SurfaceSelector::Any && this->regionName(region).empty()) {
        return "";
      }
    }

    const auto between = [&]() {
      return (a_s.m_regionA != SurfaceSelector::Any && a_s.m_regionB != SurfaceSelector::Any)
               ? ", between = (" + this->regionName(a_s.m_regionA) + ", " + this->regionName(a_s.m_regionB) + ")"
               : std::string();
    };

    if (a_s.m_rest) {
      return "remaining()";
    }
    if (a_s.m_surface != SurfaceSelector::Any) {
      const std::string label = this->labelFor(a_s);

      return label.empty() ? "" : "label(\"" + label + "\"" + between() + ")";
    }
    if (a_s.m_label != SurfaceSelector::Any) {
      return "label(\"" + index.getSurfaceLabelName(SurfaceLabelId(a_s.m_label)) + "\"" + between() + ")";
    }
    if (a_s.m_regionA != SurfaceSelector::Any && a_s.m_regionB != SurfaceSelector::Any) {
      return "interface(" + this->regionName(a_s.m_regionA) + ", " + this->regionName(a_s.m_regionB) + ")";
    }
    if (a_s.m_regionA != SurfaceSelector::Any) {
      return "region(" + this->regionName(a_s.m_regionA) + ")";
    }
    if (a_s.m_materialA != SurfaceSelector::Any) {
      return "materials(" + index.getMaterialLabelName(MaterialLabelId(a_s.m_materialA)) +
             (a_s.m_materialB != SurfaceSelector::Any
                ? ", " + index.getMaterialLabelName(MaterialLabelId(a_s.m_materialB))
                : "") +
             ")";
    }

    return "";
  }

  // A file's text as the model read it.
  const Language::Syntax::File*
  source(const std::string& a_file) const
  {
    for (const Language::Syntax::File& file : m_model->sources()) {
      if (file.m_name == a_file) {
        return &file;
      }
    }

    return nullptr;
  }

  // The file a new domain is appended to: the folder's setup.ebg, or the model's file.
  std::string
  setupFile() const
  {
    if (std::filesystem::is_directory(m_path)) {
      return (std::filesystem::path(m_path) / "setup.ebg").string();
    }

    return m_path;
  }

  // The domain's block, the edit applied: the edits to make.
  std::vector<Edit>
  domainEdits()
  {
    std::vector<Edit> edits;

    // The groups' lines, as the edit has them.
    std::vector<std::string> lines(m_pending.m_setup.getNumSurfaceGroups());
    std::vector<bool>        written(lines.size(), true);

    for (uint32_t i = 0; i < m_pending.m_setup.getNumSelectors(); i++) {
      const uint32_t    g    = m_pending.m_setup.getSurfaceGroupOfSelector(i);
      const std::string text = this->selectorText(m_pending.m_setup.getSelector(i));

      if (text.empty()) {
        written[g] = false;

        continue;
      }

      lines[g] += (lines[g].empty() ? "" : ", ") + text;
    }

    // The domain as written, if it is.
    const Language::Syntax::Statement* statement = nullptr;
    const Language::Syntax::File*      file      = nullptr;

    for (const Language::Syntax::File& f : m_model->sources()) {
      for (const Language::Syntax::Statement& s : f.m_statements) {
        if (s.m_kind == Language::Syntax::StatementKind::Domain && s.m_name == m_pending.m_domain) {
          statement = &s;
          file      = &f;
        }
      }
    }

    const auto groupLine = [&](const uint32_t a_group, const Language::Syntax::DomainItem* a_old) {
      std::string line = "group " + m_pending.m_names[a_group] + " = " + (lines[a_group].empty() ? "" : lines[a_group]);

      if (a_old != nullptr && a_old->m_properties.m_written) {
        line += " " + file->m_text.substr(a_old->m_properties.m_span.m_begin,
                                          a_old->m_properties.m_span.m_end - a_old->m_properties.m_span.m_begin);
      }

      return line;
    };

    if (statement == nullptr) {
      std::string block = "\ndomain " + m_pending.m_domain + " {\n";

      for (uint32_t g = 0; g < lines.size(); g++) {
        if (!lines[g].empty()) {
          block += "  " + groupLine(g, nullptr) + "\n";
        }
      }

      block += "}\n";

      const std::string target = this->setupFile();
      const auto        text   = readFile(target);
      const uint32_t    end    = text.has_value() ? static_cast<uint32_t>(text->size()) : 0U;

      edits.push_back(Edit{target, end, end, text.has_value() ? block : "ebg 0\n" + block});

      return edits;
    }

    // The block's items: subdomains kept as written; each group kept if unchanged, rewritten otherwise. Each item
    // keeps the text before it (its line break, indentation and the comments above it).
    const std::string& text = file->m_text;

    const auto slice = [&text](const uint32_t a_begin, const uint32_t a_end) {
      return text.substr(a_begin, a_end - a_begin);
    };

    const uint32_t open = static_cast<uint32_t>(text.find('{', statement->m_nameSpan.m_end)) + 1U;

    std::map<std::string, std::string>                         leading;
    std::map<std::string, const Language::Syntax::DomainItem*> oldGroups;
    std::vector<const Language::Syntax::DomainItem*>           oldOrder;

    uint32_t previousEnd = open;

    for (const Language::Syntax::DomainItem& item : statement->m_items) {
      leading[item.m_name] = slice(previousEnd, item.m_span.m_begin);
      previousEnd          = item.m_span.m_end;

      if (!item.m_subdomain) {
        oldGroups[item.m_name] = &item;

        oldOrder.push_back(&item);
      }
    }

    // A group's line as written: by its name, or, renamed, by its number (a group keeps its number when renamed),
    // when no group of the edit has the old name.
    const auto oldItemOf = [&](const uint32_t a_group) -> const Language::Syntax::DomainItem* {
      const auto byName = oldGroups.find(m_pending.m_names[a_group]);

      if (byName != oldGroups.end()) {
        return byName->second;
      }

      if (a_group < oldOrder.size() &&
          std::find(m_pending.m_names.begin(), m_pending.m_names.end(), oldOrder[a_group]->m_name) ==
            m_pending.m_names.end()) {
        return oldOrder[a_group];
      }

      return nullptr;
    };

    std::string block = slice(statement->m_span.m_begin, open);

    for (const Language::Syntax::DomainItem& item : statement->m_items) {
      if (item.m_subdomain) {
        block += leading[item.m_name] + slice(item.m_span.m_begin, item.m_span.m_end);
      }
    }

    for (uint32_t g = 0; g < lines.size(); g++) {
      const Language::Syntax::DomainItem* item = oldItemOf(g);

      // The same selectors as the model read, whatever the text they are written in: kept as written, renamed.
      if (item != nullptr && this->sameSelectors(*item, oldOrder, g)) {
        block += leading[item->m_name] + slice(item->m_span.m_begin, item->m_nameSpan.m_begin) + m_pending.m_names[g] +
                 slice(item->m_nameSpan.m_end, item->m_span.m_end);
        written[g] = true;

        continue;
      }

      if (lines[g].empty()) {
        written[g] = false;

        continue;
      }

      block += (item != nullptr) ? leading[item->m_name] : "\n  ";
      block += groupLine(g, item);
    }

    // What follows the last item: comments below it and the closing brace.
    block += slice(previousEnd, statement->m_span.m_end);

    edits.push_back(Edit{file->m_name, statement->m_span.m_begin, statement->m_span.m_end, block});

    for (uint32_t g = 0; g < written.size(); g++) {
      if (!written[g]) {
        m_note = "group " + m_pending.m_names[g] +
                 ": what the language cannot write (an empty group, a selector) was "
                 "left out";
      }
    }

    return edits;
  }

  // Whether a group of the edit holds the selectors the model read for a group as written, in the same order.
  bool
  sameSelectors(const Language::Syntax::DomainItem&                     a_item,
                const std::vector<const Language::Syntax::DomainItem*>& a_order,
                const uint32_t                                          a_group) const
  {
    const auto     position = std::find(a_order.begin(), a_order.end(), &a_item);
    const uint32_t read     = static_cast<uint32_t>(position - a_order.begin());

    const DomainSetup& before = m_model->domain(m_pending.m_domain).setup();
    const DomainSetup& after  = m_pending.m_setup;

    const auto selectorsOf = [](const DomainSetup& a_setup, const uint32_t a_of) {
      std::vector<SurfaceSelector> selectors;

      for (uint32_t i = 0; i < a_setup.getNumSelectors(); i++) {
        if (a_setup.getSurfaceGroupOfSelector(i) == a_of) {
          selectors.push_back(a_setup.getSelector(i));
        }
      }

      return selectors;
    };

    const std::vector<SurfaceSelector> a = selectorsOf(before, read);
    const std::vector<SurfaceSelector> b = selectorsOf(after, a_group);

    const auto same = [](const SurfaceSelector& x, const SurfaceSelector& y) {
      return x.m_surface == y.m_surface && x.m_part == y.m_part && x.m_regionA == y.m_regionA &&
             x.m_regionB == y.m_regionB && x.m_label == y.m_label && x.m_materialA == y.m_materialA &&
             x.m_materialB == y.m_materialB && x.m_rest == y.m_rest;
    };

    return position != a_order.end() && read < before.getNumSurfaceGroups() &&
           std::equal(a.begin(), a.end(), b.begin(), b.end(), same);
  }

  // The labels to add after the shapes' calls.
  std::vector<Edit>
  labelEdits() const
  {
    std::vector<Edit> edits;

    for (const Need& need : m_pending.m_needs) {
      const Language::ShapeSite* site = m_model->siteOfLeaf(need.m_leaf);

      const std::string label = (need.m_part != SurfaceSelector::Any && need.m_part < site->m_parts.size())
                                  ? ".label(" + site->m_parts[need.m_part] + " = \"" + need.m_name + "\")"
                                  : ".label(\"" + need.m_name + "\")";

      edits.push_back(Edit{site->m_span.m_file, site->m_span.m_end, site->m_span.m_end, label});
    }

    return edits;
  }

  // Write the pending edit: the labels and the domain's block, every file checked against the text the model read.
  void
  write()
  {
    std::vector<Edit> edits = this->labelEdits();

    for (Edit& edit : this->domainEdits()) {
      edits.push_back(edit);
    }

    // The files, and their new text: the edits applied from the end, so that earlier positions hold.
    std::map<std::string, std::optional<std::string>> before;

    for (const Edit& edit : edits) {
      if (before.count(edit.m_file) > 0) {
        continue;
      }

      const auto                    disk = readFile(edit.m_file);
      const Language::Syntax::File* read = this->source(edit.m_file);

      if (read != nullptr && (!disk.has_value() || *disk != read->m_text)) {
        m_note = edit.m_file + " changed on disk since it was read: not written (it reloads)";

        return;
      }

      before[edit.m_file] = disk;
    }

    std::map<std::string, std::string> after;

    for (const auto& [file, text] : before) {
      after[file] = text.value_or("");
    }

    std::stable_sort(edits.begin(), edits.end(), [](const Edit& a, const Edit& b) { return a.m_begin > b.m_begin; });

    for (const Edit& edit : edits) {
      std::string& text = after[edit.m_file];

      if (edit.m_begin > edit.m_end || edit.m_end > text.size()) {
        m_note = "not written: an edit falls outside " + edit.m_file;

        return;
      }

      text.replace(edit.m_begin, edit.m_end - edit.m_begin, edit.m_text);
    }

    Write record;

    for (const auto& [file, text] : before) {
      record.m_before.emplace_back(file, text);
      record.m_after.emplace_back(file, after[file]);
    }

    if (!writeFiles(record.m_after, record.m_before)) {
      m_note = "not written: a file could not be written";

      return;
    }

    m_undo.push_back(record);
    m_redo.clear();

    m_note = "wrote " + std::to_string(record.m_after.size()) + " file(s); Ctrl+Z undoes";
  }

  // A file written through a link is its target: the target is replaced, the link kept.
  static std::string
  target(const std::string& a_file)
  {
    std::error_code ec;

    const std::filesystem::path resolved =
      std::filesystem::is_symlink(a_file, ec) ? std::filesystem::canonical(a_file, ec) : std::filesystem::path(a_file);

    return ec ? a_file : resolved.string();
  }

  // The temporary a file's text is written to before it is renamed over the file.
  static std::string
  temporary(const std::string& a_file)
  {
    return target(a_file) + ".ebgstudio-new";
  }

  // Write each text to its file's temporary (a file with no text has none); false, the temporaries removed, when one
  // could not be written. A file a temporary replaces must be writable itself (renaming over a read-only file would
  // succeed), and the temporary takes its permissions.
  static bool
  stage(const Texts& a_files)
  {
    namespace fs = std::filesystem;

    std::vector<std::string> made;

    bool ok = true;

    for (const auto& [file, text] : a_files) {
      if (!ok || !text.has_value()) {
        continue;
      }

      std::error_code ec;

      const bool exists = fs::exists(file, ec);

      if (exists && !std::ofstream(file, std::ios::binary | std::ios::app)) {
        ok = false;

        continue;
      }

      std::ofstream out(temporary(file), std::ios::binary);

      if (out) {
        made.push_back(temporary(file));
      }

      out << *text;
      out.close();

      ok = !out.fail();

      if (ok && exists) {
        fs::permissions(temporary(file), fs::status(target(file), ec).permissions(), ec);

        ok = !ec;
      }
    }

    if (!ok) {
      for (const std::string& file : made) {
        std::error_code ec;

        fs::remove(file, ec);
      }
    }

    return ok;
  }

  // Rename each staged temporary over its file, or remove a file with no text; how many were done before one failed
  // (the temporaries left are then removed).
  static size_t
  commit(const Texts& a_files)
  {
    namespace fs = std::filesystem;

    for (size_t i = 0; i < a_files.size(); i++) {
      const auto& [file, text] = a_files[i];

      std::error_code ec;

      if (text.has_value()) {
        fs::rename(temporary(file), target(file), ec);
      }
      else {
        fs::remove(file, ec);
      }

      if (ec) {
        for (size_t k = i; k < a_files.size(); k++) {
          fs::remove(temporary(a_files[k].first), ec);
        }

        return i;
      }
    }

    return a_files.size();
  }

  // Write files' texts (removing a file with none), all or none: each text to a temporary next to its file, and once
  // every temporary is written, each renamed over its file. False when a file could not be written: the files are then
  // as before (a_previous, in the same order), put back the same way; only a failure while putting them back (the
  // disk full, say) can leave one changed. A file put back that the write removed gets the default permissions.
  static bool
  writeFiles(const Texts& a_files, const Texts& a_previous)
  {
    if (!stage(a_files)) {
      return false;
    }

    const size_t done = commit(a_files);

    if (done < a_files.size()) {
      const Texts back(a_previous.begin(), a_previous.begin() + static_cast<std::ptrdiff_t>(done));

      if (stage(back)) {
        (void)commit(back);
      }

      return false;
    }

    return true;
  }

  // Undo or redo: restore one side of a write, if the files still hold the other.
  bool
  step(std::vector<Write>& a_from, std::vector<Write>& a_to, const bool a_undo)
  {
    if (a_from.empty()) {
      return false;
    }

    const Write& record = a_from.back();

    const auto& expected = a_undo ? record.m_after : record.m_before;
    const auto& restored = a_undo ? record.m_before : record.m_after;

    for (const auto& [file, text] : expected) {
      if (readFile(file) != text) {
        m_note = file + " changed since the write: nothing " + (a_undo ? "undone" : "redone");

        return true;
      }
    }

    if (!writeFiles(restored, expected)) {
      m_note = std::string("a file could not be written: not ") + (a_undo ? "undone" : "redone");

      return true;
    }

    a_to.push_back(record);
    a_from.pop_back();

    m_note = a_undo ? "undone" : "redone";

    return true;
  }
};

} // namespace Studio

#endif
