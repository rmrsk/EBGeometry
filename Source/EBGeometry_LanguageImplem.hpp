// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_LanguageImplem.hpp
 * @brief  Implementation of EBGeometry_Language.hpp: the reader (evaluation, names, domains), the domain check, the
 * inventory and the printer.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_LANGUAGEIMPLEM_HPP
#define EBGEOMETRY_LANGUAGEIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cctype>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <type_traits>

// Our includes
#include "EBGeometry_Language.hpp"

namespace EBGeometry {
namespace Language {

namespace LanguageDetail {

/**
 * @brief The language's built-in shapes and operations, by name: the shapes first, then the operations.
 * @details Every name here has its line in the Sphinx pages of shapes and operations and is used once in
 * Studio/Models/gallery, which the unit tests check.
 * @return The names.
 */
inline const std::vector<std::string>&
builtinNames()
{
  static const std::vector<std::string> names{"plane",
                                              "sphere",
                                              "box",
                                              "torus",
                                              "cylinder",
                                              "infinite_cylinder",
                                              "capsule",
                                              "infinite_cone",
                                              "cone",
                                              "rounded_box",
                                              "perlin",
                                              "rounded_cylinder",
                                              "ellipsoid",
                                              "capped_cone",
                                              "round_cone",
                                              "polygon_prism",
                                              "tube",
                                              "capped_torus",
                                              "cut_sphere",
                                              "cut_hollow_sphere",
                                              "wedge",
                                              "mesh",
                                              "union",
                                              "bvh_union",
                                              "smooth_union",
                                              "intersection",
                                              "smooth_intersection",
                                              "difference",
                                              "bvh_difference",
                                              "smooth_difference",
                                              "complement",
                                              "translate",
                                              "rotate",
                                              "scale",
                                              "reflect",
                                              "offset",
                                              "annular",
                                              "elongate",
                                              "blur",
                                              "mollify",
                                              "repeat",
                                              "label"};

  return names;
}

/**
 * @brief A type's name in messages.
 * @param[in] a_type The type.
 * @return The name.
 */
inline const char*
typeName(const Properties::Type a_type) noexcept
{
  switch (a_type) {
  case Properties::Type::Number: {
    return "number";
  }
  case Properties::Type::Vector: {
    return "vector";
  }
  case Properties::Type::String: {
    return "string";
  }
  default: {
    return "bool";
  }
  }
}

/**
 * @brief The edit distance of two names: insertions, deletions and substitutions.
 * @param[in] a_first  A name.
 * @param[in] a_second A name.
 * @return The distance.
 */
inline size_t
editDistance(std::string_view a_first, std::string_view a_second)
{
  std::vector<size_t> row(a_second.size() + 1);

  for (size_t j = 0; j <= a_second.size(); j++) {
    row[j] = j;
  }

  for (size_t i = 1; i <= a_first.size(); i++) {
    size_t diagonal = row[0];

    row[0] = i;

    for (size_t j = 1; j <= a_second.size(); j++) {
      const size_t above = row[j];

      row[j] =
        Math::min(Math::min(row[j] + 1, row[j - 1] + 1), diagonal + (a_first[i - 1] == a_second[j - 1] ? 0U : 1U));
      diagonal = above;
    }
  }

  return row[a_second.size()];
}

/**
 * @brief The words to append to "no X 'name'": the closest known name, and the known names when there are few.
 * @param[in] a_name  The unknown name.
 * @param[in] a_known The known names.
 * @return The words, starting with a space or empty.
 */
inline std::string
suggest(std::string_view a_name, const std::vector<std::string>& a_known)
{
  if (a_known.empty()) {
    return " (there are none)";
  }

  const std::string* best     = nullptr;
  size_t             distance = 0;

  for (const std::string& known : a_known) {
    const size_t d = editDistance(a_name, known);

    if (best == nullptr || d < distance) {
      best     = &known;
      distance = d;
    }
  }

  std::string words;

  if (distance <= Math::max<size_t>(2, a_name.size() / 3)) {
    words = " (did you mean '" + *best + "'?)";
  }

  if (a_known.size() <= 12) {
    words += "; known: ";

    for (size_t i = 0; i < a_known.size(); i++) {
      words += (i > 0 ? ", " : "") + a_known[i];
    }
  }

  return words;
}

/**
 * @brief Names joined by commas.
 * @param[in] a_names The names.
 * @return The text.
 */
inline std::string
joined(const std::vector<std::string>& a_names)
{
  std::string text;

  for (size_t i = 0; i < a_names.size(); i++) {
    text += (i > 0 ? ", " : "") + a_names[i];
  }

  return text;
}

/**
 * @brief Whether two property values are the same.
 * @param[in] a_first  A value.
 * @param[in] a_second A value.
 * @return Whether.
 */
inline bool
same(const Properties::Value& a_first, const Properties::Value& a_second) noexcept
{
  if (a_first.m_type != a_second.m_type) {
    return false;
  }

  switch (a_first.m_type) {
  case Properties::Type::Vector: {
    return a_first.m_vector[0] == a_second.m_vector[0] && a_first.m_vector[1] == a_second.m_vector[1] &&
           a_first.m_vector[2] == a_second.m_vector[2];
  }
  case Properties::Type::String: {
    return a_first.m_string == a_second.m_string;
  }
  default: {
    return a_first.m_number == a_second.m_number;
  }
  }
}

/**
 * @brief A shape as the reader holds it: its expression, and what the language checks before the expression builder
 * would abort (regions placed where an Overlay may not be, names given twice, parts named on a composition).
 * @tparam T Precision.
 */
template <class T>
struct Shape
{
  /**
   * @brief The expression; for a plain shape, the shape without its labels.
   */
  std::optional<Expr<T>> m_expr;

  /**
   * @brief Whether it is one built-in shape or mesh, not yet moved or combined: its parts can be named.
   */
  bool m_plain = false;

  /**
   * @brief Its entry among the model's shapes (ShapeSite), for a plain shape.
   */
  uint32_t m_site = 0;

  /**
   * @brief A plain shape's part names.
   */
  std::vector<std::string> m_parts;

  /**
   * @brief A plain shape's label of each part ("" for none).
   */
  std::vector<std::string> m_partLabels;

  /**
   * @brief Where each part's label is given (no line: not given in the text, a mesh's face group).
   */
  std::vector<Span> m_partLabelSpans;

  /**
   * @brief A plain shape's own label ("" for none).
   */
  std::string m_label;

  /**
   * @brief Where its own label is given.
   */
  Span m_labelSpan;

  /**
   * @brief For a mesh: its face groups' names, as the file gives them.
   */
  std::vector<std::string> m_groupNames;

  /**
   * @brief For a mesh: the group of each face (its parts are its face groups).
   */
  std::vector<uint32_t> m_faceGroup;

  /**
   * @brief Whether it is a mesh.
   */
  bool m_mesh = false;

  /**
   * @brief Whether it holds regions: an Overlay, maybe moved or clipped.
   */
  bool m_partition = false;

  /**
   * @brief The names of the regions and region groups in it, and where each is given.
   */
  std::vector<std::pair<std::string, Span>> m_names;

  /**
   * @brief How many operations deep its expression is, through named values too (a shape is 1).
   */
  uint32_t m_depth = 1;

  /**
   * @brief How many operations that a tape evaluates as calls (blur, mollify, repeat, the BVH operations) nest in it.
   */
  uint32_t m_calls = 0;
};

/**
 * @brief A value of the language.
 * @tparam T Precision.
 */
template <class T>
struct Value
{
  /**
   * @brief The types; Invalid is a value whose statement had an error (using it reports nothing more).
   */
  enum class Type : uint8_t
  {
    Invalid,
    Number,
    Vector,
    String,
    Bool,
    Shape
  };

  /**
   * @brief The type.
   */
  Type m_type = Type::Invalid;

  /**
   * @brief A number, or a bool (1 or 0).
   */
  double m_number = 0.0;

  /**
   * @brief A vector.
   */
  Vec3T<double> m_vector = Vec3T<double>::zeros();

  /**
   * @brief A string.
   */
  std::string m_string;

  /**
   * @brief A shape.
   */
  std::shared_ptr<const Shape<T>> m_shape;

  /**
   * @brief Where it is written.
   */
  Span m_span;
};

/**
 * @brief A type's name in messages.
 * @tparam T Precision.
 * @param[in] a_type The type.
 * @return The name.
 */
template <class T>
inline const char*
valueTypeName(const typename Value<T>::Type a_type) noexcept
{
  switch (a_type) {
  case Value<T>::Type::Number: {
    return "a number";
  }
  case Value<T>::Type::Vector: {
    return "a vector";
  }
  case Value<T>::Type::String: {
    return "a string";
  }
  case Value<T>::Type::Bool: {
    return "a bool";
  }
  case Value<T>::Type::Shape: {
    return "a shape";
  }
  default: {
    return "invalid";
  }
  }
}

/**
 * @brief Thrown when a value with an error of its own is used: the error is reported once, where it is.
 */
struct Poisoned
{
};

/**
 * @brief What an argument of a built-in must be.
 */
enum class Kind : uint8_t
{
  Number,       ///< A number.
  Vector,       ///< A vector.
  String,       ///< A string.
  Bool,         ///< true or false.
  Shape,        ///< A shape.
  Axis,         ///< "x", "y" or "z".
  AxisOrVector, ///< An axis or a vector.
  Count,        ///< A whole number, 0 or more.
  Shapes        ///< The remaining positional arguments, each a shape.
};

/**
 * @brief An argument of a built-in: its name and kind, and whether it may be left out.
 */
struct Spec
{
  /**
   * @brief The name.
   */
  const char* m_name;

  /**
   * @brief The kind.
   */
  Kind m_kind;

  /**
   * @brief Whether it may be left out.
   */
  bool m_optional = false;
};

} // namespace LanguageDetail

// ---------------------------------------------------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------------------------------------------------

inline bool
Properties::has(std::string_view a_key) const noexcept
{
  return this->find(a_key) != nullptr;
}

inline std::vector<std::string>
Properties::keys() const
{
  std::vector<std::string> keys;

  for (const auto& entry : m_entries) {
    keys.push_back(entry.first);
  }

  return keys;
}

inline const Properties::Value*
Properties::find(std::string_view a_key) const noexcept
{
  for (const auto& entry : m_entries) {
    if (entry.first == a_key) {
      return &entry.second;
    }
  }

  return nullptr;
}

inline void
Properties::set(const std::string& a_key, const Value& a_value)
{
  for (auto& entry : m_entries) {
    if (entry.first == a_key) {
      entry.second = a_value;

      return;
    }
  }

  m_entries.emplace_back(a_key, a_value);
}

inline Error
Properties::missing(std::string_view a_key) const
{
  return Error(m_span,
               m_owner + " has no property '" + std::string(a_key) + "'" +
                 (m_entries.empty() ? " (it has none)" : "; it has: " + LanguageDetail::joined(this->keys())));
}

inline const Properties::Value*
Properties::typed(std::string_view a_key, const Type a_type) const
{
  const Value* value = this->find(a_key);

  if (value != nullptr && value->m_type != a_type) {
    throw Error(value->m_span,
                m_owner + ": '" + std::string(a_key) + "' is a " + LanguageDetail::typeName(value->m_type) +
                  ", not a " + LanguageDetail::typeName(a_type));
  }

  return value;
}

inline double
Properties::number(std::string_view a_key) const
{
  const Value* value = this->typed(a_key, Type::Number);

  if (value == nullptr) {
    throw this->missing(a_key);
  }

  return value->m_number;
}

inline double
Properties::number(std::string_view a_key, const double a_fallback) const
{
  const Value* value = this->typed(a_key, Type::Number);

  return value != nullptr ? value->m_number : a_fallback;
}

template <class T>
inline Vec3T<T>
Properties::narrowed(std::string_view a_key, const Value& a_value) const
{
  for (size_t dir = 0; dir < 3; dir++) {
    if (!(std::abs(a_value.m_vector[dir]) <= double(Math::Limits<T>::max()))) {
      throw Error(a_value.m_span,
                  m_owner + ": '" + std::string(a_key) + "' is too large for " +
                    (std::is_same<T, float>::value ? "single" : "double") + " precision");
    }
  }

  return Vec3T<T>(T(a_value.m_vector[0]), T(a_value.m_vector[1]), T(a_value.m_vector[2]));
}

template <class T>
inline Vec3T<T>
Properties::vector(std::string_view a_key) const
{
  const Value* value = this->typed(a_key, Type::Vector);

  if (value == nullptr) {
    throw this->missing(a_key);
  }

  return this->narrowed<T>(a_key, *value);
}

template <class T>
inline Vec3T<T>
Properties::vector(std::string_view a_key, const Vec3T<T>& a_fallback) const
{
  const Value* value = this->typed(a_key, Type::Vector);

  return value != nullptr ? this->narrowed<T>(a_key, *value) : a_fallback;
}

inline const std::string&
Properties::string(std::string_view a_key) const
{
  const Value* value = this->typed(a_key, Type::String);

  if (value == nullptr) {
    throw this->missing(a_key);
  }

  return value->m_string;
}

inline const std::string&
Properties::string(std::string_view a_key, const std::string& a_fallback) const
{
  const Value* value = this->typed(a_key, Type::String);

  return value != nullptr ? value->m_string : a_fallback;
}

inline bool
Properties::flag(std::string_view a_key) const
{
  const Value* value = this->typed(a_key, Type::Bool);

  if (value == nullptr) {
    throw this->missing(a_key);
  }

  return value->m_number != 0.0;
}

inline bool
Properties::flag(std::string_view a_key, const bool a_fallback) const
{
  const Value* value = this->typed(a_key, Type::Bool);

  return value != nullptr ? value->m_number != 0.0 : a_fallback;
}

inline uint32_t
Properties::code(std::string_view a_key, std::initializer_list<std::string_view> a_names) const
{
  const std::string& text = this->string(a_key);

  uint32_t                 index = 0;
  std::vector<std::string> names;

  for (const std::string_view name : a_names) {
    if (text == name) {
      return index;
    }

    names.emplace_back(name);

    index++;
  }

  throw Error(this->spanOf(a_key),
              m_owner + ": '" + std::string(a_key) + "' is \"" + text +
                "\", which is none of: " + LanguageDetail::joined(names));
}

inline Span
Properties::spanOf(std::string_view a_key) const
{
  const Value* value = this->find(a_key);

  return value != nullptr ? value->m_span : m_span;
}

// ---------------------------------------------------------------------------------------------------------------------
// ModelDomain and Model
// ---------------------------------------------------------------------------------------------------------------------

inline const std::string&
ModelDomain::subdomainName(const uint32_t a_subdomain) const
{
  EBGEOMETRY_REQUIRE(a_subdomain < m_subdomainNames.size(),
                     "ModelDomain: subdomain %u of %u",
                     unsigned(a_subdomain),
                     unsigned(m_subdomainNames.size()));

  return m_subdomainNames[a_subdomain];
}

inline uint32_t
ModelDomain::findSubdomain(std::string_view a_name) const noexcept
{
  for (uint32_t s = 0; s < m_subdomainNames.size(); s++) {
    if (m_subdomainNames[s] == a_name) {
      return s;
    }
  }

  return Subdomains::None;
}

inline const Properties&
ModelDomain::subdomainProperties(const uint32_t a_subdomain) const
{
  EBGEOMETRY_REQUIRE(a_subdomain < m_subdomainProperties.size(),
                     "ModelDomain: subdomain %u of %u",
                     unsigned(a_subdomain),
                     unsigned(m_subdomainProperties.size()));

  return m_subdomainProperties[a_subdomain];
}

inline const Span&
ModelDomain::subdomainSpan(const uint32_t a_subdomain) const
{
  EBGEOMETRY_REQUIRE(a_subdomain < m_subdomainSpans.size(),
                     "ModelDomain: subdomain %u of %u",
                     unsigned(a_subdomain),
                     unsigned(m_subdomainSpans.size()));

  return m_subdomainSpans[a_subdomain];
}

inline const std::string&
ModelDomain::surfaceGroupName(const uint32_t a_group) const
{
  EBGEOMETRY_REQUIRE(a_group < m_groupNames.size(),
                     "ModelDomain: surface group %u of %u",
                     unsigned(a_group),
                     unsigned(m_groupNames.size()));

  return m_groupNames[a_group];
}

inline uint32_t
ModelDomain::findSurfaceGroup(std::string_view a_name) const noexcept
{
  for (uint32_t g = 0; g < m_groupNames.size(); g++) {
    if (m_groupNames[g] == a_name) {
      return g;
    }
  }

  return SurfaceGroups::None;
}

inline const Properties&
ModelDomain::surfaceGroupProperties(const uint32_t a_group) const
{
  EBGEOMETRY_REQUIRE(a_group < m_groupProperties.size(),
                     "ModelDomain: surface group %u of %u",
                     unsigned(a_group),
                     unsigned(m_groupProperties.size()));

  return m_groupProperties[a_group];
}

inline const Span&
ModelDomain::surfaceGroupSpan(const uint32_t a_group) const
{
  EBGEOMETRY_REQUIRE(a_group < m_groupSpans.size(),
                     "ModelDomain: surface group %u of %u",
                     unsigned(a_group),
                     unsigned(m_groupSpans.size()));

  return m_groupSpans[a_group];
}

template <class T>
inline const Properties&
Model<T>::materialProperties(const uint32_t a_material) const
{
  EBGEOMETRY_REQUIRE(
    a_material < m_materials.size(), "Model: material %u of %u", unsigned(a_material), unsigned(m_materials.size()));

  return m_materials[a_material];
}

template <class T>
inline const ModelDomain&
Model<T>::domain(const uint32_t a_domain) const
{
  EBGEOMETRY_REQUIRE(
    a_domain < m_domains.size(), "Model: domain %u of %u", unsigned(a_domain), unsigned(m_domains.size()));

  return m_domains[a_domain];
}

template <class T>
inline const ModelDomain&
Model<T>::domain(std::string_view a_name) const
{
  std::vector<std::string> names;

  for (const ModelDomain& d : m_domains) {
    if (d.name() == a_name) {
      return d;
    }

    names.push_back(d.name());
  }

  const Span where = m_sources.empty() ? Span{} : Span{m_sources[0].m_name, 0, 0, 0, 0};

  throw Error(where, "the model has no domain '" + std::string(a_name) + "'" + LanguageDetail::suggest(a_name, names));
}

template <class T>
inline RegionNames
Model<T>::names() const
{
  RegionNames names(*m_index);

  for (uint32_t d = 0; d < m_domains.size(); d++) {
    const ModelDomain& domain = m_domains[d];

    const uint32_t id = names.addDomain(domain.name(), domain.setup());

    for (uint32_t s = 0; s < domain.getNumSubdomains(); s++) {
      names.setSubdomain(id, s, domain.subdomainName(s));
    }

    for (uint32_t g = 0; g < domain.getNumSurfaceGroups(); g++) {
      names.setSurfaceGroup(id, g, domain.surfaceGroupName(g));
    }
  }

  if (!m_domains.empty()) {
    names.setActiveDomain(0);
  }

  // Each surface by the shape call it comes from, and its label: what a viewer shows a picked surface by.
  for (uint32_t s = 0; s < m_index->getNumSurfaces(); s++) {
    const uint32_t       leaf  = m_index->getLeafOfSurface(s);
    const ShapeSite*     site  = this->siteOfLeaf(leaf);
    const SurfaceLabelId label = m_index->getSurfaceLabel(leaf);

    if (site != nullptr) {
      names.setSurface(s,
                       site->m_shape + ", " + std::filesystem::path(site->m_span.m_file).filename().string() + ":" +
                         std::to_string(site->m_span.m_line) +
                         (label.isNone() ? "" : " \"" + m_index->getSurfaceLabelName(label) + "\""));

      // A mesh's parts are its faces, which the language names only through its face groups: picked by those.
      if (site->m_shape == "mesh") {
        names.setPickedByLabel(s);
      }
    }
  }

  for (const std::string& key : m_view.keys()) {
    const Properties::Value* value = m_view.find(key);

    if (key.rfind("color.", 0) == 0 && value->m_type == Properties::Type::Vector) {
      names.setColor(key.substr(6), value->m_vector);
    }
  }

  return names;
}

inline std::vector<std::string>
partNames(std::string_view a_shape, const uint32_t a_sides)
{
  if (a_shape == "box" || a_shape == "rounded_box") {
    return {"xmin", "xmax", "ymin", "ymax", "zmin", "zmax"};
  }
  if (a_shape == "cylinder" || a_shape == "rounded_cylinder" || a_shape == "capped_cone") {
    return {"side", "cap1", "cap2"};
  }
  if (a_shape == "capsule") {
    return {"side", "tip1", "tip2"};
  }
  if (a_shape == "cone") {
    return {"side", "base"};
  }
  if (a_shape == "round_cone") {
    return {"side", "end1", "end2"};
  }
  if (a_shape == "polygon_prism") {
    std::vector<std::string> names;

    for (uint32_t k = 0; k < a_sides; k++) {
      names.push_back("side" + std::to_string(k));
    }

    names.emplace_back("cap1");
    names.emplace_back("cap2");

    return names;
  }
  if (a_shape == "tube") {
    return {"outer", "inner", "end1", "end2"};
  }
  if (a_shape == "capped_torus") {
    return {"tube", "end1", "end2"};
  }
  if (a_shape == "cut_sphere") {
    return {"sphere", "cut"};
  }
  if (a_shape == "cut_hollow_sphere") {
    return {"shell", "rim"};
  }
  if (a_shape == "wedge") {
    return {"xmin", "xmax"};
  }

  return {};
}

// ---------------------------------------------------------------------------------------------------------------------
// The reader
// ---------------------------------------------------------------------------------------------------------------------

namespace LanguageDetail {

/**
 * @brief Reads a model: loads its files, evaluates their statements in order, then resolves materials, domains and
 * couplings against the object's index.
 * @details Internal; not part of the public API (read() and parse() are). Every error is collected; a statement with an error is skipped, and a value it defined is marked invalid, so
 * that its uses report nothing more. The expression builder aborts on misuse (an Overlay where it may not be, a name
 * given twice, parts named on a composition); the reader checks each of these first, so that a mistake in a file is
 * an error at its line, never an abort.
 * @tparam T Precision.
 */
template <class T>
class Reader
{
public:
  /**
   * @brief Reader.
   * @param[in,out] a_pool      Where meshes are built.
   * @param[in]     a_overrides Values for params.
   */
  Reader(Pool& a_pool, const Overrides& a_overrides) : m_pool(a_pool), m_overrides(a_overrides)
  {}

  /**
   * @brief Read a file and the files it includes.
   * @param[in] a_path The path.
   * @return The model; an Error with every diagnostic when it has errors.
   */
  Model<T>
  readPath(const std::string& a_path)
  {
    const ThrowOnRequire checks;

    try {
      return this->readFrom(a_path);
    } catch (const RequireError& e) {
      // A library check no call site turned into an error of its own: still an error of the model, never a raw
      // RequireError.
      throw Error(Span{a_path, 0, 0, 0, 0}, checkMessage(e));
    }
  }

  /**
   * @brief Read text.
   * @param[in] a_text The text.
   * @param[in] a_name Its file's name.
   * @return The model; an Error with every diagnostic when it has errors.
   */
  Model<T>
  readText(std::string_view a_text, const std::string& a_name)
  {
    const ThrowOnRequire checks;

    try {
      this->loadText(a_text, a_name);
      this->evaluate();

      return this->finish();
    } catch (const RequireError& e) {
      throw Error(Span{a_name, 0, 0, 0, 0}, checkMessage(e));
    }
  }

private:
  /**
   * @brief readPath(), under its checks.
   * @param[in] a_path The path.
   * @return The model.
   */
  Model<T>
  readFrom(const std::string& a_path)
  {
    namespace fs = std::filesystem;

    // A model is the file named, and the files it includes: nothing is read that a file does not name. The overload
    // taking an error code: a path that cannot be inspected is read as a file, whose error says why, never a
    // filesystem_error out of read().
    std::error_code ec;

    if (fs::is_directory(a_path, ec)) {
      throw Error(Span{a_path, 0, 0, 0, 0},
                  "'" + a_path +
                    "' is a folder: a model is a file, which includes the others it needs (name the setup " +
                    "file, say)");
    }

    this->loadFile(a_path, Span{a_path, 0, 0, 0, 0});

    this->evaluate();

    return this->finish();
  }

  using V     = LanguageDetail::Value<T>;
  using VT    = typename V::Type;
  using S     = LanguageDetail::Shape<T>;
  using Kind  = LanguageDetail::Kind;
  using Spec  = LanguageDetail::Spec;
  using Node  = Syntax::Node;
  using NK    = Syntax::NodeKind;
  using Stmt  = Syntax::Statement;
  using SK    = Syntax::StatementKind;
  using Specs = std::vector<Spec>;

  /**
   * @brief A value a statement defines.
   */
  struct Defined
  {
    /**
     * @brief The value.
     */
    V m_value;

    /**
     * @brief Where its name is written.
     */
    Span m_span;
  };

  /**
   * @brief An argument as written.
   */
  struct Arg
  {
    /**
     * @brief Its keyword, or "".
     */
    std::string m_key;

    /**
     * @brief Its value.
     */
    V m_value;
  };

  /**
   * @brief The arguments matched to a built-in's specs.
   */
  struct Bound
  {
    /**
     * @brief Each spec's value, or nullptr when left out.
     */
    std::vector<const V*> m_values;

    /**
     * @brief The variadic shapes.
     */
    std::vector<const V*> m_shapes;
  };

  /**
   * @brief Where meshes are built.
   */
  Pool& m_pool;

  /**
   * @brief Values for params.
   */
  const Overrides& m_overrides;

  /**
   * @brief The overrides a param took.
   */
  std::set<std::string> m_overridesUsed;

  /**
   * @brief Every error and warning.
   */
  std::vector<Diagnostic> m_diagnostics;

  /**
   * @brief The parsed files (a deque, so that statements stay where they are as files are added).
   */
  std::deque<Syntax::File> m_sources;

  /**
   * @brief The files being read, outermost first: for include cycles and relative paths.
   */
  std::vector<std::string> m_reading;

  /**
   * @brief Every file read, by its canonical path, and where it was included from: a file is read once.
   */
  std::map<std::string, Span> m_read;

  /**
   * @brief Where each domain's remaining() is written.
   */
  std::map<std::string, Span> m_remaining;

  /**
   * @brief Every statement collected (includes expanded in their place), with its file, in order.
   */
  std::vector<std::pair<const Stmt*, const Syntax::File*>> m_order;

  /**
   * @brief The label statements (name.label(...)), by the name they label.
   */
  std::map<std::string, std::vector<const Stmt*>> m_labels;

  /**
   * @brief The values, by name.
   */
  std::map<std::string, Defined> m_values;

  /**
   * @brief The value names in the order defined.
   */
  std::vector<std::string> m_valueOrder;

  /**
   * @brief The statements resolved after the object: materials, domains, couplings; and the object and the view.
   */
  std::vector<const Stmt*> m_materials, m_domains, m_couples;

  /**
   * @brief The object's statement, or nullptr.
   */
  const Stmt* m_objectStatement = nullptr;

  /**
   * @brief The view statement, or nullptr.
   */
  const Stmt* m_viewStatement = nullptr;

  /**
   * @brief The object.
   */
  std::shared_ptr<const S> m_object;

  /**
   * @brief Where each material is first used by a region.
   */
  std::map<std::string, Span> m_materialUses;

  /**
   * @brief The domains with errors: what refers to them reports nothing more.
   */
  std::set<std::string> m_failedDomains;

  /**
   * @brief Meshes read, by file and settings: a file is read once per model.
   */
  std::map<std::string, std::shared_ptr<const S>> m_meshes;

  /**
   * @brief For each mesh read, a new leaf of it: each call is a leaf of its own (its own site), the mesh's data shared.
   */
  std::map<std::string, std::function<Expr<T>()>> m_meshLeaves;

  /**
   * @brief Each shape site's object (the shape a leaf holds), to find the site of a leaf.
   */
  std::vector<const void*> m_siteObjects;

  /**
   * @brief The model being made.
   */
  Model<T> m_model;

  /**
   * @brief The object's leaves' objects, depth first, the operands in order: the order ExprIndex numbers leaves in.
   * @param[in]     a_expr    An expression.
   * @param[in,out] a_objects The objects.
   */
  static void
  leafObjects(const Expr<T>& a_expr, std::vector<const void*>& a_objects)
  {
    const ExprDetail::Node<T>& node = a_expr.node();

    if (node.m_kind == ExprDetail::Kind::Primitive) {
      a_objects.push_back(node.m_leaf.m_object.get());

      return;
    }

    for (const Expr<T>& child : node.m_children) {
      leafObjects(child, a_objects);
    }
  }

  /**
   * @brief Add an error.
   * @param[in] a_span    Where.
   * @param[in] a_message What.
   */
  void
  error(const Span& a_span, const std::string& a_message)
  {
    m_diagnostics.push_back(Diagnostic{Diagnostic::Severity::Error, a_span, a_message});
  }

  /**
   * @brief Add a warning.
   * @param[in] a_span    Where.
   * @param[in] a_message What.
   */
  void
  warning(const Span& a_span, const std::string& a_message)
  {
    m_diagnostics.push_back(Diagnostic{Diagnostic::Severity::Warning, a_span, a_message});
  }

  /**
   * @brief A span's place in words, "file:line".
   * @param[in] a_span The span.
   * @return The words.
   */
  static std::string
  place(const Span& a_span)
  {
    return a_span.m_file + ":" + std::to_string(a_span.m_line);
  }

  /**
   * @brief Whether any error was found.
   * @return Whether.
   */
  bool
  failed() const noexcept
  {
    for (const Diagnostic& d : m_diagnostics) {
      if (d.m_severity == Diagnostic::Severity::Error) {
        return true;
      }
    }

    return false;
  }

  /**
   * @brief Load a file and evaluate its statements.
   * @param[in] a_path The path.
   * @param[in] a_from Where it is asked for (an include, or the path itself).
   */
  void
  loadFile(const std::string& a_path, const Span& a_from)
  {
    namespace fs = std::filesystem;

    std::error_code ec;

    const std::string resolved  = fs::weakly_canonical(a_path, ec).string();
    const std::string canonical = ec ? a_path : resolved;

    for (const std::string& reading : m_reading) {
      if (reading == canonical) {
        this->error(a_from, "'" + a_path + "' includes itself, through the files that include it");

        return;
      }
    }

    // A file is read once, however many files include it (by whatever path): a second read would define its names
    // again, and files including one another twice over would be read exponentially many times. So setups that each
    // include the geometry can be included together.
    if (m_read.count(canonical) > 0) {
      return;
    }

    m_read.emplace(canonical, a_from);

    if (!fs::is_regular_file(a_path, ec)) {
      this->error(a_from, fs::exists(a_path, ec) ? "'" + a_path + "' is not a file" : "cannot open '" + a_path + "'");

      return;
    }

    std::ifstream in(a_path, std::ios::binary);

    if (!in) {
      this->error(a_from, "cannot open '" + a_path + "'");

      return;
    }

    std::stringstream text;

    text << in.rdbuf();

    m_model.m_files.push_back(a_path);
    m_reading.push_back(canonical);

    this->loadText(text.str(), a_path);

    m_reading.pop_back();
  }

  /**
   * @brief Parse text and collect its statements, its includes' in their place; label statements also by name.
   * @param[in] a_text The text.
   * @param[in] a_name Its file's name.
   */
  void
  loadText(std::string_view a_text, const std::string& a_name)
  {
    try {
      m_sources.push_back(Syntax::parse(a_text, a_name));
    } catch (const Error& e) {
      m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());

      return;
    }

    if (m_reading.empty()) {
      m_reading.push_back(a_name);
    }

    const Syntax::File& file = m_sources.back();

    for (const Stmt& statement : file.m_statements) {
      if (statement.m_kind == SK::Include) {
        this->loadFile((folderOf(file.m_name) / statement.m_name).string(), statement.m_nameSpan);

        continue;
      }

      if (statement.m_kind == SK::Label) {
        m_labels[statement.m_name].push_back(&statement);
      }

      m_order.emplace_back(&statement, &file);
    }
  }

  /**
   * @brief Evaluate every statement, in the order collected.
   */
  void
  evaluate()
  {
    for (const auto& [statement, file] : m_order) {
      this->apply(*statement, *file);
    }

    // A label statement for a name nothing defines.
    for (const auto& [name, statements] : m_labels) {
      if (m_values.count(name) == 0) {
        for (const Stmt* statement : statements) {
          this->error(statement->m_nameSpan,
                      "no value '" + name + "' to label" + LanguageDetail::suggest(name, m_valueOrder));
        }
      }
    }
  }

  /**
   * @brief The folder of the file being read, for relative paths.
   * @param[in] a_file The file.
   * @return The folder.
   */
  static std::filesystem::path
  folderOf(const std::string& a_file)
  {
    return std::filesystem::path(a_file).parent_path();
  }

  /**
   * @brief Evaluate a statement, or record what is declared for later.
   * @param[in] a_statement The statement.
   * @param[in] a_file      Its file.
   */
  void
  apply(const Stmt& a_statement, const Syntax::File& a_file)
  {
    switch (a_statement.m_kind) {
    case SK::Value: {
      this->defineValue(a_statement, a_file);

      break;
    }
    case SK::Object: {
      if (m_objectStatement != nullptr) {
        this->error(a_statement.m_span,
                    "a model has one object; another is at " + place(m_objectStatement->m_span) +
                      " (combine them with union() or an overlay)");

        break;
      }

      m_objectStatement = &a_statement;

      // A named object is also a value of that name (labels apply to it, later lines use it).
      if (!a_statement.m_name.empty()) {
        this->defineValue(a_statement, a_file);
      }

      try {
        const auto named = m_values.find(a_statement.m_name);

        if (!a_statement.m_name.empty() && (named == m_values.end() || named->second.m_value.m_type == VT::Invalid)) {
          throw LanguageDetail::Poisoned{};
        }

        const V value = a_statement.m_name.empty() ? this->eval(a_statement.m_expr) : named->second.m_value;

        if (value.m_type != VT::Shape) {
          throw Error(a_statement.m_expr.m_span,
                      std::string("the object is a shape, not ") + LanguageDetail::valueTypeName<T>(value.m_type));
        }

        m_object = value.m_shape;
      } catch (const Error& e) {
        m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());
      } catch (const LanguageDetail::Poisoned&) {
      }

      break;
    }
    case SK::Material: {
      this->declare(m_materials, a_statement, "material");

      break;
    }
    case SK::Domain: {
      this->declare(m_domains, a_statement, "domain");

      break;
    }
    case SK::Couple: {
      this->declare(m_couples, a_statement, "coupling");

      break;
    }
    case SK::Include:
    case SK::Label: {
      // Includes were read where they are written; labels apply where their value is defined.
      break;
    }
    case SK::View: {
      if (m_viewStatement != nullptr) {
        this->error(a_statement.m_span, "a model has one view block; another is at " + place(m_viewStatement->m_span));

        break;
      }

      m_viewStatement = &a_statement;

      break;
    }
    default: {
      break;
    }
    }
  }

  /**
   * @brief Record a declaration resolved after the object, refusing a name given twice.
   * @param[in,out] a_list      The declarations of its kind.
   * @param[in]     a_statement The statement.
   * @param[in]     a_kind      Its kind in words.
   */
  void
  declare(std::vector<const Stmt*>& a_list, const Stmt& a_statement, const char* a_kind)
  {
    for (const Stmt* other : a_list) {
      if (other->m_name == a_statement.m_name) {
        this->error(a_statement.m_nameSpan,
                    std::string(a_kind) + " '" + a_statement.m_name + "' is already defined at " +
                      place(other->m_nameSpan));

        return;
      }
    }

    a_list.push_back(&a_statement);
  }

  /**
   * @brief Define a value, or a param (taking its override).
   * @param[in] a_statement The statement.
   * @param[in] a_file      Its file.
   */
  void
  defineValue(const Stmt& a_statement, const Syntax::File& a_file)
  {
    const auto existing = m_values.find(a_statement.m_name);

    if (existing != m_values.end()) {
      this->error(a_statement.m_nameSpan,
                  "'" + a_statement.m_name + "' is already defined at " + place(existing->second.m_span) +
                    " (a name is defined once)");

      return;
    }

    Defined defined;

    defined.m_span = a_statement.m_nameSpan;

    const auto override = m_overrides.find(a_statement.m_name);

    if (override != m_overrides.end()) {
      m_overridesUsed.insert(a_statement.m_name);

      if (!a_statement.m_param) {
        this->error(a_statement.m_nameSpan,
                    "'" + a_statement.m_name + "' is overridden, but it is not a param (write 'param " +
                      a_statement.m_name + " = ...' to allow it)");
      }
    }

    Param param;

    param.m_name  = a_statement.m_name;
    param.m_span  = a_statement.m_nameSpan;
    param.m_value = a_file.m_text.substr(a_statement.m_expr.m_span.m_begin,
                                         a_statement.m_expr.m_span.m_end - a_statement.m_expr.m_span.m_begin);

    try {
      if (a_statement.m_param && override != m_overrides.end()) {
        defined.m_value = this->eval(Syntax::parseExpression(override->second, "-D " + a_statement.m_name));

        param.m_value      = override->second;
        param.m_overridden = true;
      }
      else {
        defined.m_value = this->eval(a_statement.m_expr);
      }
    } catch (const Error& e) {
      m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());

      defined.m_value = V{};
    } catch (const LanguageDetail::Poisoned&) {
      defined.m_value = V{};
    }

    // The value's label statements, wherever they are written.
    const auto labels = m_labels.find(a_statement.m_name);

    if (labels != m_labels.end() && defined.m_value.m_type != VT::Invalid) {
      for (const Stmt* label : labels->second) {
        try {
          if (defined.m_value.m_type != VT::Shape || !defined.m_value.m_shape->m_plain) {
            throw Error(label->m_nameSpan,
                        "'" + a_statement.m_name +
                          "' is not one shape made by a call, whose parts a label statement can name (it is " +
                          (defined.m_value.m_type == VT::Shape
                             ? std::string("a composition")
                             : LanguageDetail::valueTypeName<T>(defined.m_value.m_type)) +
                          ")");
          }

          std::vector<Arg> args{Arg{"", defined.m_value}};

          // The labels apply where the shape is defined, so that every use of it has them: their arguments are read
          // there too.
          for (size_t i = 1; i < label->m_expr.m_children.size(); i++) {
            try {
              args.push_back(Arg{label->m_expr.m_keys[i], this->eval(label->m_expr.m_children[i])});
            } catch (const Error& e) {
              const Diagnostic& first = e.diagnostics().front();

              throw Error(first.m_span,
                          first.m_message + " (a label statement's arguments are read where '" + a_statement.m_name +
                            "' is defined, at " + place(a_statement.m_nameSpan) + ")");
            }
          }

          const Span span        = defined.m_value.m_span;
          defined.m_value        = this->call(label->m_expr, args);
          defined.m_value.m_span = span;
        } catch (const Error& e) {
          m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());
        } catch (const LanguageDetail::Poisoned&) {
        }
      }
    }

    if (a_statement.m_param) {
      param.m_type = defined.m_value.m_type == VT::Invalid ? "invalid" : typeWord(defined.m_value.m_type);

      m_model.m_params.push_back(param);
    }

    m_values.emplace(a_statement.m_name, defined);
    m_valueOrder.push_back(a_statement.m_name);
  }

  /**
   * @brief A type's word in a param listing.
   * @param[in] a_type The type.
   * @return "number", "vector", "string", "bool" or "shape".
   */
  static std::string
  typeWord(const VT a_type)
  {
    const std::string name = LanguageDetail::valueTypeName<T>(a_type);

    return name.substr(name.find(' ') + 1);
  }

  /**
   * @brief Evaluate an expression.
   * @param[in] a_node The expression.
   * @return The value; an Error, or Poisoned when it uses an invalid value.
   */
  V
  eval(const Node& a_node)
  {
    V value;

    value.m_span = a_node.m_span;

    switch (a_node.m_kind) {
    case NK::Number: {
      value.m_type   = VT::Number;
      value.m_number = a_node.m_number;

      return value;
    }
    case NK::String: {
      value.m_type   = VT::String;
      value.m_string = a_node.m_text;

      return value;
    }
    case NK::Bool: {
      value.m_type   = VT::Bool;
      value.m_number = a_node.m_number;

      return value;
    }
    case NK::Name: {
      return this->lookup(a_node);
    }
    case NK::Vector: {
      value.m_type = VT::Vector;

      for (int k = 0; k < 3; k++) {
        const V component = this->eval(a_node.m_children[k]);

        if (component.m_type != VT::Number) {
          throw Error(component.m_span,
                      std::string("a vector's components are numbers, not ") +
                        LanguageDetail::valueTypeName<T>(component.m_type));
        }

        value.m_vector[k] = component.m_number;
      }

      return value;
    }
    case NK::Pair: {
      throw Error(a_node.m_span, "a pair (a, b) is a selector's regions only: label(\"x\", between = (a, b))");
    }
    case NK::Negate: {
      value = this->eval(a_node.m_children[0]);

      value.m_span = a_node.m_span;

      if (value.m_type == VT::Number) {
        value.m_number = -value.m_number;
      }
      else if (value.m_type == VT::Vector) {
        value.m_vector = -value.m_vector;
      }
      else {
        throw Error(a_node.m_span,
                    std::string("only a number or a vector can be negated, not ") +
                      LanguageDetail::valueTypeName<T>(value.m_type));
      }

      return value;
    }
    case NK::Add:
    case NK::Subtract:
    case NK::Multiply:
    case NK::Divide: {
      return this->arithmetic(a_node);
    }
    case NK::Call: {
      std::vector<Arg> args;

      for (size_t i = 0; i < a_node.m_children.size(); i++) {
        args.push_back(Arg{a_node.m_keys[i], this->eval(a_node.m_children[i])});
      }

      return this->call(a_node, args);
    }
    case NK::Method: {
      std::vector<Arg> args;

      for (size_t i = 0; i < a_node.m_children.size(); i++) {
        args.push_back(Arg{a_node.m_keys[i], this->eval(a_node.m_children[i])});
      }

      return this->call(a_node, args);
    }
    case NK::Overlay: {
      return this->overlay(a_node);
    }
    default: {
      throw Error(a_node.m_span, "a region is written inside an overlay");
    }
    }
  }

  /**
   * @brief A name's value.
   * @param[in] a_node The name.
   * @return The value.
   */
  V
  lookup(const Node& a_node)
  {
    if (a_node.m_text == "outside") {
      throw Error(a_node.m_span, "'outside' is a region (region 0), named in domains and selectors, not a value");
    }

    const auto found = m_values.find(a_node.m_text);

    if (found == m_values.end()) {
      throw Error(a_node.m_span,
                  "no value '" + a_node.m_text + "' defined before this line" +
                    LanguageDetail::suggest(a_node.m_text, m_valueOrder));
    }

    if (found->second.m_value.m_type == VT::Invalid) {
      throw LanguageDetail::Poisoned{};
    }

    V value = found->second.m_value;

    value.m_span = a_node.m_span;

    return value;
  }

  /**
   * @brief An arithmetic result, refused when it overflows a double.
   * @param[in] a_value The result.
   * @param[in] a_symbol Its operator.
   * @return It.
   */
  static const V&
  bounded(const V& a_value, const char* a_symbol)
  {
    const bool finite = a_value.m_type == VT::Number
                          ? std::isfinite(a_value.m_number)
                          : (std::isfinite(a_value.m_vector[0]) && std::isfinite(a_value.m_vector[1]) &&
                             std::isfinite(a_value.m_vector[2]));

    if (!finite) {
      throw Error(a_value.m_span, std::string("the result of '") + a_symbol + "' is too large");
    }

    return a_value;
  }

  /**
   * @brief + - * / on numbers and vectors.
   * @param[in] a_node The operation.
   * @return The value.
   */
  V
  arithmetic(const Node& a_node)
  {
    const V a = this->eval(a_node.m_children[0]);
    const V b = this->eval(a_node.m_children[1]);

    V value;

    value.m_span = a_node.m_span;

    const bool an = a.m_type == VT::Number;
    const bool av = a.m_type == VT::Vector;
    const bool bn = b.m_type == VT::Number;
    const bool bv = b.m_type == VT::Vector;

    const char* symbol = "+";

    switch (a_node.m_kind) {
    case NK::Add:
    case NK::Subtract: {
      const double sign = a_node.m_kind == NK::Add ? 1.0 : -1.0;

      symbol = a_node.m_kind == NK::Add ? "+" : "-";

      if (an && bn) {
        value.m_type   = VT::Number;
        value.m_number = a.m_number + sign * b.m_number;

        return this->bounded(value, symbol);
      }
      if (av && bv) {
        value.m_type   = VT::Vector;
        value.m_vector = a.m_vector + sign * b.m_vector;

        return this->bounded(value, symbol);
      }

      break;
    }
    case NK::Multiply: {
      symbol = "*";

      if (an && bn) {
        value.m_type   = VT::Number;
        value.m_number = a.m_number * b.m_number;

        return this->bounded(value, symbol);
      }
      if ((an && bv) || (av && bn)) {
        value.m_type   = VT::Vector;
        value.m_vector = an ? a.m_number * b.m_vector : b.m_number * a.m_vector;

        return this->bounded(value, symbol);
      }

      break;
    }
    default: {
      symbol = "/";

      if (bn && b.m_number == 0.0 && (an || av)) {
        throw Error(b.m_span, "division by zero");
      }
      if (an && bn) {
        value.m_type   = VT::Number;
        value.m_number = a.m_number / b.m_number;

        return this->bounded(value, symbol);
      }
      if (av && bn) {
        value.m_type   = VT::Vector;
        value.m_vector = a.m_vector / b.m_number;

        return this->bounded(value, symbol);
      }

      break;
    }
    }

    throw Error(a_node.m_span,
                std::string("cannot compute ") + LanguageDetail::valueTypeName<T>(a.m_type) + " " + symbol + " " +
                  LanguageDetail::valueTypeName<T>(b.m_type));
  }

  /**
   * @brief A built-in's arguments, matched to its specs and checked.
   * @param[in] a_call  The call.
   * @param[in] a_name  The built-in's name.
   * @param[in] a_specs Its specs.
   * @param[in] a_args  The arguments.
   * @return The matched arguments.
   */
  Bound
  bind(const Node& a_call, const std::string& a_name, const Specs& a_specs, const std::vector<Arg>& a_args) const
  {
    std::string signature = a_name + "(";

    for (size_t i = 0; i < a_specs.size(); i++) {
      signature += std::string(i > 0 ? ", " : "") + (a_specs[i].m_kind == Kind::Shapes ? "shapes..." : "") +
                   (a_specs[i].m_kind == Kind::Shapes ? "" : a_specs[i].m_name) + (a_specs[i].m_optional ? "?" : "");
    }

    signature += ")";

    Bound bound;

    bound.m_values.assign(a_specs.size(), nullptr);

    size_t positional = 0;

    for (const Arg& arg : a_args) {
      if (arg.m_key.empty()) {
        while (positional < a_specs.size() && a_specs[positional].m_kind != Kind::Shapes &&
               bound.m_values[positional] != nullptr) {
          positional++;
        }

        if (positional >= a_specs.size()) {
          throw Error(arg.m_value.m_span, "too many arguments: " + signature);
        }
        if (a_specs[positional].m_kind == Kind::Shapes) {
          bound.m_shapes.push_back(&arg.m_value);
        }
        else {
          bound.m_values[positional++] = &arg.m_value;
        }

        continue;
      }

      bool found = false;

      for (size_t i = 0; i < a_specs.size(); i++) {
        if (a_specs[i].m_kind != Kind::Shapes && arg.m_key == a_specs[i].m_name) {
          if (bound.m_values[i] != nullptr) {
            throw Error(arg.m_value.m_span, "'" + arg.m_key + "' is given twice: " + signature);
          }

          bound.m_values[i] = &arg.m_value;
          found             = true;
        }
      }

      if (!found) {
        throw Error(arg.m_value.m_span, a_name + " has no argument '" + arg.m_key + "': " + signature);
      }
    }

    for (size_t i = 0; i < a_specs.size(); i++) {
      if (a_specs[i].m_kind == Kind::Shapes) {
        for (const V* shape : bound.m_shapes) {
          this->check(*shape, Kind::Shape, a_name, "every operand");
        }

        continue;
      }
      if (bound.m_values[i] == nullptr) {
        if (!a_specs[i].m_optional) {
          throw Error(a_call.m_span, a_name + " needs '" + a_specs[i].m_name + "': " + signature);
        }

        continue;
      }

      this->check(*bound.m_values[i], a_specs[i].m_kind, a_name, std::string("'") + a_specs[i].m_name + "'");
    }

    return bound;
  }

  /**
   * @brief Check an argument's kind.
   * @param[in] a_value The argument.
   * @param[in] a_kind  What it must be.
   * @param[in] a_name  The built-in's name.
   * @param[in] a_what  The argument in words.
   */
  void
  check(const V& a_value, const Kind a_kind, const std::string& a_name, const std::string& a_what) const
  {
    const auto refuse = [&](const char* a_wanted) {
      throw Error(a_value.m_span,
                  a_name + ": " + a_what + " is " + a_wanted + ", not " +
                    LanguageDetail::valueTypeName<T>(a_value.m_type));
    };

    // Arithmetic runs in double; a value is narrowed to T only when a shape takes it, so its range is checked here.
    const auto narrows = [&](const double a_number) {
      if (!(std::abs(a_number) <= double(Math::Limits<T>::max()))) {
        throw Error(a_value.m_span,
                    a_name + ": " + a_what + " is too large for a model read in " +
                      (std::is_same<T, float>::value ? "single" : "double") + " precision");
      }
    };

    switch (a_kind) {
    case Kind::Number: {
      if (a_value.m_type != VT::Number) {
        refuse("a number");
      }

      narrows(a_value.m_number);

      break;
    }
    case Kind::Vector: {
      if (a_value.m_type != VT::Vector) {
        refuse("a vector, (x, y, z)");
      }

      for (size_t dir = 0; dir < 3; dir++) {
        narrows(a_value.m_vector[dir]);
      }

      break;
    }
    case Kind::String: {
      if (a_value.m_type != VT::String) {
        refuse("a string");
      }

      break;
    }
    case Kind::Bool: {
      if (a_value.m_type != VT::Bool) {
        refuse("true or false");
      }

      break;
    }
    case Kind::Shape: {
      if (a_value.m_type != VT::Shape) {
        refuse("a shape");
      }

      break;
    }
    case Kind::Axis: {
      if (a_value.m_type != VT::String ||
          (a_value.m_string != "x" && a_value.m_string != "y" && a_value.m_string != "z")) {
        throw Error(a_value.m_span, a_name + ": " + a_what + " is an axis, \"x\", \"y\" or \"z\"");
      }

      break;
    }
    case Kind::AxisOrVector: {
      if (a_value.m_type != VT::Vector) {
        this->check(a_value, Kind::Axis, a_name, a_what);
      }
      else {
        this->check(a_value, Kind::Vector, a_name, a_what);
      }

      break;
    }
    case Kind::Count: {
      if (a_value.m_type != VT::Number || !(a_value.m_number >= 0.0 && a_value.m_number <= 1.0e6) ||
          std::floor(a_value.m_number) != a_value.m_number) {
        throw Error(a_value.m_span, a_name + ": " + a_what + " is a whole number from 0 to 1000000");
      }

      break;
    }
    default: {
      break;
    }
    }
  }

  /**
   * @brief A number argument.
   * @param[in] a_value The argument.
   * @return It.
   */
  static T
  number(const V* a_value) noexcept
  {
    return T(a_value->m_number);
  }

  /**
   * @brief A vector argument.
   * @param[in] a_value The argument.
   * @return It.
   */
  static Vec3T<T>
  vector(const V* a_value) noexcept
  {
    return Vec3T<T>(T(a_value->m_vector[0]), T(a_value->m_vector[1]), T(a_value->m_vector[2]));
  }

  /**
   * @brief An axis argument.
   * @param[in] a_value The argument ("x", "y" or "z").
   * @return 0, 1 or 2.
   */
  static size_t
  axis(const V* a_value) noexcept
  {
    return static_cast<size_t>(a_value->m_string[0] - 'x');
  }

  /**
   * @brief The blend of a smooth operation.
   * @param[in] a_value The argument, or nullptr for the default.
   * @return The blend.
   */
  static BlendKind
  blend(const V* a_value)
  {
    if (a_value == nullptr || a_value->m_string == "polynomial") {
      return BlendKind::Polynomial;
    }
    if (a_value->m_string == "exponential") {
      return BlendKind::Exponential;
    }

    throw Error(a_value->m_span, "blend is \"polynomial\" or \"exponential\"");
  }

  /**
   * @brief A plain shape: one built-in shape, its parts nameable.
   * @param[in] a_name  The shape's name in the language.
   * @param[in] a_expr  The shape.
   * @param[in] a_parts Its part names.
   * @param[in] a_span  Where the call is.
   * @return The value.
   */
  V
  plain(const std::string& a_name, const Expr<T>& a_expr, std::vector<std::string> a_parts, const Span& a_span)
  {
    auto shape = std::make_shared<S>();

    shape->m_expr  = a_expr;
    shape->m_plain = true;
    shape->m_site  = static_cast<uint32_t>(m_model.m_shapes.size());
    shape->m_parts = std::move(a_parts);

    shape->m_partLabels.assign(shape->m_parts.size(), "");
    shape->m_partLabelSpans.assign(shape->m_parts.size(), Span{});

    m_model.m_shapes.push_back(ShapeSite{
      a_name, shape->m_parts, shape->m_partLabels, std::vector<bool>(shape->m_parts.size(), false), "", a_span});
    m_siteObjects.push_back(a_expr.node().m_leaf.m_object.get());

    V value;

    value.m_type  = VT::Shape;
    value.m_shape = shape;
    value.m_span  = a_span;

    return value;
  }

  /**
   * @brief A composed shape.
   * @param[in] a_expr      The expression.
   * @param[in] a_span      Where.
   * @param[in] a_partition Whether it holds regions.
   * @param[in] a_names     The region and region group names in it.
   * @return The value.
   */
  static V
  composed(const Expr<T>&                            a_expr,
           const Span&                               a_span,
           const bool                                a_partition,
           std::vector<std::pair<std::string, Span>> a_names)
  {
    auto shape = std::make_shared<S>();

    shape->m_expr      = a_expr;
    shape->m_partition = a_partition;
    shape->m_names     = std::move(a_names);

    V value;

    value.m_type  = VT::Shape;
    value.m_shape = shape;
    value.m_span  = a_span;

    return value;
  }

  /**
   * @brief A label added to a site's: one shape used with two labels for the same surfaces lists both.
   * @param[in, out] a_site  The site's label.
   * @param[in]      a_label The label.
   */
  static void
  addLabel(std::string& a_site, const std::string& a_label)
  {
    if (a_site.empty()) {
      a_site = a_label;

      return;
    }

    std::istringstream listed(a_site);

    for (std::string label; std::getline(listed, label, ',');) {
      if (label == a_label) {
        return;
      }
    }

    a_site += "," + a_label;
  }

  /**
   * @brief A shape's expression, its labels applied; and its labels recorded at its site, as it is used.
   * @param[in] a_shape The shape.
   * @return The expression.
   */
  Expr<T>
  realize(const S& a_shape)
  {
    if (!a_shape.m_plain) {
      return *a_shape.m_expr;
    }

    // At its use, not where the label is written: a labeled value never used labels nothing, and two values of one
    // shape labeled differently are both listed.
    ShapeSite& site = m_model.m_shapes[a_shape.m_site];

    for (size_t k = 0; k < a_shape.m_partLabelSpans.size(); k++) {
      if (a_shape.m_partLabelSpans[k].m_line != 0) {
        if (!site.m_partLabeled[k]) {
          site.m_partLabels[k] = "";
        }

        addLabel(site.m_partLabels[k], a_shape.m_partLabels[k]);

        site.m_partLabeled[k] = true;
      }
    }

    if (!a_shape.m_label.empty()) {
      addLabel(site.m_label, a_shape.m_label);
    }

    Expr<T> expr = *a_shape.m_expr;

    bool labeled = false;

    for (const std::string& label : a_shape.m_partLabels) {
      labeled = labeled || !label.empty();
    }

    if (labeled) {
      FaceGroups groups;

      const auto groupOf = [&](const std::string& a_label) -> uint32_t {
        if (a_label.empty()) {
          return FaceGroups::None;
        }

        for (uint32_t g = 0; g < groups.m_names.size(); g++) {
          if (groups.m_names[g] == a_label) {
            return g;
          }
        }

        groups.m_names.push_back(a_label);

        return static_cast<uint32_t>(groups.m_names.size() - 1);
      };

      if (a_shape.m_mesh) {
        for (const uint32_t group : a_shape.m_faceGroup) {
          groups.m_group.push_back(group == FaceGroups::None ? FaceGroups::None : groupOf(a_shape.m_partLabels[group]));
        }
      }
      else {
        for (const std::string& label : a_shape.m_partLabels) {
          groups.m_group.push_back(groupOf(label));
        }
      }

      expr = SurfaceLabel(expr, groups);
    }

    if (!a_shape.m_label.empty()) {
      expr = SurfaceLabel(expr, a_shape.m_label);
    }

    return expr;
  }

  /**
   * @brief An operand's expression, refusing regions where an operation cannot take them.
   * @param[in] a_value     The operand.
   * @param[in] a_operation The operation's name.
   * @param[in] a_allow     Whether it may hold regions here.
   * @return The expression.
   */
  Expr<T>
  operand(const V* a_value, const std::string& a_operation, const bool a_allow)
  {
    if (a_value->m_shape->m_partition && !a_allow) {
      throw Error(a_value->m_span,
                  "an overlay (regions) cannot be an operand of " + a_operation +
                    ": which region a point belongs to would not be clear. An overlay can be the object, moved "
                    "(translate, rotate, scale, reflect), clipped (the first operand of intersection or difference), "
                    "labeled, or a region of another overlay");
    }

    return realize(*a_value->m_shape);
  }

  /**
   * @brief The region names of operands, merged, refusing a name given twice.
   * @param[in] a_operands The operands.
   * @return The names.
   */
  static std::vector<std::pair<std::string, Span>>
  mergeNames(const std::vector<const V*>& a_operands)
  {
    std::vector<std::pair<std::string, Span>> names;

    for (const V* operand : a_operands) {
      for (const auto& name : operand->m_shape->m_names) {
        for (const auto& other : names) {
          if (other.first == name.first) {
            throw Error(
              operand->m_span,
              "region '" + name.first + "' would be in the object twice (named at " + place(other.second) +
                "): a region is one place; write the second as an overlay whose regions have names of their own");
          }
        }

        names.push_back(name);
      }
    }

    return names;
  }

  /**
   * @brief Evaluate a call of a built-in shape or operation, reporting a failed check of the library at the call.
   * @param[in] a_call The call (or method).
   * @param[in] a_args The arguments, the receiver first for a method.
   * @return The value.
   */
  V
  call(const Node& a_call, const std::vector<Arg>& a_args)
  {
    V value;

    try {
      value = this->builtin(a_call, a_args);
    } catch (const RequireError& e) {
      throw Error(a_call.m_span, a_call.m_text + ": " + checkMessage(e));
    }

    std::vector<const V*> operands;

    for (const Arg& arg : a_args) {
      operands.push_back(&arg.m_value);
    }

    const std::string& name = a_call.m_text;

    const bool smoothBVH = name == "smooth_union" && [&] {
      for (const Arg& arg : a_args) {
        if (arg.m_key == "bvh" && arg.m_value.m_type == VT::Bool && arg.m_value.m_number != 0.0) {
          return true;
        }
      }

      return false;
    }();

    const bool calls = name == "blur" || name == "mollify" || name == "repeat" || name == "bvh_union" ||
                       name == "bvh_difference" || smoothBVH;

    return deepened(std::move(value), operands, calls, a_call.m_span);
  }

  /**
   * @brief How deep an expression may be built, through named values too: deeper, its index and the tape's compiler
   * would recurse further than a stack holds.
   */
  static constexpr uint32_t MaxDepth = 4096;

  /**
   * @brief A shape built from operands, given its depth and its calls' depth; an Error past what a tape allows.
   * @param[in] a_value    The shape (another value is returned as it is).
   * @param[in] a_operands The operands (values of other types are passed over).
   * @param[in] a_call     Whether the operation is one a tape evaluates as a call.
   * @param[in] a_span     Where the operation is written.
   * @return The value.
   */
  static V
  deepened(V a_value, const std::vector<const V*>& a_operands, const bool a_call, const Span& a_span)
  {
    if (a_value.m_type != VT::Shape || a_value.m_shape == nullptr) {
      return a_value;
    }

    uint32_t depth = 0;
    uint32_t calls = 0;

    for (const V* operand : a_operands) {
      if (operand->m_type == VT::Shape && operand->m_shape != nullptr) {
        depth = Math::max(depth, operand->m_shape->m_depth);
        calls = Math::max(calls, operand->m_shape->m_calls);
      }
    }

    depth += 1U;
    calls += a_call ? 1U : 0U;

    if (depth > MaxDepth) {
      throw Error(a_span,
                  "the shape is built " + std::to_string(depth) +
                    " operations deep, through the values it uses; at most " + std::to_string(MaxDepth) +
                    " are allowed");
    }

    if (calls > Tape<T>::MaxCallDepth) {
      throw Error(a_span,
                  "blur, mollify, repeat and the BVH operations nest " + std::to_string(calls) +
                    " deep here, through the values they use; a tape allows at most " +
                    std::to_string(Tape<T>::MaxCallDepth));
    }

    // The shape may be an operand returned as it is: it is copied before it is changed.
    if (a_value.m_shape->m_depth != depth || a_value.m_shape->m_calls != calls) {
      auto shape = std::make_shared<S>(*a_value.m_shape);

      shape->m_depth  = depth;
      shape->m_calls  = calls;
      a_value.m_shape = shape;
    }

    return a_value;
  }

  /**
   * @brief A library check's message, without its namespace.
   * @param[in] a_error The failed check.
   * @return The message.
   */
  static std::string
  checkMessage(const RequireError& a_error)
  {
    const std::string message = a_error.what();

    return message.rfind("EBGeometry::", 0) == 0 ? message.substr(12) : message;
  }

  /**
   * @brief Evaluate a call of a built-in shape or operation; the library's checks of its values throw RequireError.
   * @param[in] a_call The call (or method).
   * @param[in] a_args The arguments, the receiver first for a method.
   * @return The value.
   */
  V
  builtin(const Node& a_call, const std::vector<Arg>& a_args)
  {
    const std::string& name = a_call.m_text;
    const Span&        span = a_call.m_span;

    if (m_values.count(name) > 0) {
      throw Error(a_call.m_nameSpan, "'" + name + "' is a value, not an operation");
    }

    // Shapes.
    if (name == "plane") {
      const Bound b = this->bind(a_call, name, {{"point", Kind::Vector}, {"normal", Kind::Vector}}, a_args);

      return this->plain(name, PlaneSDF<T>(vector(b.m_values[0]), vector(b.m_values[1])), {}, span);
    }
    if (name == "sphere") {
      const Bound b = this->bind(a_call, name, {{"center", Kind::Vector}, {"radius", Kind::Number}}, a_args);

      return this->plain(name, SphereSDF<T>(vector(b.m_values[0]), number(b.m_values[1])), {}, span);
    }
    if (name == "box") {
      const Bound b = this->bind(a_call, name, {{"lo_corner", Kind::Vector}, {"hi_corner", Kind::Vector}}, a_args);

      return this->plain(name, BoxSDF<T>(vector(b.m_values[0]), vector(b.m_values[1])), partNames(name), span);
    }
    if (name == "torus") {
      const Bound b =
        this->bind(a_call,
                   name,
                   {{"center", Kind::Vector}, {"major_radius", Kind::Number}, {"minor_radius", Kind::Number}},
                   a_args);

      return this->plain(
        name, TorusSDF<T>(vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2])), {}, span);
    }
    if (name == "cylinder") {
      const Bound b = this->bind(
        a_call, name, {{"center1", Kind::Vector}, {"center2", Kind::Vector}, {"radius", Kind::Number}}, a_args);

      return this->plain(name,
                         CylinderSDF<T>(vector(b.m_values[0]), vector(b.m_values[1]), number(b.m_values[2])),
                         partNames(name),
                         span);
    }
    if (name == "infinite_cylinder") {
      const Bound b =
        this->bind(a_call, name, {{"center", Kind::Vector}, {"radius", Kind::Number}, {"axis", Kind::Axis}}, a_args);

      return this->plain(
        name, InfiniteCylinderSDF<T>(vector(b.m_values[0]), number(b.m_values[1]), axis(b.m_values[2])), {}, span);
    }
    if (name == "capsule") {
      const Bound b =
        this->bind(a_call, name, {{"tip1", Kind::Vector}, {"tip2", Kind::Vector}, {"radius", Kind::Number}}, a_args);

      return this->plain(name,
                         CapsuleSDF<T>(vector(b.m_values[0]), vector(b.m_values[1]), number(b.m_values[2])),
                         partNames(name),
                         span);
    }
    if (name == "infinite_cone") {
      const Bound b = this->bind(a_call, name, {{"tip", Kind::Vector}, {"angle", Kind::Number}}, a_args);

      return this->plain(name, InfiniteConeSDF<T>(vector(b.m_values[0]), number(b.m_values[1])), {}, span);
    }
    if (name == "cone") {
      const Bound b =
        this->bind(a_call, name, {{"tip", Kind::Vector}, {"height", Kind::Number}, {"angle", Kind::Number}}, a_args);

      return this->plain(
        name, ConeSDF<T>(vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2])), partNames(name), span);
    }
    if (name == "rounded_box") {
      const Bound b = this->bind(
        a_call, name, {{"center", Kind::Vector}, {"size", Kind::Vector}, {"curvature", Kind::Number}}, a_args);

      return this->plain(name,
                         RoundedBoxSDF<T>(vector(b.m_values[0]), vector(b.m_values[1]), number(b.m_values[2])),
                         partNames(name),
                         span);
    }
    if (name == "perlin") {
      const Bound b = this->bind(a_call,
                                 name,
                                 {{"noise_amplitude", Kind::Number},
                                  {"noise_frequency", Kind::Vector},
                                  {"noise_persistence", Kind::Number},
                                  {"noise_octaves", Kind::Count}},
                                 a_args);

      return this->plain(name,
                         PerlinSDF<T>(number(b.m_values[0]),
                                      vector(b.m_values[1]),
                                      number(b.m_values[2]),
                                      static_cast<unsigned int>(b.m_values[3]->m_number)),
                         {},
                         span);
    }
    if (name == "rounded_cylinder") {
      const Bound b = this->bind(
        a_call,
        name,
        {{"center", Kind::Vector}, {"radius", Kind::Number}, {"curvature", Kind::Number}, {"height", Kind::Number}},
        a_args);

      return this->plain(name,
                         RoundedCylinderSDF<T>(
                           vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2]), number(b.m_values[3])),
                         partNames(name),
                         span);
    }
    if (name == "ellipsoid") {
      const Bound b = this->bind(a_call, name, {{"center", Kind::Vector}, {"radii", Kind::Vector}}, a_args);

      return this->plain(name, EllipsoidSDF<T>(vector(b.m_values[0]), vector(b.m_values[1])), {}, span);
    }
    if (name == "capped_cone" || name == "round_cone") {
      const Bound b = this->bind(
        a_call,
        name,
        {{"center1", Kind::Vector}, {"center2", Kind::Vector}, {"radius1", Kind::Number}, {"radius2", Kind::Number}},
        a_args);

      const Vec3T<T> c1 = vector(b.m_values[0]);
      const Vec3T<T> c2 = vector(b.m_values[1]);
      const T        r1 = number(b.m_values[2]);
      const T        r2 = number(b.m_values[3]);

      if (name == "capped_cone") {
        return this->plain(name, CappedConeSDF<T>(c1, c2, r1, r2), partNames(name), span);
      }

      return this->plain(name, RoundConeSDF<T>(c1, c2, r1, r2), partNames(name), span);
    }
    if (name == "polygon_prism") {
      const Bound b = this->bind(a_call,
                                 name,
                                 {{"center1", Kind::Vector},
                                  {"center2", Kind::Vector},
                                  {"radius", Kind::Number},
                                  {"sides", Kind::Count},
                                  {"vertex_direction", Kind::Vector}},
                                 a_args);

      const auto sides = static_cast<unsigned>(b.m_values[3]->m_number);

      // Each side is a part with a name: a cap keeps a mistyped count from building millions of them.
      if (sides < 3 || sides > 4096) {
        throw Error(b.m_values[3]->m_span, "polygon_prism: a polygon has from 3 to 4096 sides");
      }

      return this->plain(
        name,
        PolygonPrismSDF<T>(
          vector(b.m_values[0]), vector(b.m_values[1]), number(b.m_values[2]), sides, vector(b.m_values[4])),
        partNames(name, sides),
        span);
    }
    if (name == "tube") {
      const Bound b = this->bind(a_call,
                                 name,
                                 {{"center1", Kind::Vector},
                                  {"center2", Kind::Vector},
                                  {"inner_radius", Kind::Number},
                                  {"outer_radius", Kind::Number}},
                                 a_args);

      return this->plain(
        name,
        TubeSDF<T>(vector(b.m_values[0]), vector(b.m_values[1]), number(b.m_values[2]), number(b.m_values[3])),
        partNames(name),
        span);
    }
    if (name == "capped_torus") {
      const Bound b = this->bind(a_call,
                                 name,
                                 {{"center", Kind::Vector},
                                  {"major_radius", Kind::Number},
                                  {"minor_radius", Kind::Number},
                                  {"angle", Kind::Number}},
                                 a_args);

      return this->plain(
        name,
        CappedTorusSDF<T>(vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2]), number(b.m_values[3])),
        partNames(name),
        span);
    }
    if (name == "cut_sphere") {
      const Bound b = this->bind(
        a_call, name, {{"center", Kind::Vector}, {"radius", Kind::Number}, {"height", Kind::Number}}, a_args);

      return this->plain(name,
                         CutSphereSDF<T>(vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2])),
                         partNames(name),
                         span);
    }
    if (name == "cut_hollow_sphere") {
      const Bound b = this->bind(
        a_call,
        name,
        {{"center", Kind::Vector}, {"radius", Kind::Number}, {"height", Kind::Number}, {"thickness", Kind::Number}},
        a_args);

      return this->plain(name,
                         CutHollowSphereSDF<T>(
                           vector(b.m_values[0]), number(b.m_values[1]), number(b.m_values[2]), number(b.m_values[3])),
                         partNames(name),
                         span);
    }
    if (name == "wedge") {
      const Bound b = this->bind(a_call, name, {{"tip", Kind::Vector}, {"angle", Kind::Number}}, a_args);

      return this->plain(name, WedgeSDF<T>(vector(b.m_values[0]), number(b.m_values[1])), partNames(name), span);
    }
    if (name == "mesh") {
      return this->mesh(a_call, a_args);
    }

    // Operations.
    if (name == "union" || name == "bvh_union" || name == "intersection") {
      const Bound b = this->bind(a_call, name, {{"shapes", Kind::Shapes}}, a_args);

      this->atLeastTwo(a_call, name, b);

      std::vector<Expr<T>> operands;

      for (const V* shape : b.m_shapes) {
        operands.push_back(operand(shape, name, name == "intersection" && operands.empty()));
      }

      const bool partition = name == "intersection" && b.m_shapes[0]->m_shape->m_partition;
      const auto names     = mergeNames(b.m_shapes);

      if (name == "intersection") {
        return composed(Intersection(operands), span, partition, names);
      }

      return composed(Union(operands, name == "bvh_union" ? Accelerate::BVH : Accelerate::None), span, false, names);
    }
    if (name == "smooth_union" || name == "smooth_intersection") {
      const bool unite = name == "smooth_union";

      Specs specs{{"shapes", Kind::Shapes}, {"smooth", Kind::Number}, {"blend", Kind::String, true}};

      if (unite) {
        specs.push_back({"bvh", Kind::Bool, true});
      }

      const Bound b = this->bind(a_call, name, specs, a_args);

      this->atLeastTwo(a_call, name, b);

      std::vector<Expr<T>> operands;

      for (const V* shape : b.m_shapes) {
        operands.push_back(operand(shape, name, false));
      }

      const T         smooth = number(b.m_values[1]);
      const BlendKind kind   = blend(b.m_values[2]);
      const auto      names  = mergeNames(b.m_shapes);

      if (unite) {
        const bool bvh = b.m_values[3] != nullptr && b.m_values[3]->m_number != 0.0;

        return composed(
          SmoothUnion(operands, smooth, kind, bvh ? Accelerate::BVH : Accelerate::None), span, false, names);
      }

      return composed(SmoothIntersection(operands, smooth, kind), span, false, names);
    }
    if (name == "difference" || name == "bvh_difference" || name == "smooth_difference") {
      const bool smooth = name == "smooth_difference";

      Specs specs{{"a", Kind::Shape}, {"shapes", Kind::Shapes}};

      if (smooth) {
        specs.push_back({"smooth", Kind::Number});
        specs.push_back({"blend", Kind::String, true});
      }

      const Bound b = this->bind(a_call, name, specs, a_args);

      if (b.m_shapes.empty()) {
        throw Error(span, name + " takes a shape and one or more shapes to remove from it");
      }

      const Expr<T> first = operand(b.m_values[0], name, name == "difference");

      std::vector<Expr<T>> others;

      for (const V* shape : b.m_shapes) {
        others.push_back(operand(shape, name, false));
      }

      std::vector<const V*> all{b.m_values[0]};

      all.insert(all.end(), b.m_shapes.begin(), b.m_shapes.end());

      const bool partition = name == "difference" && b.m_values[0]->m_shape->m_partition;
      const auto names     = mergeNames(all);

      if (smooth) {
        return composed(
          SmoothDifference(first, others, number(b.m_values[2]), blend(b.m_values[3])), span, false, names);
      }

      return composed(Difference(first, others, name == "bvh_difference" ? Accelerate::BVH : Accelerate::None),
                      span,
                      partition,
                      names);
    }
    if (name == "complement") {
      const Bound b = this->bind(a_call, name, {{"a", Kind::Shape}}, a_args);

      return composed(Complement(operand(b.m_values[0], name, false)), span, false, b.m_values[0]->m_shape->m_names);
    }
    if (name == "translate" || name == "scale" || name == "reflect" || name == "rotate") {
      Specs specs{{"a", Kind::Shape}};

      if (name == "translate") {
        specs.push_back({"shift", Kind::Vector});
      }
      else if (name == "scale") {
        specs.push_back({"factor", Kind::Number});
      }
      else if (name == "reflect") {
        specs.push_back({"axis", Kind::Axis});
      }
      else {
        specs.push_back({"degrees", Kind::Number});
        specs.push_back({"axis", Kind::AxisOrVector});
      }

      const Bound b = this->bind(a_call, name, specs, a_args);

      const Expr<T> a         = operand(b.m_values[0], name, true);
      const bool    partition = b.m_values[0]->m_shape->m_partition;
      const auto&   names     = b.m_values[0]->m_shape->m_names;

      if (name == "translate") {
        return composed(Translate(a, vector(b.m_values[1])), span, partition, names);
      }
      if (name == "scale") {
        if (number(b.m_values[1]) <= T(0)) {
          throw Error(b.m_values[1]->m_span, "scale: the factor is positive");
        }

        return composed(Scale(a, number(b.m_values[1])), span, partition, names);
      }
      if (name == "reflect") {
        return composed(Reflect(a, axis(b.m_values[1])), span, partition, names);
      }
      if (b.m_values[2]->m_type == VT::Vector) {
        return composed(Rotate(a, number(b.m_values[1]), vector(b.m_values[2])), span, partition, names);
      }

      return composed(Rotate(a, number(b.m_values[1]), axis(b.m_values[2])), span, partition, names);
    }
    if (name == "offset" || name == "annular" || name == "elongate") {
      const Kind  kind = name == "elongate" ? Kind::Vector : Kind::Number;
      const char* arg  = name == "offset" ? "offset" : (name == "annular" ? "delta" : "elongation");

      const Bound b = this->bind(a_call, name, {{"a", Kind::Shape}, {arg, kind}}, a_args);

      const Expr<T> a     = operand(b.m_values[0], name, false);
      const auto&   names = b.m_values[0]->m_shape->m_names;

      if (name == "offset") {
        return composed(Offset(a, number(b.m_values[1])), span, false, names);
      }
      if (name == "annular") {
        return composed(Annular(a, number(b.m_values[1])), span, false, names);
      }

      return composed(Elongate(a, vector(b.m_values[1])), span, false, names);
    }
    if (name == "blur") {
      const Bound b =
        this->bind(a_call, name, {{"a", Kind::Shape}, {"blur", Kind::Number}, {"alpha", Kind::Number, true}}, a_args);

      const T alpha = b.m_values[2] != nullptr ? number(b.m_values[2]) : T(0.5);

      return composed(Blur(operand(b.m_values[0], name, false), number(b.m_values[1]), alpha),
                      span,
                      false,
                      b.m_values[0]->m_shape->m_names);
    }
    if (name == "mollify") {
      const Bound b =
        this->bind(a_call, name, {{"a", Kind::Shape}, {"dist", Kind::Number}, {"samples", Kind::Count}}, a_args);

      return composed(Mollify(operand(b.m_values[0], name, false),
                              number(b.m_values[1]),
                              static_cast<size_t>(b.m_values[2]->m_number)),
                      span,
                      false,
                      b.m_values[0]->m_shape->m_names);
    }
    if (name == "repeat") {
      const Bound b = this->bind(
        a_call,
        name,
        {{"a", Kind::Shape}, {"period", Kind::Vector}, {"repeat_lo", Kind::Vector}, {"repeat_hi", Kind::Vector}},
        a_args);

      return composed(
        FiniteRepetition(
          operand(b.m_values[0], name, false), vector(b.m_values[1]), vector(b.m_values[2]), vector(b.m_values[3])),
        span,
        false,
        b.m_values[0]->m_shape->m_names);
    }
    if (name == "label") {
      return this->label(a_call, a_args);
    }

    const std::vector<std::string>& builtins = LanguageDetail::builtinNames();

    const std::string words = LanguageDetail::suggest(name, builtins);

    throw Error(a_call.m_nameSpan, "no shape or operation '" + name + "'" + words.substr(0, words.find(';')));
  }

  /**
   * @brief Refuse a composition of fewer than two shapes.
   * @param[in] a_call The call.
   * @param[in] a_name Its name.
   * @param[in] a_b    Its arguments.
   */
  static void
  atLeastTwo(const Node& a_call, const std::string& a_name, const Bound& a_b)
  {
    if (a_b.m_shapes.size() < 2) {
      throw Error(a_call.m_span, a_name + " takes two shapes or more");
    }
  }

  /**
   * @brief mesh(file, kind = "triangles", groups = true, on_defect = "warn"): read once per model.
   * @param[in] a_call The call.
   * @param[in] a_args The arguments.
   * @return The mesh, a plain shape whose parts are its face groups.
   */
  V
  mesh(const Node& a_call, const std::vector<Arg>& a_args)
  {
    const Bound b = this->bind(a_call,
                               "mesh",
                               {{"file", Kind::String},
                                {"kind", Kind::String, true},
                                {"groups", Kind::Bool, true},
                                {"on_defect", Kind::String, true}},
                               a_args);

    const std::string kind   = b.m_values[1] != nullptr ? b.m_values[1]->m_string : "triangles";
    const bool        groups = b.m_values[2] == nullptr || b.m_values[2]->m_number != 0.0;
    // A mesh with a topology defect is read with a warning unless the model asks for an error: a model is often
    // started from a mesh someone else made.
    const std::string onDefect = b.m_values[3] != nullptr ? b.m_values[3]->m_string : "warn";

    if (kind != "triangles" && kind != "polygons") {
      throw Error(b.m_values[1]->m_span, "mesh: kind is \"triangles\" (TriMeshSDF) or \"polygons\" (MeshSDF)");
    }
    if (onDefect != "throw" && onDefect != "warn") {
      throw Error(b.m_values[3]->m_span, "mesh: on_defect is \"throw\" or \"warn\"");
    }

    const std::string file = b.m_values[0]->m_string;
    const std::string path = (folderOf(a_call.m_span.m_file) / file).string();

    std::error_code ec;

    const std::string canonical = std::filesystem::weakly_canonical(path, ec).string();
    const std::string key       = (ec ? path : canonical) + "|" + kind + "|" + onDefect;

    std::shared_ptr<const S> read;

    const auto cached = m_meshes.find(key);

    if (cached != m_meshes.end()) {
      read = cached->second;
    }
    else {
      const auto defect = onDefect == "warn" ? EBGeometry::Parser::OnDefect::Warn : EBGeometry::Parser::OnDefect::Throw;

      auto shape = std::make_shared<S>();

      try {
        const PolygonSoup<T> soup = EBGeometry::Parser::readIntoPolygonSoup<T>(path);

        // Its parts are its face groups: their names made names of the language (letters, digits and underscores), so
        // that .label(part = "x") can name each; their labels by default are the groups' own names.
        shape->m_groupNames = soup.getFaceGroups().m_names;
        shape->m_faceGroup  = soup.getFaceGroups().m_group;

        for (const std::string& group : shape->m_groupNames) {
          std::string part;

          for (const char c : group) {
            part += (std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_') ? c : '_';
          }

          if (part.empty() || std::isdigit(static_cast<unsigned char>(part[0])) != 0 || Syntax::isReserved(part)) {
            part = "_" + part;
          }

          while (std::find(shape->m_parts.begin(), shape->m_parts.end(), part) != shape->m_parts.end()) {
            part += "_";
          }

          shape->m_parts.push_back(part);
        }

        // The SDF is built from the soup read above, as Parser::readIntoTriMeshSDF() and readIntoMeshSDF() would:
        // reading the file again would double the time, and a file saved in between could give other faces than
        // the groups.
        using Triangles = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;
        using Polygons  = MeshSDF<T, BVH::DefaultBranchingRatio<T>()>;

        if (kind == "triangles") {
          const Triangles sdf(
            soup.convertToTriangles(defect), m_pool, BVH::Construction::SAH, Triangles::defaultConstructionOptions(4));

          m_meshLeaves[key] = [sdf] { return toExpr(sdf); };
        }
        else {
          const Polygons sdf(
            soup.convertToDCEL(m_pool, defect), m_pool, BVH::Construction::SAH, Polygons::defaultConstructionOptions());

          m_meshLeaves[key] = [sdf] { return toExpr(sdf); };
        }
      } catch (const std::exception& e) {
        throw Error(a_call.m_span, std::string("mesh: ") + e.what());
      }

      m_model.m_files.push_back(path);

      read          = shape;
      m_meshes[key] = shape;
    }

    V value = this->plain("mesh", m_meshLeaves.at(key)(), read->m_parts, a_call.m_span);

    auto shape = std::make_shared<S>(*value.m_shape);

    shape->m_mesh      = true;
    shape->m_faceGroup = read->m_faceGroup;

    if (groups) {
      shape->m_partLabels = read->m_groupNames;

      m_model.m_shapes[shape->m_site].m_partLabels = shape->m_partLabels;
    }

    value.m_shape = shape;

    return value;
  }

  /**
   * @brief label(a, "name") names a shape's surfaces; label(a, part = "name", ...) names a plain shape's parts.
   * @param[in] a_call The call.
   * @param[in] a_args The arguments.
   * @return The labeled shape.
   */
  V
  label(const Node& a_call, const std::vector<Arg>& a_args)
  {
    if (a_args.empty() || a_args[0].m_value.m_type != VT::Shape || !a_args[0].m_key.empty()) {
      throw Error(a_call.m_span, "label takes a shape, then a name, part = name pairs, or both");
    }

    const V&    receiver = a_args[0].m_value;
    std::string whole;

    std::vector<std::pair<std::string, const V*>> parts;

    for (size_t i = 1; i < a_args.size(); i++) {
      const V& value = a_args[i].m_value;

      if (value.m_type != VT::String || value.m_string.empty()) {
        throw Error(value.m_span, "label: a label is a name in quotes, not empty");
      }
      if (a_args[i].m_key.empty()) {
        if (!whole.empty()) {
          throw Error(value.m_span, "label: a shape takes one label of its own (its parts take part = name)");
        }

        whole = value.m_string;
      }
      else {
        parts.emplace_back(a_args[i].m_key, &value);
      }
    }

    if (whole.empty() && parts.empty()) {
      throw Error(a_call.m_span, "label takes a name, part = name pairs, or both");
    }

    const S& shape = *receiver.m_shape;

    if (!parts.empty() && !shape.m_plain) {
      throw Error(parts[0].second->m_span,
                  "label: only one shape's own parts can be named; name them on the shape, before it is moved or "
                  "combined");
    }

    if (shape.m_plain) {
      auto labeled = std::make_shared<S>(shape);

      for (const auto& [part, value] : parts) {
        const auto at = std::find(shape.m_parts.begin(), shape.m_parts.end(), part);

        if (at == shape.m_parts.end()) {
          const std::string& site = m_model.m_shapes[shape.m_site].m_shape;

          throw Error(
            value->m_span,
            shape.m_parts.empty()
              ? "label: " + site + " has no parts to name" + (shape.m_mesh ? " (its file names no face groups)" : "")
              : "label: " + site + " has no part '" + part + "'" + LanguageDetail::suggest(part, shape.m_parts));
        }

        const size_t k = static_cast<size_t>(at - shape.m_parts.begin());

        // A part labeled twice (a mesh's face group is not labeled in the text, and may be).
        if (labeled->m_partLabelSpans[k].m_line != 0) {
          throw Error(value->m_span,
                      "label: part '" + part + "' is already labeled \"" + labeled->m_partLabels[k] + "\" at " +
                        place(labeled->m_partLabelSpans[k]));
        }

        labeled->m_partLabels[k]     = value->m_string;
        labeled->m_partLabelSpans[k] = value->m_span;
      }

      if (!whole.empty()) {
        if (!labeled->m_label.empty()) {
          throw Error(a_call.m_span,
                      "label: the shape is already labeled \"" + labeled->m_label + "\" at " +
                        place(labeled->m_labelSpan));
        }

        labeled->m_label     = whole;
        labeled->m_labelSpan = a_call.m_span;
      }

      V value = receiver;

      value.m_shape = labeled;
      value.m_span  = a_call.m_span;

      return value;
    }

    return composed(SurfaceLabel(*shape.m_expr, whole), a_call.m_span, shape.m_partition, shape.m_names);
  }

  /**
   * @brief overlay { region name = shape [material name] ... }.
   * @param[in] a_node The overlay.
   * @return The partition.
   */
  V
  overlay(const Node& a_node)
  {
    std::vector<Expr<T>>                      operands;
    std::vector<std::pair<std::string, Span>> names;
    std::vector<V>                            shapes;

    for (const Node& region : a_node.m_children) {
      const V value = this->eval(region.m_children[0]);

      if (value.m_type != VT::Shape) {
        throw Error(value.m_span,
                    "region '" + region.m_text + "' is a shape, not " + LanguageDetail::valueTypeName<T>(value.m_type));
      }

      for (const auto& other : names) {
        if (other.first == region.m_text) {
          throw Error(region.m_nameSpan, "region '" + region.m_text + "' is already named at " + place(other.second));
        }
      }

      names.emplace_back(region.m_text, region.m_nameSpan);

      for (const auto& name : value.m_shape->m_names) {
        for (const auto& other : names) {
          if (other.first == name.first) {
            throw Error(
              value.m_span,
              "region '" + name.first + "' (named at " + place(name.second) +
                ") would be in the object twice: a region is one place; write the second as an overlay whose regions have names of their own");
          }
        }

        names.push_back(name);
      }

      std::optional<Expr<T>> expr;

      try {
        expr = RegionName(realize(*value.m_shape), region.m_text);

        if (!region.m_material.empty()) {
          expr = MaterialLabel(*expr, region.m_material);

          m_materialUses.emplace(region.m_material, region.m_materialSpan);
        }
      } catch (const RequireError& e) {
        throw Error(region.m_span, "region '" + region.m_text + "': " + checkMessage(e));
      }

      operands.push_back(*expr);
      shapes.push_back(value);
    }

    V value;

    try {
      value = composed(Overlay(operands), a_node.m_span, true, names);
    } catch (const RequireError& e) {
      throw Error(a_node.m_span, "overlay: " + checkMessage(e));
    }

    std::vector<const V*> regions;

    for (const V& shape : shapes) {
      regions.push_back(&shape);
    }

    return deepened(std::move(value), regions, false, a_node.m_span);
  }

  /**
   * @brief Finish: check what the whole model needs, build the index, resolve materials, domains, couplings and the
   * view.
   * @return The model; an Error with every diagnostic when it has errors.
   */
  Model<T>
  finish()
  {
    for (const auto& [name, value] : m_overrides) {
      if (m_overridesUsed.count(name) == 0) {
        this->error(Span{"-D " + name, 0, 0, 0, 0},
                    "the model has no param '" + name + "'" + LanguageDetail::suggest(name, this->paramNames()));
      }
    }

    if (m_objectStatement == nullptr && !this->failed()) {
      const Span where = m_sources.empty() ? Span{} : Span{m_sources[0].m_name, 0, 0, 0, 0};

      this->error(where, "the model has no object: write 'object <shape>'");
    }

    this->stopOnErrors();

    try {
      m_model.m_object = realize(*m_object);
      m_model.m_index  = std::make_shared<const ExprIndex<T>>(*m_model.m_object);

      // Each leaf's shape site, by the object it holds (a mesh read once and used twice: its first site).
      std::vector<const void*> objects;

      leafObjects(*m_model.m_object, objects);

      for (const void* object : objects) {
        const auto site = std::find(m_siteObjects.begin(), m_siteObjects.end(), object);

        m_model.m_leafSites.push_back(
          site == m_siteObjects.end() ? ShapeSite::None : static_cast<uint32_t>(site - m_siteObjects.begin()));
      }
    } catch (const RequireError& e) {
      this->error(m_objectStatement->m_span, "the object: " + checkMessage(e));
    }

    this->stopOnErrors();

    this->resolveMaterials();
    this->resolveDomains();
    this->resolveCouplings();

    if (m_viewStatement != nullptr) {
      m_model.m_view = this->properties(m_viewStatement->m_properties, "the view", m_viewStatement->m_span);

      this->checkView();
    }

    this->stopOnErrors();

    for (const Diagnostic& d : m_diagnostics) {
      m_model.m_warnings.push_back(d);
    }

    m_model.m_sources.assign(m_sources.begin(), m_sources.end());

    return std::move(m_model);
  }

  /**
   * @brief Throw every diagnostic if any is an error.
   */
  void
  stopOnErrors()
  {
    if (this->failed()) {
      std::stable_sort(m_diagnostics.begin(), m_diagnostics.end(), [](const Diagnostic& a, const Diagnostic& b) {
        return a.m_severity == Diagnostic::Severity::Error && b.m_severity != Diagnostic::Severity::Error;
      });

      throw Error(m_diagnostics);
    }
  }

  /**
   * @brief The params' names.
   * @return They.
   */
  std::vector<std::string>
  paramNames() const
  {
    std::vector<std::string> names;

    for (const Param& p : m_model.m_params) {
      names.push_back(p.m_name);
    }

    return names;
  }

  /**
   * @brief A properties block's values.
   * @param[in] a_block The block.
   * @param[in] a_owner Its owner in words.
   * @param[in] a_span  Where its owner is.
   * @return The properties.
   */
  Properties
  properties(const Syntax::Properties& a_block, const std::string& a_owner, const Span& a_span)
  {
    Properties properties(a_owner, a_span);

    for (const Syntax::Property& entry : a_block.m_entries) {
      if (properties.has(entry.m_key)) {
        this->error(entry.m_keySpan, a_owner + ": '" + entry.m_key + "' is given twice");

        continue;
      }

      try {
        const V value = this->eval(entry.m_value);

        Properties::Value p;

        p.m_span = value.m_span;

        switch (value.m_type) {
        case VT::Number: {
          p.m_type   = Properties::Type::Number;
          p.m_number = value.m_number;

          break;
        }
        case VT::Vector: {
          p.m_type   = Properties::Type::Vector;
          p.m_vector = value.m_vector;

          break;
        }
        case VT::String: {
          p.m_type   = Properties::Type::String;
          p.m_string = value.m_string;

          break;
        }
        case VT::Bool: {
          p.m_type   = Properties::Type::Bool;
          p.m_number = value.m_number;

          break;
        }
        default: {
          throw Error(value.m_span, a_owner + ": a property is a number, a vector, a string or a bool, not a shape");
        }
        }

        properties.set(entry.m_key, p);
      } catch (const Error& e) {
        m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());
      } catch (const LanguageDetail::Poisoned&) {
      }
    }

    return properties;
  }

  /**
   * @brief The materials: each name the object's regions use, with its declared properties.
   */
  void
  resolveMaterials()
  {
    const ExprIndex<T>& index = *m_model.m_index;

    for (uint32_t m = 0; m < index.getNumMaterialLabels(); m++) {
      const std::string& name = index.getMaterialLabelName(MaterialLabelId(m));

      const Stmt* declared = nullptr;

      for (const Stmt* statement : m_materials) {
        if (statement->m_name == name) {
          declared = statement;
        }
      }

      if (declared == nullptr) {
        const auto use = m_materialUses.find(name);

        this->warning(use != m_materialUses.end() ? use->second : Span{},
                      "material '" + name + "' is not declared: it has no properties (write 'material " + name +
                        " { ... }')");

        m_model.m_materials.emplace_back("material '" + name + "'", use != m_materialUses.end() ? use->second : Span{});
      }
      else {
        m_model.m_materials.push_back(
          this->properties(declared->m_properties, "material '" + name + "'", declared->m_nameSpan));
      }
    }

    for (const Stmt* statement : m_materials) {
      if (index.findMaterialLabel(statement->m_name).isNone()) {
        this->warning(statement->m_nameSpan, "material '" + statement->m_name + "' is declared but no region uses it");

        // Its properties are still checked.
        (void)this->properties(statement->m_properties, "material '" + statement->m_name + "'", statement->m_nameSpan);
      }
    }
  }

  /**
   * @brief Every name a region reference may be: the regions' names, the region groups', and `outside`.
   * @return The names.
   */
  std::vector<std::string>
  regionNames() const
  {
    const ExprIndex<T>& index = *m_model.m_index;

    std::vector<std::string> names{"outside"};

    for (uint32_t r = 0; r < index.getNumRegions(); r++) {
      if (!index.getRegion(r).m_name.empty()) {
        names.push_back(index.getRegion(r).m_name);
      }
    }

    for (uint32_t g = 0; g < index.getNumRegionGroups(); g++) {
      if (!index.getRegionGroup(g).m_name.empty()) {
        names.push_back(index.getRegionGroup(g).m_name);
      }
    }

    return names;
  }

  /**
   * @brief The regions a name means: a region, all of a region group's, or region 0 for `outside`.
   * @param[in] a_name The name.
   * @param[in] a_span Where it is written.
   * @return The regions.
   */
  std::vector<uint32_t>
  regionsOf(const std::string& a_name, const Span& a_span) const
  {
    const ExprIndex<T>& index = *m_model.m_index;

    if (a_name == "outside") {
      return {0};
    }

    const uint32_t region = index.findRegion(a_name);

    if (region < index.getNumRegions()) {
      return {region};
    }

    for (uint32_t g = 0; g < index.getNumRegionGroups(); g++) {
      if (index.getRegionGroup(g).m_name == a_name) {
        return index.getRegionsOfRegionGroup(g);
      }
    }

    throw Error(a_span, "no region '" + a_name + "'" + LanguageDetail::suggest(a_name, this->regionNames()));
  }

  /**
   * @brief The domains: subdomains with their regions and properties, surface groups with their selectors.
   */
  void
  resolveDomains()
  {
    for (const Stmt* statement : m_domains) {
      ModelDomain domain;

      domain.m_name = statement->m_name;
      domain.m_span = statement->m_nameSpan;

      std::map<std::string, Span> itemNames;

      bool ok = true;

      for (const Syntax::DomainItem& item : statement->m_items) {
        const auto clash = itemNames.find(item.m_name);

        if (clash != itemNames.end()) {
          this->error(item.m_nameSpan,
                      "'" + item.m_name + "' is already a subdomain or group of domain '" + domain.m_name + "', at " +
                        place(clash->second));

          ok = false;

          continue;
        }

        itemNames.emplace(item.m_name, item.m_nameSpan);

        try {
          if (item.m_subdomain) {
            this->subdomain(domain, item);
          }
          else {
            this->group(domain, item);
          }
        } catch (const Error& e) {
          m_diagnostics.insert(m_diagnostics.end(), e.diagnostics().begin(), e.diagnostics().end());

          ok = false;
        } catch (const LanguageDetail::Poisoned&) {
          ok = false;
        }
      }

      if (ok) {
        m_model.m_domains.push_back(std::move(domain));
      }
      else {
        m_failedDomains.insert(statement->m_name);
      }
    }
  }

  /**
   * @brief A subdomain: its regions, and its properties over its regions' materials'.
   * @param[in,out] a_domain The domain.
   * @param[in]     a_item   The subdomain's line.
   */
  void
  subdomain(ModelDomain& a_domain, const Syntax::DomainItem& a_item)
  {
    const ExprIndex<T>& index = *m_model.m_index;

    std::vector<uint32_t> regions;

    for (const Node& name : a_item.m_list) {
      for (const uint32_t r : this->regionsOf(name.m_text, name.m_span)) {
        const uint32_t other = a_domain.m_setup.subdomainOf(r);

        if (other != Subdomains::None) {
          throw Error(name.m_span,
                      "region '" + this->regionName(r) + "' is already in subdomain '" +
                        a_domain.m_subdomainNames[other] + "' of domain '" + a_domain.m_name + "'");
        }

        regions.push_back(r);
      }
    }

    const std::string owner = "subdomain '" + a_item.m_name + "' of domain '" + a_domain.m_name + "'";

    Properties own = this->properties(a_item.m_properties, owner, a_item.m_span);

    // The regions' materials give what the subdomain does not set; where they differ on a key, it must set it.
    Properties merged = own;

    std::map<std::string, std::pair<Properties::Value, std::string>> fromMaterials;

    for (const uint32_t r : regions) {
      const MaterialLabelId material = index.getMaterialOf(r);

      if (material.isNone()) {
        continue;
      }

      const Properties&  properties = m_model.m_materials[material.value()];
      const std::string& name       = index.getMaterialLabelName(material);

      for (const std::string& key : properties.keys()) {
        if (own.has(key)) {
          continue;
        }

        const Properties::Value& value = *properties.find(key);
        const auto               seen  = fromMaterials.find(key);

        if (seen == fromMaterials.end()) {
          fromMaterials.emplace(key, std::make_pair(value, name));

          merged.set(key, value);
        }
        else if (!LanguageDetail::same(seen->second.first, value)) {
          throw Error(a_item.m_span,
                      owner + ": its regions' materials '" + seen->second.second + "' and '" + name + "' differ on '" +
                        key + "'; set it on the subdomain");
        }
      }
    }

    a_domain.m_setup.addSubdomain(regions);
    a_domain.m_subdomainNames.push_back(a_item.m_name);
    a_domain.m_subdomainProperties.push_back(merged);
    a_domain.m_subdomainSpans.push_back(a_item.m_span);
  }

  /**
   * @brief A region's name for messages.
   * @param[in] a_region The region.
   * @return Its name, `outside`, or "region N".
   */
  std::string
  regionName(const uint32_t a_region) const
  {
    if (a_region == 0) {
      return "outside";
    }

    const std::string& name = m_model.m_index->getRegion(a_region).m_name;

    return name.empty() ? "region " + std::to_string(a_region) : name;
  }

  /**
   * @brief A surface group: its selectors.
   * @param[in,out] a_domain The domain.
   * @param[in]     a_item   The group's line.
   */
  void
  group(ModelDomain& a_domain, const Syntax::DomainItem& a_item)
  {
    const ExprIndex<T>& index = *m_model.m_index;

    std::vector<SurfaceSelector> selectors;

    for (const Node& call : a_item.m_list) {
      const auto arity = [&](const size_t a_min, const size_t a_max, const char* a_form) {
        if (call.m_children.size() < a_min || call.m_children.size() > a_max) {
          throw Error(call.m_span, std::string("a selector: ") + a_form);
        }

        for (const std::string& key : call.m_keys) {
          if (!key.empty()) {
            throw Error(call.m_span, std::string("a selector takes no keywords: ") + a_form);
          }
        }
      };

      const auto nameOf = [&](const Node& a_arg, const char* a_what) -> const std::string& {
        if (a_arg.m_kind != NK::Name) {
          throw Error(a_arg.m_span, std::string("expected ") + a_what + " by name");
        }

        return a_arg.m_text;
      };

      if (call.m_text == "label") {
        const char* form = "label(\"name\") or label(\"name\", between = (region, region))";

        // The name first; between = (a, b) narrows it to the surfaces between two regions.
        const Node* between = nullptr;

        for (size_t k = 0; k < call.m_children.size(); k++) {
          if (call.m_keys[k] == "between" && between == nullptr && k > 0) {
            between = &call.m_children[k];
          }
          else if (k > 0 || !call.m_keys[k].empty()) {
            throw Error(call.m_children[k].m_span, std::string("a selector: ") + form);
          }
        }

        if (call.m_children.empty()) {
          throw Error(call.m_span, std::string("a selector: ") + form);
        }

        const V value = this->eval(call.m_children[0]);

        if (value.m_type != VT::String) {
          throw Error(value.m_span, "label(\"name\"): the label is a string");
        }

        const SurfaceLabelId label = index.findSurfaceLabel(value.m_string);

        if (label.isNone()) {
          std::vector<std::string> labels;

          for (uint32_t l = 0; l < index.getNumSurfaceLabels(); l++) {
            labels.push_back(index.getSurfaceLabelName(SurfaceLabelId(l)));
          }

          throw Error(value.m_span,
                      "no surface label \"" + value.m_string + "\"" + LanguageDetail::suggest(value.m_string, labels));
        }

        if (between == nullptr) {
          selectors.push_back(SurfaceSelector::label(label));
        }
        else {
          if (between->m_kind != NK::Pair) {
            throw Error(between->m_span, "between is a pair of regions: between = (a, b)");
          }

          const auto first = this->regionsOf(nameOf(between->m_children[0], "a region"), between->m_children[0].m_span);
          const auto second =
            this->regionsOf(nameOf(between->m_children[1], "a region"), between->m_children[1].m_span);

          const size_t before = selectors.size();

          for (const uint32_t a : first) {
            for (const uint32_t b : second) {
              if (a != b) {
                SurfaceSelector selector = SurfaceSelector::label(label);

                selector.m_regionA = a;
                selector.m_regionB = b;

                selectors.push_back(selector);
              }
            }
          }

          if (selectors.size() == before) {
            this->warning(between->m_span, "this label selects nothing: a region has no interface with itself");
          }
        }
      }
      else if (call.m_text == "interface") {
        arity(2, 2, "interface(region, region)");

        const auto first  = this->regionsOf(nameOf(call.m_children[0], "a region"), call.m_children[0].m_span);
        const auto second = this->regionsOf(nameOf(call.m_children[1], "a region"), call.m_children[1].m_span);

        const size_t before = selectors.size();

        for (const uint32_t a : first) {
          for (const uint32_t b : second) {
            if (a != b) {
              selectors.push_back(SurfaceSelector::interface(a, b));
            }
          }
        }

        if (selectors.size() == before) {
          this->warning(call.m_span, "this interface selects nothing: a region has no interface with itself");
        }
      }
      else if (call.m_text == "region") {
        arity(1, 1, "region(name)");

        for (const uint32_t r : this->regionsOf(nameOf(call.m_children[0], "a region"), call.m_children[0].m_span)) {
          selectors.push_back(SurfaceSelector::region(r));
        }
      }
      else if (call.m_text == "materials") {
        arity(1, 2, "materials(material) or materials(material, material)");

        MaterialLabelId ids[2] = {MaterialLabelId::none(), MaterialLabelId::none()};

        for (size_t k = 0; k < call.m_children.size(); k++) {
          const std::string& name = nameOf(call.m_children[k], "a material");

          ids[k] = index.findMaterialLabel(name);

          if (ids[k].isNone()) {
            std::vector<std::string> materials;

            for (uint32_t m = 0; m < index.getNumMaterialLabels(); m++) {
              materials.push_back(index.getMaterialLabelName(MaterialLabelId(m)));
            }

            throw Error(call.m_children[k].m_span,
                        "no material '" + name + "' in the object" + LanguageDetail::suggest(name, materials));
          }
        }

        selectors.push_back(SurfaceSelector::materials(ids[0], ids[1]));
      }
      else if (call.m_text == "remaining") {
        arity(0, 0, "remaining()");

        // The rest is one group's: in a second, which group a surface is in would not be clear.
        const auto first = m_remaining.emplace(a_domain.m_name, call.m_span);

        if (!first.second) {
          throw Error(call.m_span,
                      "remaining() is already in a group of domain '" + a_domain.m_name + "', at " +
                        place(first.first->second) + ": the rest of the surfaces is one group's");
        }

        selectors.push_back(SurfaceSelector::rest());
      }
      else {
        throw Error(call.m_nameSpan,
                    "no selector '" + call.m_text +
                      "': label(\"x\"), interface(a, b), region(a), materials(m1, m2), remaining()");
      }
    }

    const std::string owner = "group '" + a_item.m_name + "' of domain '" + a_domain.m_name + "'";

    const uint32_t g = a_domain.m_setup.addSurfaceGroup();

    for (const SurfaceSelector& selector : selectors) {
      a_domain.m_setup.add(g, selector);
    }

    a_domain.m_groupNames.push_back(a_item.m_name);
    a_domain.m_groupProperties.push_back(this->properties(a_item.m_properties, owner, a_item.m_span));
    a_domain.m_groupSpans.push_back(a_item.m_span);
  }

  /**
   * @brief Warn about what a viewer would pass over in the view block: a key it does not read, and a color for a name
   * nothing in the model has.
   * @details A viewer reads the keys below and ignores the rest, so a misspelled key would change nothing without a
   * word; warnings, since programs reading the model ignore the block. A key of the form NAME.color, or with "color"
   * misspelled before or after the dot, is pointed to color.NAME.
   */
  void
  checkView()
  {
    static const std::vector<std::string> keys{"camera",
                                               "target",
                                               "up",
                                               "yaw",
                                               "pitch",
                                               "distance",
                                               "field_of_view",
                                               "orthographic",
                                               "background",
                                               "sky",
                                               "floor"};

    const ExprIndex<T>& index = *m_model.m_index;

    // Every name a color may be given to: the regions' and region groups', the materials', the surface labels' and
    // the surface groups'.
    std::vector<std::string> names = this->regionNames();

    for (uint32_t m = 0; m < index.getNumMaterialLabels(); m++) {
      names.push_back(index.getMaterialLabelName(MaterialLabelId(m)));
    }

    for (uint32_t l = 0; l < index.getNumSurfaceLabels(); l++) {
      names.push_back(index.getSurfaceLabelName(SurfaceLabelId(l)));
    }

    for (const ModelDomain& domain : m_model.m_domains) {
      for (uint32_t g = 0; g < domain.getNumSurfaceGroups(); g++) {
        names.push_back(domain.surfaceGroupName(g));
      }
    }

    const Properties& view = m_model.m_view;

    for (const std::string& key : view.keys()) {
      const Span& where = view.find(key)->m_span;

      if (key.rfind("color.", 0) == 0) {
        const std::string name = key.substr(6);

        if (std::find(names.begin(), names.end(), name) == names.end()) {
          this->warning(where,
                        "view: '" + key +
                          "' colors nothing: no region, material, surface label or surface group is "
                          "named '" +
                          name + "'" + LanguageDetail::suggest(name, names));
        }

        continue;
      }

      if (std::find(keys.begin(), keys.end(), key) != keys.end()) {
        continue;
      }

      // NAME.color, or "color" misspelled on either side of the dot: a color meant for NAME.
      const size_t dot = key.find('.');

      if (dot != std::string::npos && key.find('.', dot + 1) == std::string::npos) {
        const std::string first  = key.substr(0, dot);
        const std::string second = key.substr(dot + 1);

        const bool        colorAfter = LanguageDetail::editDistance(second, "color") <= 2U;
        const bool        colorFirst = !colorAfter && LanguageDetail::editDistance(first, "color") <= 2U;
        const std::string name       = colorAfter ? first : second;

        if (colorAfter || colorFirst) {
          const bool named = std::find(names.begin(), names.end(), name) != names.end();

          this->warning(
            where,
            "view: '" + key + "' is no view key (did you mean 'color." + name + "'?)" +
              (named ? std::string() : "; and nothing is named '" + name + "'" + LanguageDetail::suggest(name, names)));

          continue;
        }
      }

      this->warning(where,
                    "view: '" + key + "' is no view key" + LanguageDetail::suggest(key, keys) + ", or color.NAME");
    }
  }

  /**
   * @brief The couplings: ends that exist and are of the right kind; a volume coupling's subdomains hold the same
   * regions (a surface coupling is checked against the pieces, by completeness()).
   */
  void
  resolveCouplings()
  {
    for (const Stmt* statement : m_couples) {
      Coupling coupling;

      coupling.m_name    = statement->m_name;
      coupling.m_kind    = statement->m_coupleKind == "volume" ? Coupling::Kind::Volume : Coupling::Kind::Surface;
      coupling.m_domainA = statement->m_ends[0];
      coupling.m_partA   = statement->m_ends[1];
      coupling.m_domainB = statement->m_ends[2];
      coupling.m_partB   = statement->m_ends[3];
      coupling.m_span    = statement->m_span;

      const ModelDomain* domains[2] = {nullptr, nullptr};
      uint32_t           parts[2]   = {0, 0};

      bool ok = m_failedDomains.count(coupling.m_domainA) == 0 && m_failedDomains.count(coupling.m_domainB) == 0;

      for (int end = 0; end < 2 && ok; end++) {
        const std::string& d = statement->m_ends[2 * end];
        const std::string& p = statement->m_ends[2 * end + 1];

        std::vector<std::string> names;

        for (const ModelDomain& domain : m_model.m_domains) {
          names.push_back(domain.name());

          if (domain.name() == d) {
            domains[end] = &domain;
          }
        }

        if (domains[end] == nullptr) {
          this->error(statement->m_endSpans[2 * end], "no domain '" + d + "'" + LanguageDetail::suggest(d, names));

          ok = false;

          break;
        }

        const bool volume = coupling.m_kind == Coupling::Kind::Volume;

        parts[end] = volume ? domains[end]->findSubdomain(p) : domains[end]->findSurfaceGroup(p);

        if (parts[end] == Subdomains::None) {
          std::vector<std::string> known;

          const uint32_t count = volume ? domains[end]->getNumSubdomains() : domains[end]->getNumSurfaceGroups();

          for (uint32_t i = 0; i < count; i++) {
            known.push_back(volume ? domains[end]->subdomainName(i) : domains[end]->surfaceGroupName(i));
          }

          this->error(statement->m_endSpans[2 * end + 1],
                      "domain '" + d + "' has no " + (volume ? "subdomain" : "surface group") + " '" + p + "'" +
                        LanguageDetail::suggest(p, known));

          ok = false;
        }
      }

      if (!ok) {
        continue;
      }

      if (coupling.m_kind == Coupling::Kind::Volume) {
        const auto a = domains[0]->setup().getRegions(parts[0]);
        const auto b = domains[1]->setup().getRegions(parts[1]);

        if (a != b) {
          this->error(statement->m_span,
                      "coupling '" + coupling.m_name + "': subdomains " + coupling.m_domainA + "." + coupling.m_partA +
                        " and " + coupling.m_domainB + "." + coupling.m_partB +
                        " hold different regions; a volume coupling joins the same regions");
        }
      }

      coupling.m_properties =
        this->properties(statement->m_properties, "coupling '" + coupling.m_name + "'", statement->m_nameSpan);

      m_model.m_couplings.push_back(coupling);
    }
  }
};

} // namespace LanguageDetail

template <class T>
inline Model<T>
read(const std::string& a_path, Pool& a_pool, const Overrides& a_overrides)
{
  LanguageDetail::Reader<T> reader(a_pool, a_overrides);

  return reader.readPath(a_path);
}

template <class T>
inline Model<T>
parse(std::string_view a_text, const std::string& a_name, Pool& a_pool, const Overrides& a_overrides)
{
  LanguageDetail::Reader<T> reader(a_pool, a_overrides);

  return reader.readText(a_text, a_name);
}

// ---------------------------------------------------------------------------------------------------------------------
// Completeness and inventory
// ---------------------------------------------------------------------------------------------------------------------

namespace LanguageDetail {

/**
 * @brief A region's name for messages.
 * @tparam T Precision.
 * @param[in] a_index  The index.
 * @param[in] a_region The region.
 * @return Its name, `outside`, or "region N".
 */
template <class T>
inline std::string
regionName(const ExprIndex<T>& a_index, const uint32_t a_region)
{
  if (a_region == 0) {
    return "outside";
  }

  const std::string& name = a_index.getRegion(a_region).m_name;

  return name.empty() ? "region " + std::to_string(a_region) : "'" + name + "'";
}

/**
 * @brief A surface in words: the shape call it comes from (or its number), and its label when it has one.
 * @tparam T Precision.
 * @param[in] a_model     The model.
 * @param[in] a_inventory The surfaces.
 * @param[in] a_surface   The surface.
 * @return The words.
 */
template <class T>
inline std::string
surfaceWords(const Model<T>& a_model, const SurfaceInventory& a_inventory, const uint32_t a_surface)
{
  const ExprIndex<T>& a_index = a_model.index();

  const uint32_t       leaf  = a_inventory.getLeaf(a_surface);
  const SurfaceLabelId label = a_index.getSurfaceLabel(leaf);
  const ShapeSite*     site  = a_model.siteOfLeaf(leaf);

  const std::string what =
    site != nullptr ? "the " + site->m_shape + " at " + site->m_span.m_file + ":" + std::to_string(site->m_span.m_line)
                    : "surface " + std::to_string(a_surface);

  return what + (label.isNone() ? std::string() : " (label \"" + a_index.getSurfaceLabelName(label) + "\")");
}

} // namespace LanguageDetail

template <class T, class... UserPrimitives>
inline Completeness
completeness(const Model<T>& a_model, const Tape<T, UserPrimitives...>& a_tape, std::string_view a_domain)
{
  const ModelDomain&  domain = a_model.domain(a_domain);
  const ExprIndex<T>& index  = a_model.index();

  Completeness result;

  const auto finding = [&](const bool a_error, const Span& a_span, const std::string& a_message) {
    result.m_findings.push_back(
      Diagnostic{a_error ? Diagnostic::Severity::Error : Diagnostic::Severity::Warning, a_span, a_message});

    result.m_complete = result.m_complete && !a_error;
  };

  for (uint32_t s = 0; s < domain.getNumSubdomains(); s++) {
    if (domain.subdomainProperties(s).empty()) {
      finding(true,
              domain.subdomainSpan(s),
              "subdomain '" + domain.subdomainName(s) + "' of domain '" + domain.name() +
                "' has no properties: give it some, or give its regions' materials some");
    }
  }

  const SurfaceInventory           inventory(index);
  const std::vector<PossiblePiece> pieces = possiblePieces(inventory);

  // The pieces the rays find, with the parts of each they cross: an uncovered piece among them is missing, judged on
  // those parts (a shape's parts need not all lie between the same two regions: a channel's side against the solid,
  // its caps against the outside); one they miss may not exist.
  std::map<std::tuple<uint32_t, uint32_t, uint32_t>, std::vector<uint32_t>> found;

  bool searched = false;

  const auto foundParts = [&](const PossiblePiece& a_piece) -> const std::vector<uint32_t>* {
    if (!searched) {
      for (const auto& piece : findSurfacePieces(a_tape)) {
        std::vector<uint32_t>& parts =
          found[{piece.m_surface.m_surface,
                 Math::max(piece.m_surface.m_ownerRegion, piece.m_surface.m_neighborRegion),
                 Math::min(piece.m_surface.m_ownerRegion, piece.m_surface.m_neighborRegion)}];

        parts.insert(parts.end(), piece.m_parts.begin(), piece.m_parts.end());
      }

      searched = true;
    }

    const auto at = found.find({a_piece.m_surface, a_piece.m_ownerRegion, a_piece.m_neighborRegion});

    return (at == found.end()) ? nullptr : &at->second;
  };

  for (const PossiblePiece& piece : pieces) {
    SurfaceRef<double> surface;

    surface.m_surface        = piece.m_surface;
    surface.m_ownerRegion    = piece.m_ownerRegion;
    surface.m_neighborRegion = piece.m_neighborRegion;

    const SurfaceRole role = domain.setup().role(surface);

    if (role != SurfaceRole::Boundary && role != SurfaceRole::Interface) {
      continue;
    }

    PieceCoverage coverage = coverageOf(piece, inventory, domain.setup());

    if (coverage.m_coverage == Coverage::Full) {
      continue;
    }

    const std::vector<uint32_t>* parts = foundParts(piece);
    const bool                   real  = parts != nullptr;

    if (real) {
      coverage = coverageOf(piece, inventory, domain.setup(), *parts);

      if (coverage.m_coverage == Coverage::Full) {
        continue;
      }
    }

    finding(real,
            domain.span(),
            LanguageDetail::surfaceWords(a_model, inventory, piece.m_surface) + " between " +
              LanguageDetail::regionName(index, piece.m_ownerRegion) + " and " +
              LanguageDetail::regionName(index, piece.m_neighborRegion) + " is " +
              (coverage.m_coverage == Coverage::Partly ? "partly " : "") + "in no surface group of domain '" +
              domain.name() + "'" + (real ? "" : " (it may not exist: no ray found it)"));
  }

  for (const Coupling& coupling : a_model.couplings()) {
    if (coupling.m_kind != Coupling::Kind::Surface ||
        (coupling.m_domainA != domain.name() && coupling.m_domainB != domain.name())) {
      continue;
    }

    const ModelDomain& a = a_model.domain(coupling.m_domainA);
    const ModelDomain& b = a_model.domain(coupling.m_domainB);

    const uint32_t ga = a.findSurfaceGroup(coupling.m_partA);
    const uint32_t gb = b.findSurfaceGroup(coupling.m_partB);

    for (const PossiblePiece& piece : pieces) {
      bool inA = coverageOf(piece, inventory, a.setup()).m_surfaceGroup == ga;
      bool inB = coverageOf(piece, inventory, b.setup()).m_surfaceGroup == gb;

      if (inA != inB) {
        const std::vector<uint32_t>* parts = foundParts(piece);
        const bool                   real  = parts != nullptr;

        // A piece found is judged on the parts of it found.
        if (real) {
          inA = coverageOf(piece, inventory, a.setup(), *parts).m_surfaceGroup == ga;
          inB = coverageOf(piece, inventory, b.setup(), *parts).m_surfaceGroup == gb;

          if (inA == inB) {
            continue;
          }
        }

        finding(real,
                coupling.m_span,
                "coupling '" + coupling.m_name +
                  "': " + LanguageDetail::surfaceWords(a_model, inventory, piece.m_surface) + " between " +
                  LanguageDetail::regionName(index, piece.m_ownerRegion) + " and " +
                  LanguageDetail::regionName(index, piece.m_neighborRegion) + " is in " +
                  (inA ? coupling.m_domainA + "." + coupling.m_partA : coupling.m_domainB + "." + coupling.m_partB) +
                  " but not in " +
                  (inA ? coupling.m_domainB + "." + coupling.m_partB : coupling.m_domainA + "." + coupling.m_partA) +
                  (real ? "" : " (it may not exist: no ray found it)"));
      }
    }
  }

  return result;
}

template <class T>
inline std::string
inventory(const Model<T>& a_model)
{
  const ExprIndex<T>& index = a_model.index();

  std::ostringstream out;

  out << "regions:\n";

  for (uint32_t r = 0; r < index.getNumRegions(); r++) {
    const MaterialLabelId material = index.getMaterialOf(r);

    out << "  " << r << " " << LanguageDetail::regionName(index, r);

    if (!material.isNone()) {
      out << ", material '" << index.getMaterialLabelName(material) << "'";
    }

    out << "\n";
  }

  for (uint32_t g = 0; g < index.getNumRegionGroups(); g++) {
    if (!index.getRegionGroup(g).m_name.empty()) {
      out << "  group '" << index.getRegionGroup(g).m_name << "':";

      for (const uint32_t r : index.getRegionsOfRegionGroup(g)) {
        out << " " << LanguageDetail::regionName(index, r);
      }

      out << "\n";
    }
  }

  out << "materials:\n";

  for (uint32_t m = 0; m < a_model.getNumMaterials(); m++) {
    const Properties& properties = a_model.materialProperties(m);

    out << "  " << index.getMaterialLabelName(MaterialLabelId(m)) << ":";

    for (const std::string& key : properties.keys()) {
      out << " " << key;
    }

    out << (properties.empty() ? " (no properties)" : "") << "\n";
  }

  out << "surface labels:\n";

  for (uint32_t l = 0; l < index.getNumSurfaceLabels(); l++) {
    out << "  \"" << index.getSurfaceLabelName(SurfaceLabelId(l)) << "\"\n";
  }

  out << "shapes and their parts:\n";

  for (const ShapeSite& site : a_model.shapes()) {
    out << "  " << site.m_shape << " at " << site.m_span.m_file << ":" << site.m_span.m_line << ":"
        << site.m_span.m_column;

    if (!site.m_parts.empty()) {
      out << ", parts:";

      for (size_t p = 0; p < site.m_parts.size(); p++) {
        out << " " << site.m_parts[p];

        if (p < site.m_partLabels.size() && !site.m_partLabels[p].empty()) {
          out << "=\"" << site.m_partLabels[p] << "\"";
        }
      }
    }

    out << "\n";
  }

  out << "domains:\n";

  for (uint32_t d = 0; d < a_model.getNumDomains(); d++) {
    const ModelDomain& domain = a_model.domain(d);

    out << "  " << domain.name() << ":\n";

    for (uint32_t s = 0; s < domain.getNumSubdomains(); s++) {
      out << "    subdomain " << domain.subdomainName(s) << ":";

      for (const uint32_t r : domain.setup().getRegions(s)) {
        out << " " << LanguageDetail::regionName(index, r);
      }

      out << "\n";
    }

    for (uint32_t g = 0; g < domain.getNumSurfaceGroups(); g++) {
      out << "    group " << domain.surfaceGroupName(g) << "\n";
    }
  }

  return out.str();
}

// ---------------------------------------------------------------------------------------------------------------------
// The printer
// ---------------------------------------------------------------------------------------------------------------------

namespace LanguageDetail {

/**
 * @brief Prints parsed files in canonical form, with their comments.
 */
class Printer
{
public:
  /**
   * @brief Printer of a file.
   * @param[in] a_file   The file.
   * @param[in] a_folder Where its relative mesh paths lead from the text it is printed into: "" when that is its own
   *                     folder; the included file's folder, relative to the first file's, when flattening.
   */
  explicit Printer(const Syntax::File& a_file, std::filesystem::path a_folder = {})
    : m_file(a_file), m_folder(std::move(a_folder))
  {}

  /**
   * @brief Print the file's statements (not its version line).
   * @param[in,out] a_out     Where.
   * @param[in]     a_include What to print for an include statement (the included file's statements, when
   *                          flattening), or nullptr to print the statement.
   */
  template <class F>
  void
  statements(std::string& a_out, F&& a_include)
  {
    uint32_t previous = 1; // the last line printed

    for (const Syntax::Statement& s : m_file.m_statements) {
      previous = this->commentsBefore(a_out, s.m_span.m_line, previous, "");

      if (s.m_span.m_line > previous + 1 && !a_out.empty() && a_out.back() == '\n' &&
          (a_out.size() < 2 || a_out[a_out.size() - 2] != '\n')) {
        a_out += "\n";
      }

      if (s.m_kind == Syntax::StatementKind::Include && a_include(s, a_out)) {
        previous = this->lastLine(s.m_span);

        continue;
      }

      this->statement(a_out, s);
      this->trailing(a_out, this->lastLine(s.m_span));

      a_out += "\n";

      previous = this->lastLine(s.m_span);
    }

    this->commentsBefore(a_out, 0xFFFFFFFFU, previous, "");
  }

private:
  /**
   * @brief The width a method chain is broken at.
   */
  static constexpr size_t Width = 100;

  /**
   * @brief The file.
   */
  const Syntax::File& m_file;

  /**
   * @brief Where its relative mesh paths lead from (see the constructor).
   */
  std::filesystem::path m_folder;

  /**
   * @brief The string being printed that is a mesh's path, written literally in a mesh() call.
   */
  const Syntax::Node* m_meshPath = nullptr;

  /**
   * @brief The next comment to print.
   */
  size_t m_comment = 0;

  /**
   * @brief The line a span ends on.
   * @param[in] a_span The span.
   * @return The line.
   */
  uint32_t
  lastLine(const Span& a_span) const
  {
    uint32_t line = a_span.m_line;

    for (uint32_t i = a_span.m_begin; i < a_span.m_end && i < m_file.m_text.size(); i++) {
      line += m_file.m_text[i] == '\n' ? 1U : 0U;
    }

    return line;
  }

  /**
   * @brief Print the comments on their own lines before a line, keeping a blank line before them where the file has
   * one.
   * @param[in,out] a_out      Where.
   * @param[in]     a_line     The line.
   * @param[in]     a_previous The last line printed.
   * @param[in]     a_indent   The indentation.
   * @return The last line printed.
   */
  uint32_t
  commentsBefore(std::string& a_out, const uint32_t a_line, uint32_t a_previous, const char* a_indent)
  {
    while (m_comment < m_file.m_comments.size() && m_file.m_comments[m_comment].m_span.m_line < a_line) {
      const Syntax::Comment& comment = m_file.m_comments[m_comment++];

      if (!comment.m_ownLine) {
        continue;
      }
      if (comment.m_span.m_line > a_previous + 1 && !a_out.empty() &&
          (a_out.size() < 2 || a_out.substr(a_out.size() - 2) != "\n\n")) {
        a_out += "\n";
      }

      a_out += std::string(a_indent) + comment.m_text + "\n";

      a_previous = comment.m_span.m_line;
    }

    return a_previous;
  }

  /**
   * @brief Print the comments up to a line not printed yet, after the code printed for it.
   * @param[in,out] a_out    Where.
   * @param[in]     a_line   The line.
   * @param[in]     a_indent The indentation of the code.
   */
  void
  trailing(std::string& a_out, const uint32_t a_line, const std::string& a_indent = "")
  {
    // The comments up to the line not printed yet: inside what was written over several lines and is printed on one,
    // say. Those after code follow the printed line; those on their own lines follow it, each on its own line.
    std::string own;

    while (m_comment < m_file.m_comments.size() && m_file.m_comments[m_comment].m_span.m_line <= a_line) {
      const Syntax::Comment& comment = m_file.m_comments[m_comment++];

      if (comment.m_ownLine) {
        own += "\n" + a_indent + comment.m_text;
      }
      else {
        a_out += "  " + comment.m_text;
      }
    }

    a_out += own;
  }

  /**
   * @brief A node's precedence: 1 for + and -, 2 for * and /, 3 for negation, 4 for everything else.
   * @param[in] a_node The node.
   * @return The precedence.
   */
  static int
  precedence(const Syntax::Node& a_node) noexcept
  {
    switch (a_node.m_kind) {
    case Syntax::NodeKind::Add:
    case Syntax::NodeKind::Subtract: {
      return 1;
    }
    case Syntax::NodeKind::Multiply:
    case Syntax::NodeKind::Divide: {
      return 2;
    }
    case Syntax::NodeKind::Negate: {
      return 3;
    }
    default: {
      return 4;
    }
    }
  }

  /**
   * @brief A string literal.
   * @param[in] a_text The text.
   * @return It quoted and escaped.
   */
  static std::string
  quoted(const std::string& a_text)
  {
    std::string out = "\"";

    for (const char c : a_text) {
      if (c == '"' || c == '\\') {
        out += '\\';
      }

      out += c;
    }

    return out + "\"";
  }

  /**
   * @brief Print a node.
   * @param[in,out] a_out    Where.
   * @param[in]     a_node   The node.
   * @param[in]     a_indent The indentation of the line it is on.
   */
  void
  node(std::string& a_out, const Syntax::Node& a_node, const std::string& a_indent)
  {
    using NK = Syntax::NodeKind;

    const auto child = [&](const Syntax::Node& a_child, const bool a_parens) {
      if (a_parens) {
        a_out += "(";
      }

      this->node(a_out, a_child, a_indent);

      if (a_parens) {
        a_out += ")";
      }
    };

    const auto args = [&](const size_t a_first) {
      a_out += "(";

      for (size_t i = a_first; i < a_node.m_children.size(); i++) {
        a_out += i > a_first ? ", " : "";

        if (!a_node.m_keys[i].empty()) {
          a_out += a_node.m_keys[i] + " = ";
        }

        this->node(a_out, a_node.m_children[i], a_indent);
      }

      a_out += ")";
    };

    switch (a_node.m_kind) {
    case NK::Number:
    case NK::Bool:
    case NK::Name: {
      a_out += a_node.m_text;

      break;
    }
    case NK::String: {
      const std::filesystem::path path(a_node.m_text);

      // A mesh's path, relative to its file, made relative to the file it is printed into.
      if (&a_node == m_meshPath && !m_folder.empty() && path.is_relative()) {
        a_out += quoted((m_folder / path).lexically_normal().generic_string());
      }
      else {
        a_out += quoted(a_node.m_text);
      }

      break;
    }
    case NK::Pair:
    case NK::Vector: {
      a_out += "(";

      for (size_t k = 0; k < a_node.m_children.size(); k++) {
        a_out += k > 0 ? ", " : "";

        this->node(a_out, a_node.m_children[k], a_indent);
      }

      a_out += ")";

      break;
    }
    case NK::Negate: {
      a_out += "-";

      child(a_node.m_children[0], precedence(a_node.m_children[0]) < 3);

      break;
    }
    case NK::Add:
    case NK::Subtract:
    case NK::Multiply:
    case NK::Divide: {
      const int p = precedence(a_node);

      const char* symbols[] = {" + ", " - ", " * ", " / "};

      child(a_node.m_children[0], precedence(a_node.m_children[0]) < p);

      a_out += symbols[static_cast<int>(a_node.m_kind) - static_cast<int>(NK::Add)];

      child(a_node.m_children[1], precedence(a_node.m_children[1]) <= p);

      break;
    }
    case NK::Call: {
      a_out += a_node.m_text;

      if (a_node.m_text == "mesh") {
        for (size_t k = 0; k < a_node.m_children.size(); k++) {
          const bool file = a_node.m_keys[k] == "file" || (k == 0 && a_node.m_keys[k].empty());

          if (file && a_node.m_children[k].m_kind == NK::String) {
            m_meshPath = &a_node.m_children[k];
          }
        }
      }

      args(0);

      break;
    }
    case NK::Method: {
      child(a_node.m_children[0], precedence(a_node.m_children[0]) < 4);

      // A method that would take the line past the width starts the next line.
      std::string method = "." + a_node.m_text;
      {
        std::string rest;

        std::swap(a_out, rest);

        args(1);

        std::swap(a_out, rest);

        method += rest;
      }

      const size_t lineStart = a_out.rfind('\n') == std::string::npos ? 0 : a_out.rfind('\n') + 1;

      if (a_out.size() - lineStart + method.size() > Width && method.find('\n') == std::string::npos) {
        a_out += "\n" + a_indent + "  ";
      }

      a_out += method;

      break;
    }
    case NK::Overlay: {
      const std::string inner = a_indent + "  ";

      a_out += "overlay {";

      this->trailing(a_out, a_node.m_span.m_line);

      a_out += "\n";

      uint32_t previous = a_node.m_span.m_line;

      for (const Syntax::Node& region : a_node.m_children) {
        previous = this->commentsBefore(a_out, region.m_span.m_line, previous, inner.c_str());

        a_out += inner + "region " + region.m_text + " = ";

        this->node(a_out, region.m_children[0], inner);

        if (!region.m_material.empty()) {
          a_out += " material " + region.m_material;
        }

        this->trailing(a_out, this->lastLine(region.m_span), inner);

        a_out += "\n";

        previous = this->lastLine(region.m_span);
      }

      this->commentsBefore(a_out, this->lastLine(a_node.m_span), previous, inner.c_str());

      a_out += a_indent + "}";

      break;
    }
    default: {
      break;
    }
    }
  }

  /**
   * @brief Print a properties block on one line.
   * @param[in,out] a_out   Where.
   * @param[in]     a_block The block.
   */
  void
  properties(std::string& a_out, const Syntax::Properties& a_block)
  {
    if (!a_block.m_written) {
      return;
    }

    a_out += " {";

    for (size_t i = 0; i < a_block.m_entries.size(); i++) {
      a_out += (i > 0 ? ", " : " ") + a_block.m_entries[i].m_key + " = ";

      this->node(a_out, a_block.m_entries[i].m_value, "");
    }

    a_out += a_block.m_entries.empty() ? "}" : " }";
  }

  /**
   * @brief Print a statement (without its line end).
   * @param[in,out] a_out Where.
   * @param[in]     a_s   The statement.
   */
  void
  statement(std::string& a_out, const Syntax::Statement& a_s)
  {
    using SK = Syntax::StatementKind;

    switch (a_s.m_kind) {
    case SK::Value: {
      a_out += std::string(a_s.m_param ? "param " : "") + a_s.m_name + " = ";

      this->node(a_out, a_s.m_expr, "");

      break;
    }
    case SK::Object: {
      a_out += "object " + (a_s.m_name.empty() ? std::string() : a_s.m_name + " = ");

      this->node(a_out, a_s.m_expr, "");

      break;
    }
    case SK::Material: {
      a_out += "material " + a_s.m_name;

      this->properties(a_out, a_s.m_properties);

      break;
    }
    case SK::Domain: {
      a_out += "domain " + a_s.m_name + " {";

      this->trailing(a_out, a_s.m_span.m_line);

      a_out += "\n";

      uint32_t previous = a_s.m_span.m_line;

      for (const Syntax::DomainItem& item : a_s.m_items) {
        previous = this->commentsBefore(a_out, item.m_span.m_line, previous, "  ");

        a_out += std::string("  ") + (item.m_subdomain ? "subdomain " : "group ") + item.m_name + " = ";

        for (size_t i = 0; i < item.m_list.size(); i++) {
          a_out += i > 0 ? ", " : "";

          this->node(a_out, item.m_list[i], "  ");
        }

        this->properties(a_out, item.m_properties);
        this->trailing(a_out, this->lastLine(item.m_span), "  ");

        a_out += "\n";

        previous = this->lastLine(item.m_span);
      }

      this->commentsBefore(a_out, this->lastLine(a_s.m_span), previous, "  ");

      a_out += "}";

      break;
    }
    case SK::Couple: {
      a_out += "couple " + a_s.m_name + " = " + a_s.m_coupleKind + "(" + a_s.m_ends[0] + "." + a_s.m_ends[1] + ", " +
               a_s.m_ends[2] + "." + a_s.m_ends[3] + ")";

      this->properties(a_out, a_s.m_properties);

      break;
    }
    case SK::Include: {
      a_out += "include(" + quoted(a_s.m_name) + ")";

      break;
    }
    case SK::Label: {
      this->node(a_out, a_s.m_expr, "");

      break;
    }
    case SK::View: {
      a_out += "view";

      this->properties(a_out, a_s.m_properties);

      break;
    }
    default: {
      break;
    }
    }
  }
};

/**
 * @brief Print a file, flattening its includes when asked.
 * @param[in,out] a_out     Where.
 * @param[in]     a_sources Every parsed file.
 * @param[in]     a_file    The file.
 * @param[in]     a_flatten Whether to print included files in place of their include statements.
 * @param[in,out] a_printed The files printed, against include cycles.
 * @param[in]     a_folder  Its folder relative to the first file's when it is included into it ("" for the first).
 */
inline void
printFile(std::string&                     a_out,
          const std::vector<Syntax::File>& a_sources,
          const Syntax::File&              a_file,
          const bool                       a_flatten,
          std::set<std::string>&           a_printed,
          const std::filesystem::path&     a_folder = {})
{
  a_printed.insert(a_file.m_name);

  Printer printer(a_file, a_folder);

  printer.statements(a_out, [&](const Syntax::Statement& a_include, std::string& a_text) {
    if (!a_flatten) {
      return false;
    }

    // Spellings compared as normal forms: "parts/a.ebg" and "./parts/a.ebg" are one file.
    const auto normal = [](const std::filesystem::path& a_path) { return a_path.lexically_normal().generic_string(); };

    const std::string path = normal(std::filesystem::path(a_file.m_name).parent_path() / a_include.m_name);

    for (const Syntax::File& other : a_sources) {
      if (normal(other.m_name) == path && a_printed.count(other.m_name) == 0) {
        // Its mesh paths lead from its folder: relative to the first file's, they read the same meshes.
        const std::filesystem::path folder =
          (a_folder / std::filesystem::path(a_include.m_name).parent_path()).lexically_normal();

        printFile(a_text, a_sources, other, a_flatten, a_printed, folder == "." ? std::filesystem::path() : folder);

        return true;
      }
    }

    return false;
  });
}

} // namespace LanguageDetail

template <class T>
inline std::string
print(const Model<T>& a_model, const bool a_flatten)
{
  const std::vector<Syntax::File>& sources = a_model.sources();

  if (sources.empty()) {
    return "ebg 0\n";
  }

  std::string out = "ebg " + std::to_string(sources[0].m_version);

  // A comment after the version.
  for (const Syntax::Comment& comment : sources[0].m_comments) {
    if (!comment.m_ownLine && comment.m_span.m_line == 1U) {
      out += "  " + comment.m_text;
    }
  }

  out += "\n\n";

  std::set<std::string> printed;

  LanguageDetail::printFile(out, sources, sources[0], a_flatten, printed);

  while (out.size() > 1 && out[out.size() - 1] == '\n' && out[out.size() - 2] == '\n') {
    out.pop_back();
  }

  return out;
}

} // namespace Language
} // namespace EBGeometry

#endif
