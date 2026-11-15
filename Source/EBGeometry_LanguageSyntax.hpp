// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_LanguageSyntax.hpp
 * @brief  The .ebg language's text: positions, diagnostics, tokens and the syntax tree a file is parsed into.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_LANGUAGESYNTAX_HPP
#define EBGEOMETRY_LANGUAGESYNTAX_HPP

// Std includes
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {
namespace Language {

/**
 * @brief Where a piece of text is: its file, its first and one-past-last byte, and the line and column of its first
 * byte (both from 1). What every diagnostic points at, and what a viewer edits.
 */
struct Span
{
  /**
   * @brief The file, as the model names it (the path given to read(), or the name given to parse()).
   */
  std::string m_file;

  /**
   * @brief The first byte.
   */
  uint32_t m_begin = 0;

  /**
   * @brief One past the last byte.
   */
  uint32_t m_end = 0;

  /**
   * @brief The line of the first byte, from 1; 0 for a span that is a whole file or no file (an override).
   */
  uint32_t m_line = 0;

  /**
   * @brief The column of the first byte, from 1, in bytes.
   */
  uint32_t m_column = 0;
};

/**
 * @brief One error or warning, at a span.
 */
struct Diagnostic
{
  /**
   * @brief Whether a diagnostic stops the model.
   */
  enum class Severity
  {
    Error,  ///< The model cannot be built.
    Warning ///< The model is built; something looks unintended.
  };

  /**
   * @brief Error or warning.
   */
  Severity m_severity = Severity::Error;

  /**
   * @brief Where.
   */
  Span m_span;

  /**
   * @brief What is wrong, in words.
   */
  std::string m_message;

  /**
   * @brief The diagnostic as one line: "file:line:column: error: message" ("file: error: message" for a whole file).
   * @return The line.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  str() const
  {
    const std::string where =
      m_span.m_line == 0 ? m_span.m_file
                         : m_span.m_file + ":" + std::to_string(m_span.m_line) + ":" + std::to_string(m_span.m_column);

    return where + ": " + (m_severity == Severity::Error ? "error: " : "warning: ") + m_message;
  }
};

/**
 * @brief What reading a model throws when it has errors: every error found (not only the first), and the warnings.
 * @details what() is every diagnostic, one per line, errors first.
 */
class Error : public std::runtime_error
{
public:
  /**
   * @brief Error with its diagnostics.
   * @param[in] a_diagnostics The diagnostics, at least one an error.
   */
  EBGEOMETRY_HOST
  explicit Error(std::vector<Diagnostic> a_diagnostics)
    : std::runtime_error(joined(a_diagnostics)), m_diagnostics(std::move(a_diagnostics))
  {}

  /**
   * @brief One error at a span.
   * @param[in] a_span    Where.
   * @param[in] a_message What.
   */
  EBGEOMETRY_HOST
  Error(const Span& a_span, const std::string& a_message)
    : Error(std::vector<Diagnostic>{Diagnostic{Diagnostic::Severity::Error, a_span, a_message}})
  {}

  /**
   * @brief The diagnostics, errors first.
   * @return The diagnostics.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<Diagnostic>&
  diagnostics() const noexcept
  {
    return m_diagnostics;
  }

private:
  /**
   * @brief The diagnostics.
   */
  std::vector<Diagnostic> m_diagnostics;

  /**
   * @brief Every diagnostic, one per line, errors first.
   * @param[in] a_diagnostics The diagnostics.
   * @return The lines.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static std::string
  joined(const std::vector<Diagnostic>& a_diagnostics)
  {
    std::string text;

    for (const auto severity : {Diagnostic::Severity::Error, Diagnostic::Severity::Warning}) {
      for (const Diagnostic& d : a_diagnostics) {
        if (d.m_severity == severity) {
          text += (text.empty() ? "" : "\n") + d.str();
        }
      }
    }

    return text;
  }
};

/**
 * @brief The syntax of the language: tokens, the tree a file is parsed into, and the parser. Used by the reader and
 * the printer; a user reads models through Language::read().
 */
namespace Syntax {

/**
 * @brief The kinds of token.
 */
enum class Token : uint8_t
{
  End,     ///< The end of the text.
  Newline, ///< One or more line ends (a statement's end, outside brackets).
  Name,    ///< A name or a keyword.
  Number,  ///< A number.
  String,  ///< A double-quoted string, its text unescaped.
  Symbol   ///< One of ( ) { } , . = + - * /.
};

/**
 * @brief A token: its kind, its text (a symbol's character, a name, a string's unescaped text, a number as written),
 * and where.
 */
struct Lexeme
{
  /**
   * @brief The kind.
   */
  Token m_token = Token::End;

  /**
   * @brief The text.
   */
  std::string m_text;

  /**
   * @brief A number's value.
   */
  double m_number = 0.0;

  /**
   * @brief Where.
   */
  Span m_span;
};

/**
 * @brief A comment, kept for the printer: its text (from the `#`) and where it is.
 */
struct Comment
{
  /**
   * @brief The text, from the `#` to the end of the line.
   */
  std::string m_text;

  /**
   * @brief Where it is.
   */
  Span m_span;

  /**
   * @brief Whether it is alone on its line (otherwise it follows code).
   */
  bool m_ownLine = true;
};

/**
 * @brief Split a file into tokens.
 * @details `#` starts a comment to the end of the line. Consecutive line ends are one Newline token, which the parser
 * skips inside parentheses (a call's arguments may span lines) and where a statement cannot end (after an operator, a
 * comma or an opening brace).
 * @param[in]  a_text     The file's text.
 * @param[in]  a_file     The file's name, for spans.
 * @param[out] a_comments The comments, if not nullptr.
 * @return The tokens, ending with End. Throws Error at the first character that starts no token.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<Lexeme>
tokenize(std::string_view a_text, const std::string& a_file, std::vector<Comment>* a_comments = nullptr);

/**
 * @brief The kinds of node of an expression.
 */
enum class NodeKind : uint8_t
{
  Number,   ///< m_number.
  String,   ///< m_text.
  Bool,     ///< m_number 1 or 0.
  Name,     ///< A name: m_text.
  Vector,   ///< (x, y, z): three children.
  Pair,     ///< (a, b): two children; only a selector's keyword takes one (between = (a, b)).
  Negate,   ///< -a: one child.
  Add,      ///< a + b.
  Subtract, ///< a - b.
  Multiply, ///< a * b.
  Divide,   ///< a / b.
  Call,     ///< m_text(args): the children are the arguments, m_keys their keywords ("" for a positional one).
  Method,   ///< a.m_text(args): the first child is a, the others the arguments, m_keys their keywords.
  Overlay,  ///< overlay { region ... }: the children are Region nodes.
  Region    ///< region m_text = child [material m_material], inside an overlay.
};

/**
 * @brief A node of an expression.
 */
struct Node
{
  /**
   * @brief The kind.
   */
  NodeKind m_kind = NodeKind::Number;

  /**
   * @brief A number's value, or a bool's (1 or 0).
   */
  double m_number = 0.0;

  /**
   * @brief A name, a string, a called function's name, or a region's name.
   */
  std::string m_text;

  /**
   * @brief Where the node is, its operands included.
   */
  Span m_span;

  /**
   * @brief Where its name is: a call's function, a method, a region.
   */
  Span m_nameSpan;

  /**
   * @brief Operands or arguments.
   */
  std::vector<Node> m_children;

  /**
   * @brief Each argument's keyword ("" for positional), for Call and Method (for Method, the receiver's is "").
   */
  std::vector<std::string> m_keys;

  /**
   * @brief A region's material, empty for none.
   */
  std::string m_material;

  /**
   * @brief Where the material's name is.
   */
  Span m_materialSpan;
};

/**
 * @brief A key and value of a properties block, `key = expr`; the key may be dotted (color.copper).
 */
struct Property
{
  /**
   * @brief The key.
   */
  std::string m_key;

  /**
   * @brief Where the key is.
   */
  Span m_keySpan;

  /**
   * @brief The value.
   */
  Node m_value;
};

/**
 * @brief A properties block, `{ key = value, ... }`, and where it is.
 */
struct Properties
{
  /**
   * @brief Whether the block was written (an empty `{}` included).
   */
  bool m_written = false;

  /**
   * @brief The entries, in order.
   */
  std::vector<Property> m_entries;

  /**
   * @brief Where the block is.
   */
  Span m_span;
};

/**
 * @brief One item of a domain: a subdomain or a surface group.
 */
struct DomainItem
{
  /**
   * @brief Whether it is a subdomain (a group otherwise).
   */
  bool m_subdomain = true;

  /**
   * @brief Its name.
   */
  std::string m_name;

  /**
   * @brief Where its name is.
   */
  Span m_nameSpan;

  /**
   * @brief Where the item is.
   */
  Span m_span;

  /**
   * @brief A subdomain's regions (Name nodes) or a group's selectors (Call nodes).
   */
  std::vector<Node> m_list;

  /**
   * @brief Its properties.
   */
  Properties m_properties;
};

/**
 * @brief The kinds of statement.
 */
enum class StatementKind : uint8_t
{
  Value,    ///< [param] name = expr.
  Object,   ///< object [name =] expr.
  Material, ///< material name [properties].
  Domain,   ///< domain name { items }.
  Couple,   ///< couple name = volume|surface(a.x, b.y) [properties].
  Include,  ///< include("file").
  View,     ///< view properties.
  Label     ///< name.label(...): labels for a named shape, wherever it is used.
};

/**
 * @brief A statement of a file.
 */
struct Statement
{
  /**
   * @brief The kind.
   */
  StatementKind m_kind = StatementKind::Value;

  /**
   * @brief Whether a value is a param.
   */
  bool m_param = false;

  /**
   * @brief The name it defines (value, the object when named, material, domain, coupling), or an include's file.
   */
  std::string m_name;

  /**
   * @brief Where the name is.
   */
  Span m_nameSpan;

  /**
   * @brief Where the statement is.
   */
  Span m_span;

  /**
   * @brief A value's or the object's expression; a label statement's call (a method on a name).
   */
  Node m_expr;

  /**
   * @brief A material's, a coupling's or the view's properties.
   */
  Properties m_properties;

  /**
   * @brief A domain's items.
   */
  std::vector<DomainItem> m_items;

  /**
   * @brief A coupling's kind ("volume" or "surface") and its two ends, each a domain and a subdomain or group.
   */
  std::string m_coupleKind;

  /**
   * @brief A coupling's ends: domain A, part A, domain B, part B.
   */
  std::string m_ends[4];

  /**
   * @brief Where each end's names are.
   */
  Span m_endSpans[4];
};

/**
 * @brief A parsed file: its version and statements, its text, and the comments the printer keeps.
 */
struct File
{
  /**
   * @brief The file's name.
   */
  std::string m_name;

  /**
   * @brief The file's text.
   */
  std::string m_text;

  /**
   * @brief The version on its first line.
   */
  uint32_t m_version = 0;

  /**
   * @brief The statements, in order.
   */
  std::vector<Statement> m_statements;

  /**
   * @brief The comments, in order.
   */
  std::vector<Comment> m_comments;
};

/**
 * @brief Parse a file.
 * @param[in] a_text The text.
 * @param[in] a_file The file's name, for spans.
 * @return The file. Throws Error with every syntax error found, the parser resuming at the next statement.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline File
parse(std::string_view a_text, const std::string& a_file);

/**
 * @brief Parse one expression: a value given outside a file (an override, `-D name=value`).
 * @param[in] a_text The text.
 * @param[in] a_name A name for its spans ("-D radius").
 * @return The expression. Throws Error when the text is not one expression.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline Node
parseExpression(std::string_view a_text, const std::string& a_name);

/**
 * @brief The words a name may not be.
 * @param[in] a_name A name.
 * @return Whether it is reserved.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline bool
isReserved(std::string_view a_name) noexcept;

} // namespace Syntax
} // namespace Language
} // namespace EBGeometry

#include "EBGeometry_LanguageSyntaxImplem.hpp"

#endif
