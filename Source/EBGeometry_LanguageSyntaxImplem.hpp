// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_LanguageSyntaxImplem.hpp
 * @brief  Implementation of EBGeometry_LanguageSyntax.hpp: the tokenizer and the parser.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_LANGUAGESYNTAXIMPLEM_HPP
#define EBGEOMETRY_LANGUAGESYNTAXIMPLEM_HPP

// Std includes
#include <cctype>
#include <cmath>
#include <cstring>

// Our includes
#include "EBGeometry_LanguageSyntax.hpp"

namespace EBGeometry {
namespace Language {
namespace Syntax {

inline bool
isReserved(std::string_view a_name) noexcept
{
  static constexpr const char* reserved[] = {"ebg",
                                             "param",
                                             "object",
                                             "overlay",
                                             "region",
                                             "material",
                                             "domain",
                                             "subdomain",
                                             "group",
                                             "couple",
                                             "include",
                                             "view",
                                             "outside",
                                             "true",
                                             "false",
                                             "pi"};

  for (const char* word : reserved) {
    if (a_name == word) {
      return true;
    }
  }

  return false;
}

inline std::vector<Lexeme>
tokenize(std::string_view a_text, const std::string& a_file, std::vector<Comment>* a_comments)
{
  std::vector<Lexeme> tokens;

  size_t   i      = 0;
  uint32_t line   = 1;
  size_t   lineAt = 0; // the byte where the current line starts

  const auto spanAt = [&](const size_t a_begin, const size_t a_end, const uint32_t a_line, const size_t a_lineAt) {
    return Span{a_file,
                static_cast<uint32_t>(a_begin),
                static_cast<uint32_t>(a_end),
                a_line,
                static_cast<uint32_t>(a_begin - a_lineAt + 1)};
  };

  const auto isNameStart = [](const char c) { return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_'; };
  const auto isNameChar  = [](const char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_'; };
  const auto isDigit     = [](const char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; };

  // A UTF-8 byte-order mark, as some editors write, is not part of the text.
  if (a_text.substr(0, 3) == "\xEF\xBB\xBF") {
    i      = 3;
    lineAt = 3;
  }

  while (i < a_text.size()) {
    const char c = a_text[i];

    if (c == '#') {
      const size_t begin = i;

      while (i < a_text.size() && a_text[i] != '\n') {
        ++i;
      }

      if (a_comments != nullptr) {
        size_t end = i;

        while (end > begin && (a_text[end - 1] == ' ' || a_text[end - 1] == '\t' || a_text[end - 1] == '\r')) {
          --end;
        }

        const bool ownLine =
          tokens.empty() || tokens.back().m_token == Token::Newline || tokens.back().m_span.m_line != line;

        a_comments->push_back(
          Comment{std::string(a_text.substr(begin, end - begin)), spanAt(begin, end, line, lineAt), ownLine});
      }

      continue;
    }

    if (c == '\n') {
      if (!tokens.empty() && tokens.back().m_token != Token::Newline) {
        Lexeme lexeme;

        lexeme.m_token = Token::Newline;
        lexeme.m_span  = spanAt(i, i + 1, line, lineAt);

        tokens.push_back(lexeme);
      }

      ++i;
      ++line;
      lineAt = i;

      continue;
    }

    if (c == ' ' || c == '\t' || c == '\r') {
      ++i;

      continue;
    }

    Lexeme lexeme;

    const size_t begin = i;

    if (isNameStart(c)) {
      while (i < a_text.size() && isNameChar(a_text[i])) {
        ++i;
      }

      lexeme.m_token = Token::Name;
      lexeme.m_text  = std::string(a_text.substr(begin, i - begin));
    }
    else if (isDigit(c) || (c == '.' && i + 1 < a_text.size() && isDigit(a_text[i + 1]))) {
      while (i < a_text.size() && isDigit(a_text[i])) {
        ++i;
      }

      if (i < a_text.size() && a_text[i] == '.') {
        ++i;

        while (i < a_text.size() && isDigit(a_text[i])) {
          ++i;
        }
      }

      if (i < a_text.size() && (a_text[i] == 'e' || a_text[i] == 'E')) {
        size_t j = i + 1;

        if (j < a_text.size() && (a_text[j] == '+' || a_text[j] == '-')) {
          ++j;
        }

        if (j < a_text.size() && isDigit(a_text[j])) {
          i = j;

          while (i < a_text.size() && isDigit(a_text[i])) {
            ++i;
          }
        }
      }

      if (i < a_text.size() && isNameStart(a_text[i])) {
        throw Error(spanAt(begin, i + 1, line, lineAt),
                    "a number runs into a name ('" + std::string(a_text.substr(begin, i + 1 - begin)) + "')");
      }

      lexeme.m_token  = Token::Number;
      lexeme.m_text   = std::string(a_text.substr(begin, i - begin));
      lexeme.m_number = std::strtod(lexeme.m_text.c_str(), nullptr);

      if (!std::isfinite(lexeme.m_number)) {
        throw Error(spanAt(begin, i, line, lineAt), "the number " + lexeme.m_text + " is too large");
      }
    }
    else if (c == '"') {
      ++i;

      std::string text;

      while (true) {
        if (i >= a_text.size() || a_text[i] == '\n') {
          throw Error(spanAt(begin, i, line, lineAt), "a string is not closed on its line");
        }
        if (a_text[i] == '"') {
          ++i;

          break;
        }
        if (a_text[i] == '\\' && i + 1 < a_text.size() && (a_text[i + 1] == '"' || a_text[i + 1] == '\\')) {
          text += a_text[i + 1];
          i += 2;

          continue;
        }

        text += a_text[i];
        ++i;
      }

      lexeme.m_token = Token::String;
      lexeme.m_text  = text;
    }
    else if (c != '\0' && std::strchr("(){},.=+-*/", c) != nullptr) {
      ++i;

      lexeme.m_token = Token::Symbol;
      lexeme.m_text  = std::string(1, c);
    }
    else {
      static const char* hex = "0123456789ABCDEF";

      const unsigned char u = static_cast<unsigned char>(c);

      if (u >= 0x80U) {
        throw Error(spanAt(begin, i + 1, line, lineAt),
                    std::string("unexpected byte 0x") + hex[u >> 4U] + hex[u & 15U] +
                      ": names and symbols are ASCII (text in quotes may be UTF-8)");
      }
      if (u < 0x20U || u == 0x7FU) {
        throw Error(spanAt(begin, i + 1, line, lineAt),
                    std::string("unexpected control character 0x") + hex[u >> 4U] + hex[u & 15U]);
      }

      throw Error(spanAt(begin, i + 1, line, lineAt), "unexpected character '" + std::string(1, c) + "'");
    }

    lexeme.m_span = spanAt(begin, i, line, lineAt);

    tokens.push_back(lexeme);
  }

  Lexeme end;

  end.m_token = Token::End;
  end.m_span  = spanAt(a_text.size(), a_text.size(), line, lineAt);

  tokens.push_back(end);

  return tokens;
}

/**
 * @brief The parser's state and its rules, one function per rule of the grammar (see the language's reference).
 */
class Parser
{
public:
  /**
   * @brief Parser over a file's tokens.
   * @param[in] a_tokens The tokens, ending with End.
   */
  explicit Parser(std::vector<Lexeme> a_tokens) : m_tokens(std::move(a_tokens))
  {}

  /**
   * @brief Parse the file: its version line and statements, collecting every syntax error.
   * @param[in,out] a_file   The file, its name and text set.
   * @param[out]    a_errors The syntax errors.
   */
  void
  file(File& a_file, std::vector<Diagnostic>& a_errors)
  {
    skipNewlines();

    if (!isName("ebg")) {
      a_errors.push_back(error(peek().m_span, "a file starts with its version line, 'ebg 0'"));

      return;
    }

    next();

    if (peek().m_token != Token::Number || peek().m_text.find_first_not_of("0123456789") != std::string::npos) {
      a_errors.push_back(error(peek().m_span, "the version is a whole number: 'ebg 0'"));

      return;
    }

    if (peek().m_text.size() > 6) {
      a_errors.push_back(error(peek().m_span, "version " + peek().m_text + " is unknown; this reader knows version 0"));

      return;
    }

    a_file.m_version = static_cast<uint32_t>(next().m_number);

    if (a_file.m_version != 0) {
      a_errors.push_back(
        error(m_tokens[m_at - 1].m_span,
              "version " + std::to_string(a_file.m_version) + " is unknown; this reader knows version 0"));

      return;
    }

    if (peek().m_token != Token::Newline && peek().m_token != Token::End) {
      a_errors.push_back(error(peek().m_span, "the version line holds the version only: 'ebg 0'"));

      return;
    }

    while (true) {
      skipNewlines();

      if (peek().m_token == Token::End) {
        break;
      }

      const size_t start = m_at;

      try {
        a_file.m_statements.push_back(statement());

        if (peek().m_token != Token::Newline && peek().m_token != Token::End) {
          fail(peek().m_span, "expected the end of the statement, found " + describe(peek()));
        }
      } catch (const Error& e) {
        a_errors.insert(a_errors.end(), e.diagnostics().begin(), e.diagnostics().end());

        m_parens = 0;
        m_depth  = 0;

        recover(start);
      }
    }
  }

  /**
   * @brief Parse the tokens as one expression.
   * @return The expression; an Error when anything follows it.
   */
  Node
  expressionOnly()
  {
    skipNewlines();

    Node node = expr();

    skipNewlines();

    if (peek().m_token != Token::End) {
      fail(peek().m_span, "expected one value, found " + describe(peek()) + " after it");
    }

    return node;
  }

private:
  /**
   * @brief The tokens.
   */
  std::vector<Lexeme> m_tokens;

  /**
   * @brief The next token's index.
   */
  size_t m_at = 0;

  /**
   * @brief How many parentheses are open: line ends inside them are skipped (a call's arguments may span lines).
   */
  int m_parens = 0;

  /**
   * @brief Where the last token taken ends (not a line end skipped since: spans end at what they hold).
   */
  uint32_t m_lastEnd = 0;

  /**
   * @brief How deep the expression being parsed is: its nesting, and its chains of operators and methods (each link
   * a level of the tree).
   */
  int m_depth = 0;

  /**
   * @brief The deepest an expression may be: the reader, the printer and the expression builder recurse through it,
   * so that a deeper one would overflow the stack.
   */
  static constexpr int MaxDepth = 256;

  /**
   * @brief One level deeper into an expression; an Error past MaxDepth.
   * @param[in] a_span Where.
   */
  void
  descend(const Span& a_span)
  {
    if (m_depth >= MaxDepth) {
      fail(a_span, "the expression is nested (or chained) more than " + std::to_string(MaxDepth) + " deep");
    }

    ++m_depth;
  }

  /**
   * @brief Restores the depth when a rule returns.
   */
  struct Level
  {
    /**
     * @brief The parser.
     */
    Parser& m_parser;

    /**
     * @brief The depth to restore.
     */
    int m_depth;

    /**
     * @brief Restore it.
     */
    ~Level()
    {
      m_parser.m_depth = m_depth;
    }
  };

  /**
   * @brief The next token.
   * @return It.
   */
  const Lexeme&
  peek() noexcept
  {
    while (m_parens > 0 && m_tokens[m_at].m_token == Token::Newline) {
      ++m_at;
    }

    return m_tokens[m_at];
  }

  /**
   * @brief The token at a place, or the first past the line ends there.
   * @param[in] a_at The place.
   * @return It.
   */
  [[nodiscard]] const Lexeme&
  pastNewlines(size_t a_at) const noexcept
  {
    while (m_tokens[a_at].m_token == Token::Newline) {
      ++a_at;
    }

    return m_tokens[a_at];
  }

  /**
   * @brief The token after the next one, past any line ends.
   * @return It.
   */
  const Lexeme&
  peekPastNewlines() const noexcept
  {
    size_t at = m_at;

    while (m_tokens[at].m_token == Token::Newline) {
      ++at;
    }

    return m_tokens[at];
  }

  /**
   * @brief Take the next token.
   * @return It.
   */
  const Lexeme&
  next() noexcept
  {
    const Lexeme& lexeme = peek();

    if (lexeme.m_token != Token::End) {
      ++m_at;
    }

    m_lastEnd = lexeme.m_span.m_end;
    if (lexeme.m_token == Token::Symbol && lexeme.m_text[0] == '(') {
      ++m_parens;
    }
    if (lexeme.m_token == Token::Symbol && lexeme.m_text[0] == ')' && m_parens > 0) {
      --m_parens;
    }

    return lexeme;
  }

  /**
   * @brief Where the last token taken ends.
   * @return The byte.
   */
  uint32_t
  lastEnd() const noexcept
  {
    return m_lastEnd;
  }

  /**
   * @brief A span from a first token's span to the last token taken.
   * @param[in] a_first The first token's span.
   * @return The span.
   */
  Span
  from(const Span& a_first) const
  {
    Span span = a_first;

    span.m_end = lastEnd();

    return span;
  }

  /**
   * @brief Skip line ends.
   */
  void
  skipNewlines() noexcept
  {
    while (peek().m_token == Token::Newline) {
      next();
    }
  }

  /**
   * @brief Whether the next token is a symbol.
   * @param[in] a_symbol The symbol.
   * @return Whether.
   */
  bool
  isSymbol(const char a_symbol) noexcept
  {
    return peek().m_token == Token::Symbol && peek().m_text[0] == a_symbol;
  }

  /**
   * @brief Whether the next token is a name.
   * @param[in] a_name The name.
   * @return Whether.
   */
  bool
  isName(const char* a_name) noexcept
  {
    return peek().m_token == Token::Name && peek().m_text == a_name;
  }

  /**
   * @brief A token in words, for messages.
   * @param[in] a_lexeme The token.
   * @return The words.
   */
  static std::string
  describe(const Lexeme& a_lexeme)
  {
    switch (a_lexeme.m_token) {
    case Token::End: {
      return "the end of the file";
    }
    case Token::Newline: {
      return "the end of the line";
    }
    case Token::String: {
      return "\"" + a_lexeme.m_text + "\"";
    }
    default: {
      return "'" + a_lexeme.m_text + "'";
    }
    }
  }

  /**
   * @brief An error diagnostic.
   * @param[in] a_span    Where.
   * @param[in] a_message What.
   * @return The diagnostic.
   */
  static Diagnostic
  error(const Span& a_span, const std::string& a_message)
  {
    return Diagnostic{Diagnostic::Severity::Error, a_span, a_message};
  }

  /**
   * @brief Stop the statement with an error.
   * @param[in] a_span    Where.
   * @param[in] a_message What.
   */
  [[noreturn]] static void
  fail(const Span& a_span, const std::string& a_message)
  {
    throw Error(a_span, a_message);
  }

  /**
   * @brief Take a symbol, or fail.
   * @param[in] a_symbol The symbol.
   * @param[in] a_where  What it is for, in words ("after the arguments").
   */
  void
  expect(const char a_symbol, const char* a_where)
  {
    if (!isSymbol(a_symbol)) {
      fail(peek().m_span, std::string("expected '") + a_symbol + "' " + a_where + ", found " + describe(peek()));
    }

    next();
  }

  /**
   * @brief Take a name that is not reserved, or fail.
   * @param[in] a_what What it names, in words.
   * @return The name's token.
   */
  const Lexeme&
  name(const char* a_what)
  {
    if (peek().m_token != Token::Name) {
      fail(peek().m_span, std::string("expected ") + a_what + ", found " + describe(peek()));
    }
    if (isReserved(peek().m_text)) {
      fail(peek().m_span, "'" + peek().m_text + "' is a reserved word and cannot be " + a_what);
    }

    return next();
  }

  /**
   * @brief Skip to the start of the next statement after an error: the first token of a later line that starts no
   * further right than the failed statement did (deeper lines are inside it), other than a closing brace, or the end.
   * @param[in] a_start Where the failed statement starts.
   */
  void
  recover(const size_t a_start) noexcept
  {
    const uint32_t column = m_tokens[a_start].m_span.m_column;

    // The error may have been found past a line end already (inside parentheses, line ends are skipped).
    if (m_at > a_start && m_tokens[m_at].m_token != Token::End && m_tokens[m_at - 1].m_token == Token::Newline &&
        this->startsStatement(m_tokens[m_at], column)) {
      return;
    }

    while (m_tokens[m_at].m_token != Token::End) {
      const bool newline = m_tokens[m_at].m_token == Token::Newline;

      ++m_at;

      if (newline && this->startsStatement(m_tokens[m_at], column)) {
        return;
      }
    }
  }

  /**
   * @brief Whether a token that begins a line can begin the next statement: it starts no further right than the failed
   * one, and is not a closing brace (which ends the block the error was in).
   * @param[in] a_token  The token.
   * @param[in] a_column The failed statement's column.
   * @return Whether.
   */
  [[nodiscard]] static bool
  startsStatement(const Lexeme& a_token, const uint32_t a_column) noexcept
  {
    return a_token.m_token != Token::End && a_token.m_span.m_column <= a_column &&
           !(a_token.m_token == Token::Symbol && a_token.m_text == "}");
  }

  /**
   * @brief statement := [param] name = expr | object expr | material name [properties] | domain name { items }
   * | couple name = (volume|surface)(a.x, b.y) [properties] | include("file") | view properties.
   * @return The statement.
   */
  Statement
  statement()
  {
    Statement s;

    const Span first = peek().m_span;

    if (isName("param")) {
      next();

      s.m_param = true;
    }

    if (!s.m_param && isName("object")) {
      next();

      s.m_kind = StatementKind::Object;

      // object name = shape: the object, and a value of that name.
      if (peek().m_token == Token::Name && m_tokens[m_at + 1].m_token == Token::Symbol &&
          m_tokens[m_at + 1].m_text == "=") {
        const Lexeme& n = name("the object's name");

        s.m_name     = n.m_text;
        s.m_nameSpan = n.m_span;

        next();
        skipNewlines();
      }

      s.m_expr = expr();
    }
    else if (!s.m_param && isName("material")) {
      next();

      const Lexeme& n = name("a material's name");

      s.m_kind     = StatementKind::Material;
      s.m_name     = n.m_text;
      s.m_nameSpan = n.m_span;

      if (isSymbol('{')) {
        s.m_properties = properties();
      }
    }
    else if (!s.m_param && isName("domain")) {
      next();

      const Lexeme& n = name("a domain's name");

      s.m_kind     = StatementKind::Domain;
      s.m_name     = n.m_text;
      s.m_nameSpan = n.m_span;

      domain(s);
    }
    else if (!s.m_param && isName("couple")) {
      next();

      couple(s);
    }
    else if (!s.m_param && isName("include")) {
      next();
      expect('(', "after include");

      if (peek().m_token != Token::String) {
        fail(peek().m_span, "include takes a file name in quotes: include(\"setup.ebg\")");
      }

      s.m_kind     = StatementKind::Include;
      s.m_nameSpan = peek().m_span;
      s.m_name     = next().m_text;

      expect(')', "after the file name");
    }
    else if (!s.m_param && isName("view")) {
      next();

      if (!isSymbol('{')) {
        fail(peek().m_span, "view takes a properties block: view { camera = (8, -6, 5) }");
      }

      s.m_kind       = StatementKind::View;
      s.m_properties = properties();
    }
    else {
      if (peek().m_token == Token::Name && !isReserved(peek().m_text) && m_tokens[m_at + 1].m_token == Token::Symbol &&
          m_tokens[m_at + 1].m_text == "=") {
        const Lexeme& n = next();

        next();
        skipNewlines();

        s.m_kind     = StatementKind::Value;
        s.m_name     = n.m_text;
        s.m_nameSpan = n.m_span;
        s.m_expr     = expr();
      }
      else if (!s.m_param && peek().m_token == Token::Name && !isReserved(peek().m_text) &&
               pastNewlines(m_at + 1).m_token == Token::Symbol && pastNewlines(m_at + 1).m_text == ".") {
        // name.label(...) on its own: labels for the shape the name holds.
        const Lexeme& n = peek();

        s.m_kind     = StatementKind::Label;
        s.m_name     = n.m_text;
        s.m_nameSpan = n.m_span;
        s.m_expr     = postfix();

        if (s.m_expr.m_kind != NodeKind::Method || s.m_expr.m_text != "label" ||
            s.m_expr.m_children[0].m_kind != NodeKind::Name) {
          fail(
            s.m_expr.m_span,
            "a statement that starts with a name is 'name = value', or 'name.label(...)' labeling the shape it holds");
        }
      }
      else if (s.m_param) {
        fail(peek().m_span, "param is followed by a name and a value: param radius = 0.5");
      }
      else if (peek().m_token == Token::Name && isReserved(peek().m_text)) {
        fail(peek().m_span, "'" + peek().m_text + "' cannot start a statement here");
      }
      else {
        fail(peek().m_span,
             "expected a statement (name = value, object, material, domain, couple, include, view), found " +
               describe(peek()));
      }
    }

    s.m_span = from(first);

    return s;
  }

  /**
   * @brief properties := { [key = expr {(,|newline) key = expr}] }.
   * @return The block.
   */
  Properties
  properties()
  {
    Properties block;

    const Span first = peek().m_span;

    block.m_written = true;

    expect('{', "to open the properties");
    skipNewlines();

    while (!isSymbol('}')) {
      Property p;

      const Lexeme& key = name("a property's name");

      p.m_key     = key.m_text;
      p.m_keySpan = key.m_span;

      while (isSymbol('.')) {
        next();

        const Lexeme& part = name("a property's name");

        p.m_key += "." + part.m_text;
        p.m_keySpan.m_end = part.m_span.m_end;
      }

      expect('=', "after a property's name");

      p.m_value = expr();

      block.m_entries.push_back(std::move(p));

      if (isSymbol(',')) {
        next();
      }
      else if (peek().m_token != Token::Newline && !isSymbol('}')) {
        fail(peek().m_span, "expected ',', a new line or '}' after a property, found " + describe(peek()));
      }

      skipNewlines();
    }

    next();

    block.m_span = from(first);

    return block;
  }

  /**
   * @brief domain name { (subdomain name = r, ... [properties] | group name = s, ... [properties]) ... }.
   * @param[in,out] a_statement The domain statement.
   */
  void
  domain(Statement& a_statement)
  {
    expect('{', "to open the domain");
    skipNewlines();

    while (!isSymbol('}')) {
      DomainItem item;

      const Span first = peek().m_span;

      if (isName("subdomain")) {
        item.m_subdomain = true;
      }
      else if (isName("group")) {
        item.m_subdomain = false;
      }
      else {
        fail(peek().m_span, "a domain holds subdomain and group lines, found " + describe(peek()));
      }

      next();

      const Lexeme& n = name(item.m_subdomain ? "a subdomain's name" : "a group's name");

      item.m_name     = n.m_text;
      item.m_nameSpan = n.m_span;

      expect('=', item.m_subdomain ? "after the subdomain's name" : "after the group's name");

      do {
        if (item.m_subdomain) {
          if (peek().m_token != Token::Name) {
            fail(peek().m_span, "a subdomain lists regions by name, found " + describe(peek()));
          }

          Node region;

          region.m_kind = NodeKind::Name;
          region.m_span = peek().m_span;
          region.m_text = next().m_text;

          item.m_list.push_back(std::move(region));
        }
        else {
          Node selector = postfix();

          if (selector.m_kind != NodeKind::Call) {
            fail(selector.m_span,
                 "a group lists selectors: label(\"x\"), interface(a, b), region(a), materials(m1, m2), remaining()");
          }

          item.m_list.push_back(std::move(selector));
        }
      } while (isSymbol(',') && (next(), skipNewlines(), true));

      if (isSymbol('{')) {
        item.m_properties = properties();
      }

      item.m_span = from(first);

      a_statement.m_items.push_back(std::move(item));

      if (peek().m_token != Token::Newline && !isSymbol('}')) {
        fail(peek().m_span, "expected a new line or '}' after a domain's line, found " + describe(peek()));
      }

      skipNewlines();
    }

    next();
  }

  /**
   * @brief couple name = (volume|surface)(domain.part, domain.part) [properties].
   * @param[in,out] a_statement The statement.
   */
  void
  couple(Statement& a_statement)
  {
    const Lexeme& n = name("a coupling's name");

    a_statement.m_kind     = StatementKind::Couple;
    a_statement.m_name     = n.m_text;
    a_statement.m_nameSpan = n.m_span;

    expect('=', "after the coupling's name");

    if (!isName("volume") && !isName("surface")) {
      fail(peek().m_span, "a coupling is volume(a.x, b.y) or surface(a.x, b.y), found " + describe(peek()));
    }

    a_statement.m_coupleKind = next().m_text;

    expect('(', "after the coupling's kind");

    for (int end = 0; end < 2; end++) {
      const Lexeme& d = name("a domain's name");

      a_statement.m_ends[2 * end]     = d.m_text;
      a_statement.m_endSpans[2 * end] = d.m_span;

      expect('.', "between a domain and its subdomain or group");

      const Lexeme& p = name("a subdomain's or group's name");

      a_statement.m_ends[2 * end + 1]     = p.m_text;
      a_statement.m_endSpans[2 * end + 1] = p.m_span;

      expect(end == 0 ? ',' : ')', end == 0 ? "between the coupling's ends" : "after the coupling's ends");
    }

    if (isSymbol('{')) {
      a_statement.m_properties = properties();
    }
  }

  /**
   * @brief A binary node.
   * @param[in] a_kind  The kind.
   * @param[in] a_left  The left operand.
   * @param[in] a_right The right operand.
   * @return The node.
   */
  static Node
  binary(const NodeKind a_kind, Node a_left, Node a_right)
  {
    Node node;

    node.m_kind       = a_kind;
    node.m_span       = a_left.m_span;
    node.m_span.m_end = a_right.m_span.m_end;

    node.m_children.push_back(std::move(a_left));
    node.m_children.push_back(std::move(a_right));

    return node;
  }

  /**
   * @brief expr := term {(+|-) term}.
   * @return The node.
   */
  Node
  expr()
  {
    const Level level{*this, m_depth};

    this->descend(peek().m_span);

    Node left = term();

    while (isSymbol('+') || isSymbol('-')) {
      this->descend(peek().m_span);

      const NodeKind kind = next().m_text == "+" ? NodeKind::Add : NodeKind::Subtract;

      skipNewlines();

      left = binary(kind, std::move(left), term());
    }

    return left;
  }

  /**
   * @brief term := unary {(*|/) unary}.
   * @return The node.
   */
  Node
  term()
  {
    const Level level{*this, m_depth};

    Node left = unary();

    while (isSymbol('*') || isSymbol('/')) {
      this->descend(peek().m_span);

      const NodeKind kind = next().m_text == "*" ? NodeKind::Multiply : NodeKind::Divide;

      skipNewlines();

      left = binary(kind, std::move(left), unary());
    }

    return left;
  }

  /**
   * @brief unary := [-] postfix.
   * @return The node.
   */
  Node
  unary()
  {
    if (isSymbol('-')) {
      const Level level{*this, m_depth};

      this->descend(peek().m_span);

      const Span first = next().m_span;

      skipNewlines();

      if (isSymbol('-')) {
        fail(peek().m_span, "two '-' in a row; write -(-x)");
      }

      Node node;

      node.m_kind = NodeKind::Negate;

      node.m_children.push_back(unary());

      node.m_span = from(first);

      return node;
    }

    return postfix();
  }

  /**
   * @brief postfix := primary {.name(args)}; a method may start the next line.
   * @return The node.
   */
  Node
  postfix()
  {
    const Level level{*this, m_depth};

    Node node = primary();

    while (isSymbol('.') || (peek().m_token == Token::Newline && peekPastNewlines().m_token == Token::Symbol &&
                             peekPastNewlines().m_text == ".")) {
      skipNewlines();

      this->descend(peek().m_span);

      next();

      if (peek().m_token != Token::Name) {
        fail(peek().m_span, "expected an operation's name after '.', found " + describe(peek()));
      }

      Node method;

      method.m_kind     = NodeKind::Method;
      method.m_nameSpan = peek().m_span;
      method.m_text     = next().m_text;

      if (!isSymbol('(')) {
        fail(peek().m_span, "'." + method.m_text + "' is an operation and takes arguments in parentheses");
      }

      method.m_span = node.m_span;
      method.m_children.push_back(std::move(node));
      method.m_keys.emplace_back();

      arguments(method);

      method.m_span.m_end = lastEnd();

      node = std::move(method);
    }

    return node;
  }

  /**
   * @brief args := ( [arg {, arg}] ), arg := expr | name = expr; appended to a call's children and keys.
   * @param[in,out] a_call The call.
   */
  void
  arguments(Node& a_call)
  {
    expect('(', "to open the arguments");

    while (!isSymbol(')')) {
      std::string key;

      // Inside the parentheses line ends are skipped, between a keyword and its '=' too.
      if (peek().m_token == Token::Name && pastNewlines(m_at + 1).m_token == Token::Symbol &&
          pastNewlines(m_at + 1).m_text == "=") {
        key = next().m_text;

        next();
      }

      a_call.m_children.push_back(expr());
      a_call.m_keys.push_back(key);

      if (isSymbol(',')) {
        next();

        if (isSymbol(')')) {
          fail(peek().m_span, "expected an argument after ',', found ')'");
        }
      }
      else if (!isSymbol(')')) {
        fail(peek().m_span, "expected ',' or ')' in the arguments, found " + describe(peek()));
      }
    }

    next();
  }

  /**
   * @brief primary := number | string | true | false | pi | name | name(args) | (expr) | (expr, expr, expr)
   * | overlay { region name = expr [material name] ... }.
   * @return The node.
   */
  Node
  primary()
  {
    Node node;

    const Span first = peek().m_span;

    node.m_span = first;

    switch (peek().m_token) {
    case Token::Number: {
      node.m_kind   = NodeKind::Number;
      node.m_text   = peek().m_text;
      node.m_number = next().m_number;

      return node;
    }
    case Token::String: {
      node.m_kind = NodeKind::String;
      node.m_text = next().m_text;

      return node;
    }
    case Token::Name: {
      break;
    }
    case Token::Symbol: {
      if (isSymbol('(')) {
        next();

        Node inner = expr();

        if (isSymbol(',')) {
          node.m_kind = NodeKind::Vector;

          node.m_children.push_back(std::move(inner));

          next();

          node.m_children.push_back(expr());

          // Two values make a pair, (a, b); three a vector.
          if (isSymbol(')')) {
            node.m_kind = NodeKind::Pair;
          }
          else {
            expect(',', "between a vector's components");

            node.m_children.push_back(expr());
          }

          expect(')', node.m_kind == NodeKind::Pair ? "after a pair" : "after a vector's three components");

          node.m_span = from(first);

          return node;
        }

        expect(')', "to close the parenthesis");

        inner.m_span = from(first);

        return inner;
      }

      fail(peek().m_span, "expected a value, found " + describe(peek()));
    }
    default: {
      fail(peek().m_span, "expected a value, found " + describe(peek()));
    }
    }

    const std::string& word = peek().m_text;

    if (word == "true" || word == "false") {
      node.m_kind   = NodeKind::Bool;
      node.m_text   = word;
      node.m_number = word == "true" ? 1.0 : 0.0;

      next();

      return node;
    }
    if (word == "pi") {
      node.m_kind   = NodeKind::Number;
      node.m_text   = "pi";
      node.m_number = 3.14159265358979323846;

      next();

      return node;
    }
    if (word == "overlay") {
      next();

      return overlay(first);
    }
    // `outside` names region 0 in selectors and subdomains; region(...) is a selector.
    const bool selector =
      word == "region" && m_tokens[m_at + 1].m_token == Token::Symbol && m_tokens[m_at + 1].m_text == "(";

    if (isReserved(word) && word != "outside" && !selector) {
      fail(peek().m_span, "'" + word + "' is a reserved word and not a value");
    }

    node.m_text     = word;
    node.m_nameSpan = next().m_span;

    if (isSymbol('(')) {
      node.m_kind = NodeKind::Call;

      arguments(node);
    }
    else {
      node.m_kind = NodeKind::Name;
    }

    node.m_span = from(first);

    return node;
  }

  /**
   * @brief overlay { region name = expr [material name] ... }, after the keyword.
   * @param[in] a_first The keyword's span.
   * @return The node.
   */
  Node
  overlay(const Span& a_first)
  {
    Node node;

    node.m_kind = NodeKind::Overlay;

    expect('{', "after overlay");

    // Its regions are lines, also inside parentheses (an overlay clipped by intersection(...)).
    const int parens = m_parens;

    m_parens = 0;

    skipNewlines();

    while (!isSymbol('}')) {
      if (!isName("region")) {
        fail(peek().m_span,
             "an overlay holds region lines, region <name> = <shape> [material <name>], found " + describe(peek()));
      }

      const Span first = next().m_span;

      Node region;

      region.m_kind = NodeKind::Region;

      const Lexeme& n = name("a region's name");

      region.m_text     = n.m_text;
      region.m_nameSpan = n.m_span;

      expect('=', "after the region's name");

      region.m_children.push_back(expr());

      if (isName("material")) {
        next();

        const Lexeme& m = name("a material's name");

        region.m_material     = m.m_text;
        region.m_materialSpan = m.m_span;
      }

      region.m_span = from(first);

      node.m_children.push_back(std::move(region));

      if (peek().m_token != Token::Newline && !isSymbol('}')) {
        fail(peek().m_span, "expected a new line or '}' after a region, found " + describe(peek()));
      }

      skipNewlines();
    }

    m_parens = parens;

    next();

    if (node.m_children.empty()) {
      fail(a_first, "an overlay needs at least one region");
    }

    node.m_span = from(a_first);

    return node;
  }
};

inline File
parse(std::string_view a_text, const std::string& a_file)
{
  File file;

  file.m_name = a_file;
  file.m_text = std::string(a_text);

  std::vector<Diagnostic> errors;

  Parser parser(tokenize(a_text, a_file, &file.m_comments));

  parser.file(file, errors);

  if (!errors.empty()) {
    throw Error(std::move(errors));
  }

  return file;
}

inline Node
parseExpression(std::string_view a_text, const std::string& a_name)
{
  Parser parser(tokenize(a_text, a_name));

  return parser.expressionOnly();
}

} // namespace Syntax
} // namespace Language
} // namespace EBGeometry

#endif
