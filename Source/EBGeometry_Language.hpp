// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Language.hpp
 * @brief  The .ebg language: models read from text files (geometry, regions, materials, domains and couplings), and
 * their properties as tables in a pool. Opt-in: EBGeometry.hpp does not include it.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_LANGUAGE_HPP
#define EBGEOMETRY_LANGUAGE_HPP

// Std includes
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Our includes
#include "../EBGeometry.hpp"
#include "EBGeometry_LanguageSyntax.hpp"

namespace EBGeometry {

/**
 * @brief The .ebg language: a model (geometry, its regions and materials, and the domains a solver computes in)
 * written in a text file and read at run time, instead of composed in compiled C++.
 * @details Host only, except Table, which a device reads. The language's reference is the Sphinx page; read() and
 * parse() are the way in, Model what they give.
 */
namespace Language {

/**
 * @brief The properties a material, subdomain, surface group, coupling or the view block carries: numbers, vectors and
 * strings by key, passed through unread for the program to interpret.
 * @details A subdomain's properties are its own over its regions' material's, key by key. Every failure (a missing
 * key, a value of another type, a string not among the names given) throws an Error at the key, or at the line where
 * the owner is written when the key is missing.
 */
class Properties
{
public:
  /**
   * @brief The types a value can have.
   */
  enum class Type : uint8_t
  {
    Number, ///< A number.
    Vector, ///< Three numbers.
    String, ///< A string.
    Bool    ///< true or false.
  };

  /**
   * @brief One value, and where it is written.
   */
  struct Value
  {
    /**
     * @brief Its type.
     */
    Type m_type = Type::Number;

    /**
     * @brief A number (or a bool, 1 or 0).
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
     * @brief Where the value is written.
     */
    Span m_span;
  };

  /**
   * @brief No properties, owned by nothing written.
   */
  Properties() = default;

  /**
   * @brief No properties yet, of an owner.
   * @param[in] a_owner The owner in words ("material 'air'"), for messages.
   * @param[in] a_span  Where the owner is written.
   */
  EBGEOMETRY_HOST
  Properties(std::string a_owner, const Span& a_span) : m_owner(std::move(a_owner)), m_span(a_span)
  {}

  /**
   * @brief Whether a key is set.
   * @param[in] a_key The key.
   * @return Whether.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  has(std::string_view a_key) const noexcept;

  /**
   * @brief The keys, in the order written.
   * @return The keys.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<std::string>
  keys() const;

  /**
   * @brief Whether there are none.
   * @return Whether.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  empty() const noexcept
  {
    return m_entries.empty();
  }

  /**
   * @brief A number.
   * @param[in] a_key The key.
   * @return The number; an Error when the key is missing or not a number.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  double
  number(std::string_view a_key) const;

  /**
   * @brief A number, or a fallback when the key is missing.
   * @param[in] a_key      The key.
   * @param[in] a_fallback The value when the key is missing.
   * @return The number; an Error when the key is set and not a number.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  double
  number(std::string_view a_key, double a_fallback) const;

  /**
   * @brief A vector.
   * @tparam T Precision.
   * @param[in] a_key The key.
   * @return The vector; an Error when the key is missing, not a vector, or too large for T.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST
  Vec3T<T>
  vector(std::string_view a_key) const;

  /**
   * @brief A vector, or a fallback when the key is missing.
   * @tparam T Precision.
   * @param[in] a_key      The key.
   * @param[in] a_fallback The value when the key is missing.
   * @return The vector; an Error when the key is set and not a vector, or too large for T.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST
  Vec3T<T>
  vector(std::string_view a_key, const Vec3T<T>& a_fallback) const;

  /**
   * @brief A string.
   * @param[in] a_key The key.
   * @return The string; an Error when the key is missing or not a string.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  string(std::string_view a_key) const;

  /**
   * @brief A string, or a fallback when the key is missing.
   * @param[in] a_key      The key.
   * @param[in] a_fallback The value when the key is missing.
   * @return The string; an Error when the key is set and not a string.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  string(std::string_view a_key, const std::string& a_fallback) const;

  /**
   * @brief A string that is one of a set of names, as its index among them: how a solver turns a kind ("no_slip")
   * into a code its kernels switch on.
   * @param[in] a_key   The key.
   * @param[in] a_names The names.
   * @return The index; an Error listing the names when the string is none of them, or the key is missing.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  code(std::string_view a_key, std::initializer_list<std::string_view> a_names) const;

  /**
   * @brief Where a key's value is written; where the owner is when the key is missing.
   * @param[in] a_key The key.
   * @return The span.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  Span
  spanOf(std::string_view a_key) const;

  /**
   * @brief A value by key, or nullptr.
   * @param[in] a_key The key.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Value*
  find(std::string_view a_key) const noexcept;

  /**
   * @brief Set a key, replacing its value if it is set.
   * @param[in] a_key   The key.
   * @param[in] a_value The value.
   */
  EBGEOMETRY_HOST
  void
  set(const std::string& a_key, const Value& a_value);

  /**
   * @brief The owner in words.
   * @return It.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  owner() const noexcept
  {
    return m_owner;
  }

  /**
   * @brief Where the owner is written.
   * @return The span.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Span&
  span() const noexcept
  {
    return m_span;
  }

private:
  /**
   * @brief The owner in words.
   */
  std::string m_owner;

  /**
   * @brief Where the owner is.
   */
  Span m_span;

  /**
   * @brief The keys and values, in order.
   */
  std::vector<std::pair<std::string, Value>> m_entries;

  /**
   * @brief A value of a type, or an Error.
   * @param[in] a_key  The key.
   * @param[in] a_type The type.
   * @return The value, nullptr when the key is missing.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Value*
  typed(std::string_view a_key, Type a_type) const;

  /**
   * @brief The Error for a missing key.
   * @param[in] a_key The key.
   * @return The error.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  Error
  missing(std::string_view a_key) const;

  /**
   * @brief A vector value in T, refused when a component is too large for T.
   * @tparam T Precision.
   * @param[in] a_key   Its key.
   * @param[in] a_value The value, a vector.
   * @return The vector.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST
  Vec3T<T>
  narrowed(std::string_view a_key, const Value& a_value) const;
};

/**
 * @brief A table of a program's own rows in a pool, one per region, subdomain or surface group, read on the host and
 * on a device by the numbers EBGeometry computes there (Tape::region(), Domain::subdomainAt(), Domain::surfaceGroupOf()).
 * @details A pool handle like every other: copying it copies a view, the pool must outlive it and every copy, and
 * rebasedView() moves it to a mirror of its pool. Built by Model::regions(), ModelDomain::volumes() and
 * ModelDomain::surfaces(), each row filled on the host from its properties.
 * @tparam S The row type: trivially copyable.
 */
template <class S>
class Table : public PoolHandle
{
  static_assert(std::is_trivially_copyable_v<S>, "Table<S> requires a trivially copyable row type");

public:
  /**
   * @brief Where the rows are in the pool.
   */
  struct Layout
  {
    /**
     * @brief The rows.
     */
    PODVector<S> m_rows;
  };

  /**
   * @brief An empty table in no pool.
   */
  Table() noexcept = default;

  /**
   * @brief Rows copied into a pool.
   * @param[in,out] a_pool The pool; not frozen, and it must outlive the table.
   * @param[in]     a_rows The rows.
   */
  EBGEOMETRY_HOST
  Table(Pool& a_pool, const std::vector<S>& a_rows)
  {
    this->attach(a_pool, "Table");

    m_layout.m_rows.reserveSizedFrom(a_pool, static_cast<uint32_t>(a_rows.size()));

    for (uint32_t i = 0; i < a_rows.size(); i++) {
      m_layout.m_rows.at(this->base(), i) = a_rows[i];
    }
  }

  /**
   * @brief A table at a location.
   * @param[in] a_layout   Where the rows are.
   * @param[in] a_location The pool's location.
   */
  EBGEOMETRY_HOST_DEVICE
  Table(const Layout& a_layout, const PoolLocation& a_location) noexcept : PoolHandle(a_location), m_layout(a_layout)
  {}

  /**
   * @brief A row.
   * @param[in] a_row The row's number, below size().
   * @return The row.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const S&
  operator[](const uint32_t a_row) const noexcept
  {
    EBGEOMETRY_EXPECT(a_row < m_layout.m_rows.size());

    return m_layout.m_rows.at(this->base(), a_row);
  }

  /**
   * @brief How many rows.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  size() const noexcept
  {
    return m_layout.m_rows.size();
  }

  /**
   * @brief Where the rows are.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Layout&
  layout() const noexcept
  {
    return m_layout;
  }

  /**
   * @brief The same table in another pool with the same layout (a mirror).
   * @param[in] a_pool The pool.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Table
  rebasedView(const Pool& a_pool) const noexcept
  {
    Table view = *this;

    view.setLocation(this->rebasedLocation(a_pool, m_layout.m_rows.endByte(), "Table"));

    return view;
  }

private:
  /**
   * @brief Where the rows are.
   */
  Layout m_layout;
};

/**
 * @brief Values for a model's params, by name, each written as in a file ("0.5", "(1, 0, 0)", "\"steel\""): what
 * `ebgstudio -D name=value` gives. Overriding a value that is not a param, or a name the model does not define, is an
 * error.
 */
using Overrides = std::map<std::string, std::string>;

/**
 * @brief A param of a model: its name, its value as text (after overrides), its type, and where it is written.
 */
struct Param
{
  /**
   * @brief The name.
   */
  std::string m_name;

  /**
   * @brief The value, as text in the language.
   */
  std::string m_value;

  /**
   * @brief The type: "number", "vector", "string", "bool" or "shape".
   */
  std::string m_type;

  /**
   * @brief Where the param is written.
   */
  Span m_span;

  /**
   * @brief Whether an override set its value.
   */
  bool m_overridden = false;
};

/**
 * @brief A domain of a model: its name, its setup, and the names and properties of its subdomains and surface groups.
 * @details A Domain is built from the setup for the model's tape: Domain(pool, tape, modelDomain.setup()). Its numbers
 * are the setup's: subdomain i is label i, surface group g is group g.
 */
class ModelDomain
{
public:
  /**
   * @brief The name.
   * @return It.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  name() const noexcept
  {
    return m_name;
  }

  /**
   * @brief How the domain is made.
   * @return The setup.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const DomainSetup&
  setup() const noexcept
  {
    return m_setup;
  }

  /**
   * @brief How many subdomains.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSubdomains() const noexcept
  {
    return static_cast<uint32_t>(m_subdomainNames.size());
  }

  /**
   * @brief A subdomain's name.
   * @param[in] a_subdomain The subdomain.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  subdomainName(uint32_t a_subdomain) const;

  /**
   * @brief A subdomain by name.
   * @param[in] a_name The name.
   * @return Its number, or Subdomains::None.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  findSubdomain(std::string_view a_name) const noexcept;

  /**
   * @brief A subdomain's properties: its own over its regions' material's.
   * @param[in] a_subdomain The subdomain.
   * @return The properties.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Properties&
  subdomainProperties(uint32_t a_subdomain) const;

  /**
   * @brief How many surface groups.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSurfaceGroups() const noexcept
  {
    return static_cast<uint32_t>(m_groupNames.size());
  }

  /**
   * @brief A surface group's name.
   * @param[in] a_group The group.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  surfaceGroupName(uint32_t a_group) const;

  /**
   * @brief A surface group by name.
   * @param[in] a_name The name.
   * @return Its number, or SurfaceGroups::None.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  findSurfaceGroup(std::string_view a_name) const noexcept;

  /**
   * @brief A surface group's properties.
   * @param[in] a_group The group.
   * @return The properties.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Properties&
  surfaceGroupProperties(uint32_t a_group) const;

  /**
   * @brief A table with one row per subdomain, each filled from the subdomain's properties: what a kernel indexes
   * with Domain::subdomainAt() (a region in no subdomain is Domain::Excluded, which has no row).
   * @tparam S The row type: trivially copyable.
   * @tparam F A function S(const Properties&).
   * @param[in,out] a_pool The pool the table is put in.
   * @param[in]     a_fill The function filling a row.
   * @return The table.
   */
  template <class S, class F>
  [[nodiscard]] EBGEOMETRY_HOST
  Table<S>
  volumes(Pool& a_pool, F&& a_fill) const
  {
    std::vector<S> rows;

    for (const Properties& properties : m_subdomainProperties) {
      rows.push_back(a_fill(properties));
    }

    return Table<S>(a_pool, rows);
  }

  /**
   * @brief A table with one row per surface group, each filled from the group's properties: what a kernel indexes
   * with Domain::surfaceGroupOf().
   * @tparam S The row type: trivially copyable.
   * @tparam F A function S(const Properties&).
   * @param[in,out] a_pool The pool the table is put in.
   * @param[in]     a_fill The function filling a row.
   * @return The table.
   */
  template <class S, class F>
  [[nodiscard]] EBGEOMETRY_HOST
  Table<S>
  surfaces(Pool& a_pool, F&& a_fill) const
  {
    std::vector<S> rows;

    for (const Properties& properties : m_groupProperties) {
      rows.push_back(a_fill(properties));
    }

    return Table<S>(a_pool, rows);
  }

  /**
   * @brief Where the domain is written.
   * @return The span of its name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Span&
  span() const noexcept
  {
    return m_span;
  }

  /**
   * @brief Where a subdomain is written.
   * @param[in] a_subdomain The subdomain.
   * @return The span of its line.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Span&
  subdomainSpan(uint32_t a_subdomain) const;

  /**
   * @brief Where a surface group is written.
   * @param[in] a_group The group.
   * @return The span of its line.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Span&
  surfaceGroupSpan(uint32_t a_group) const;

private:
  template <class T>
  friend class Reader;

  /**
   * @brief The name.
   */
  std::string m_name;

  /**
   * @brief Where its name is written.
   */
  Span m_span;

  /**
   * @brief How it is made.
   */
  DomainSetup m_setup;

  /**
   * @brief The subdomains' names.
   */
  std::vector<std::string> m_subdomainNames;

  /**
   * @brief The subdomains' properties.
   */
  std::vector<Properties> m_subdomainProperties;

  /**
   * @brief Where the subdomains are written.
   */
  std::vector<Span> m_subdomainSpans;

  /**
   * @brief The surface groups' names.
   */
  std::vector<std::string> m_groupNames;

  /**
   * @brief The surface groups' properties.
   */
  std::vector<Properties> m_groupProperties;

  /**
   * @brief Where the surface groups are written.
   */
  std::vector<Span> m_groupSpans;
};

/**
 * @brief A coupling of two domains: two subdomains that hold the same regions (volume), or two surface groups that
 * cover the same pieces of surface (surface), with its properties.
 */
struct Coupling
{
  /**
   * @brief What is coupled.
   */
  enum class Kind : uint8_t
  {
    Volume, ///< Two subdomains.
    Surface ///< Two surface groups.
  };

  /**
   * @brief The name.
   */
  std::string m_name;

  /**
   * @brief Volume or surface.
   */
  Kind m_kind = Kind::Volume;

  /**
   * @brief The first end's domain.
   */
  std::string m_domainA;

  /**
   * @brief The first end's subdomain or surface group.
   */
  std::string m_partA;

  /**
   * @brief The second end's domain.
   */
  std::string m_domainB;

  /**
   * @brief The second end's subdomain or surface group.
   */
  std::string m_partB;

  /**
   * @brief Its properties.
   */
  Properties m_properties;

  /**
   * @brief Where it is written.
   */
  Span m_span;
};

/**
 * @brief A shape the model's text builds, and where: what the inventory lists and a viewer names parts by.
 */
struct ShapeSite
{
  /**
   * @brief No site (a leaf the text did not build).
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The shape's name in the language ("box", "mesh").
   */
  std::string m_shape;

  /**
   * @brief Its part names, in the order of its parts (a mesh's: its face groups).
   */
  std::vector<std::string> m_parts;

  /**
   * @brief The surface label each part gets ("" for none), as the text labels the shape where it is used; labels
   * separated by commas when it is used with different ones.
   */
  std::vector<std::string> m_partLabels;

  /**
   * @brief Whether the text labels each part (a mesh's face groups label its parts without it).
   */
  std::vector<bool> m_partLabeled;

  /**
   * @brief The shape's own label, as the text gives it where the shape is used ("" for none); labels separated by
   * commas when it is used with different ones.
   */
  std::string m_label;

  /**
   * @brief Where the call is written.
   */
  Span m_span;
};

/**
 * @brief A model read from a .ebg file or folder: its object, the names of its regions, materials and labels, its
 * materials, domains, couplings and params, and the files it read.
 * @details The caller compiles the object into a tape, Tape<T, UserPrimitives...>(pool, model.object()), picking the
 * tape's features; and builds a domain for that tape from a ModelDomain's setup, Domain(pool, tape,
 * model.domain("cfd").setup()). Meshes the model reads are built into the pool given to read(). The model keeps the
 * parsed files, so that it can be printed (print()) and a viewer can edit them.
 * @tparam T Precision; numbers are read as double and converted.
 */
template <class T>
class Model
{
public:
  /**
   * @brief The object, to compile into a tape.
   * @return The expression.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Expr<T>&
  object() const noexcept
  {
    return *m_object;
  }

  /**
   * @brief The object's index: its regions, region groups, materials and surface labels by name.
   * @return The index.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ExprIndex<T>&
  index() const noexcept
  {
    return *m_index;
  }

  /**
   * @brief How many materials: every name a region uses, numbered as the index numbers them (MaterialLabelId).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumMaterials() const noexcept
  {
    return static_cast<uint32_t>(m_materials.size());
  }

  /**
   * @brief A material's properties (none for a material no material statement declares).
   * @param[in] a_material The material.
   * @return The properties.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Properties&
  materialProperties(uint32_t a_material) const;

  /**
   * @brief How many domains.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumDomains() const noexcept
  {
    return static_cast<uint32_t>(m_domains.size());
  }

  /**
   * @brief A domain by number, in the order written.
   * @param[in] a_domain The domain.
   * @return It.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ModelDomain&
  domain(uint32_t a_domain) const;

  /**
   * @brief A domain by name.
   * @param[in] a_name The name.
   * @return It; an Error listing the domains when the model has none of that name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ModelDomain&
  domain(std::string_view a_name) const;

  /**
   * @brief The couplings.
   * @return They.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<Coupling>&
  couplings() const noexcept
  {
    return m_couplings;
  }

  /**
   * @brief The params, in the order written.
   * @return They.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<Param>&
  params() const noexcept
  {
    return m_params;
  }

  /**
   * @brief The warnings found reading it.
   * @return They.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<Diagnostic>&
  warnings() const noexcept
  {
    return m_warnings;
  }

  /**
   * @brief Every file read, the model's own and the meshes: what a viewer watches to reload.
   * @return The paths.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<std::string>&
  files() const noexcept
  {
    return m_files;
  }

  /**
   * @brief The view block's properties (camera, colors): for viewers; programs ignore them.
   * @return The properties.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Properties&
  view() const noexcept
  {
    return m_view;
  }

  /**
   * @brief The shapes the text builds, in the order built.
   * @return They.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<ShapeSite>&
  shapes() const noexcept
  {
    return m_shapes;
  }

  /**
   * @brief The shape call a leaf of the object comes from: what a viewer names a picked surface by.
   * @param[in] a_leaf The leaf (ExprIndex's numbering; LeafRef::m_leaf).
   * @return Its site among shapes(), or nullptr.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ShapeSite*
  siteOfLeaf(const uint32_t a_leaf) const noexcept
  {
    return (a_leaf < m_leafSites.size() && m_leafSites[a_leaf] != ShapeSite::None) ? &m_shapes[m_leafSites[a_leaf]]
                                                                                   : nullptr;
  }

  /**
   * @brief The index among shapes() of the shape call a leaf comes from.
   * @param[in] a_leaf The leaf.
   * @return The index, or ShapeSite::None.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  siteIndexOfLeaf(const uint32_t a_leaf) const noexcept
  {
    return a_leaf < m_leafSites.size() ? m_leafSites[a_leaf] : ShapeSite::None;
  }

  /**
   * @brief The parsed files, the model's first.
   * @return They.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<Syntax::File>&
  sources() const noexcept
  {
    return m_sources;
  }

  /**
   * @brief The setup file a model folder includes after its model.ebg.
   * @return Its index among sources(), or -1 for none.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  int
  setupSource() const noexcept
  {
    return m_setupSource;
  }

  /**
   * @brief The model's names for a viewer: regions, materials and labels, every domain's setup with the names of its
   * subdomains and surface groups, and the colors of the view block.
   * @return The names.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  RegionNames
  names() const;

  /**
   * @brief A table with one row per region of the object (region 0, `outside`, first), each filled from its
   * material's properties: what a kernel indexes with Tape::region(). A region without a material, `outside`
   * included, gets empty properties, so the function should give fallbacks.
   * @tparam S The row type: trivially copyable.
   * @tparam F A function S(const Properties&).
   * @param[in,out] a_pool The pool the table is put in.
   * @param[in]     a_fill The function filling a row.
   * @return The table.
   */
  template <class S, class F>
  [[nodiscard]] EBGEOMETRY_HOST
  Table<S>
  regions(Pool& a_pool, F&& a_fill) const
  {
    std::vector<S> rows;

    const Properties none("a region without a material", Span{});

    for (uint32_t r = 0; r < m_index->getNumRegions(); r++) {
      const MaterialLabelId material = m_index->getMaterialOf(r);

      rows.push_back(a_fill(material.isNone() ? none : m_materials[material.value()]));
    }

    return Table<S>(a_pool, rows);
  }

private:
  template <class U>
  friend class Reader;

  /**
   * @brief An empty model, which only the reader makes and fills.
   */
  Model() = default;

  /**
   * @brief The object.
   */
  std::optional<Expr<T>> m_object;

  /**
   * @brief Its index.
   */
  std::shared_ptr<const ExprIndex<T>> m_index;

  /**
   * @brief The materials' properties, by MaterialLabelId.
   */
  std::vector<Properties> m_materials;

  /**
   * @brief The domains.
   */
  std::vector<ModelDomain> m_domains;

  /**
   * @brief The couplings.
   */
  std::vector<Coupling> m_couplings;

  /**
   * @brief The params.
   */
  std::vector<Param> m_params;

  /**
   * @brief The warnings.
   */
  std::vector<Diagnostic> m_warnings;

  /**
   * @brief The files read.
   */
  std::vector<std::string> m_files;

  /**
   * @brief The view block.
   */
  Properties m_view;

  /**
   * @brief The shapes built.
   */
  std::vector<ShapeSite> m_shapes;

  /**
   * @brief Each leaf's shape site, or ShapeSite::None.
   */
  std::vector<uint32_t> m_leafSites;

  /**
   * @brief The parsed files.
   */
  std::vector<Syntax::File> m_sources;

  /**
   * @brief The setup file a folder includes after its model.ebg, among the sources, or -1 for none.
   */
  int m_setupSource = -1;
};

/**
 * @brief Read a model: a .ebg file, or a folder holding model.ebg, with setup.ebg included after it when present.
 * @tparam T Precision.
 * @param[in]     a_path      The file or folder.
 * @param[in,out] a_pool      Where meshes the model reads are built; it must outlive the model's object.
 * @param[in]     a_overrides Values for its params.
 * @return The model. Throws Error with every diagnostic when the model has errors.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
Model<T>
read(const std::string& a_path, Pool& a_pool, const Overrides& a_overrides = {});

/**
 * @brief Read a model from text.
 * @tparam T Precision.
 * @param[in]     a_text      The text.
 * @param[in]     a_name      The name of its file, in diagnostics; include()s and meshes are found from its folder.
 * @param[in,out] a_pool      Where meshes the model reads are built.
 * @param[in]     a_overrides Values for its params.
 * @return The model. Throws Error with every diagnostic when the model has errors.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
Model<T>
parse(std::string_view a_text, const std::string& a_name, Pool& a_pool, const Overrides& a_overrides = {});

/**
 * @brief Whether a domain is complete for a solver, and what is missing.
 */
struct Completeness
{
  /**
   * @brief Whether every subdomain has properties and every piece of surface bounding the domain is in a surface
   * group.
   */
  bool m_complete = true;

  /**
   * @brief What is missing (errors), and what may be (warnings), at the lines to change.
   */
  std::vector<Diagnostic> m_findings;
};

/**
 * @brief Check that a domain is complete for a solver: every subdomain has properties (its own or its regions'
 * material's), and every piece of surface bounding the domain (a boundary, or an interface between two subdomains) is
 * held by exactly one surface group. Surface couplings are checked too: both groups hold the same pieces.
 * @details The pieces are the ones the object may have (possiblePieces()); an uncovered one is an error when the
 * tape's rays find it (findSurfacePieces()), a warning otherwise (it may not exist).
 * @tparam T              Precision.
 * @tparam UserPrimitives The tape's user primitives.
 * @param[in] a_model  The model.
 * @param[in] a_tape   The tape compiled from its object.
 * @param[in] a_domain The domain's name.
 * @return The result.
 */
template <class T, class... UserPrimitives>
[[nodiscard]] EBGEOMETRY_HOST
Completeness
completeness(const Model<T>& a_model, const Tape<T, UserPrimitives...>& a_tape, std::string_view a_domain);

/**
 * @brief The part names of a built-in shape, in the order of its parts ("box": xmin, xmax, ymin, ymax, zmin, zmax).
 * @param[in] a_shape The shape's name in the language.
 * @param[in] a_sides A polygon prism's sides (its parts are side0, side1, ..., cap1, cap2).
 * @return The names; empty for a shape without parts or an unknown name.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<std::string>
partNames(std::string_view a_shape, uint32_t a_sides = 0);

/**
 * @brief Every name a model offers, as text: its surface labels, the parts of each shape written (with their labels),
 * its regions and materials, and its domains with their subdomains and groups. What `ebgstudio check --inventory`
 * prints.
 * @tparam T Precision.
 * @param[in] a_model The model.
 * @return The text.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
std::string
inventory(const Model<T>& a_model);

/**
 * @brief A model as text in canonical form: one statement per line, the layout a viewer writes, comments kept.
 * @tparam T Precision.
 * @param[in] a_model   The model.
 * @param[in] a_flatten Whether include()s are resolved into one text (they are kept otherwise, and only the model's
 *                      first file is printed).
 * @return The text.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
std::string
print(const Model<T>& a_model, bool a_flatten = false);

} // namespace Language
} // namespace EBGeometry

#include "EBGeometry_LanguageImplem.hpp"

#endif
