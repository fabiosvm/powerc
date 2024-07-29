//
// lexer.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "lexer.h"
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define char_at(l, i)   ((l)->curr[(i)])
#define current_char(l) char_at(l, 0)

static inline bool skip_space(Lexer *lex);
static inline bool skip_comment(Lexer *lex);
static inline void next_char(Lexer *lex);
static inline void next_chars(Lexer *lex, int length);
static inline bool match_char(Lexer *lex, char c, TokenKind kind);
static inline bool match_chars(Lexer *lex, const char *chars, TokenKind kind);
static inline bool match_keyword(Lexer *lex, const char *kw, TokenKind kind);
static inline bool match_number(Lexer *lex);
static inline bool match_rune(Lexer *lex);
static inline bool match_string(Lexer *lex);
static inline bool match_ident(Lexer *lex);
static inline Token token(Lexer *lex, TokenKind kind, int length, char *chars);
static inline void lexical_error(Lexer *lex, const char *fmt, ...);

static inline bool skip_space(Lexer *lex)
{
  if (!isspace(current_char(lex)))
    return false;
  do
    next_char(lex);
  while (isspace(current_char(lex)));
  return true;
}

static inline bool skip_comment(Lexer *lex)
{
  if (current_char(lex) != '/')
    return false;
  if (char_at(lex, 1) == '/')
  {
    next_chars(lex, 2);
    while (current_char(lex) != '\n')
      next_char(lex);
    return true;
  }
  if (char_at(lex, 1) != '*')
    return false;
  next_chars(lex, 2);
  while (current_char(lex) != '*' || char_at(lex, 1) != '/')
    next_char(lex);
  next_chars(lex, 2);
  return true;
}

static inline void next_char(Lexer *lex)
{
  if (current_char(lex) == '\n')
  {
    ++lex->ln;
    lex->col = 1;
    ++lex->curr;
    return;
  }
  ++lex->col;
  ++lex->curr;
}

static inline void next_chars(Lexer *lex, int length)
{
  for (int i = 0; i < length; ++i)
    next_char(lex);
}

static inline bool match_char(Lexer *lex, char c, TokenKind kind)
{
  if (current_char(lex) != c)
    return false;
  lex->token = token(lex, kind, 1, lex->curr);
  next_char(lex);
  return true;
}

static inline bool match_chars(Lexer *lex, const char *chars, TokenKind kind)
{
  int length = (int) strlen(chars);
  if (memcmp(lex->curr, chars, length))
    return false;
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_keyword(Lexer *lex, const char *kw, TokenKind kind)
{
  int length = (int) strlen(kw);
  if (strncmp(lex->curr, kw, length)
   || isalnum(char_at(lex, length))
   || char_at(lex, length) == '_')
    return false;
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_number(Lexer *lex)
{
  int length = 0;
  if (char_at(lex, length) == '0')
    ++length;
  else
  {
    if (char_at(lex, length) < '1' || char_at(lex, length) > '9')
      return false;
    ++length;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  TokenKind kind = TOKEN_KIND_INT;
  if (char_at(lex, length) == '.')
  {
    kind = TOKEN_KIND_FLOAT;
    if (!isdigit(char_at(lex, length + 1)))
      goto end;
    length += 2;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  if (char_at(lex, length) == 'e' || char_at(lex, length) == 'E')
  {
    ++length;
    if (char_at(lex, length) == '+' || char_at(lex, length) == '-')
      ++length;
    if (!isdigit(char_at(lex, length)))
      return false;
    ++length;
    while (isdigit(char_at(lex, length)))
      ++length;
  }
  if (isalnum(char_at(lex, length)) || char_at(lex, length) == '_')
    return false;
end:
  lex->token = token(lex, kind, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline bool match_rune(Lexer *lex)
{
  if (current_char(lex) != '\'')
    return false;
  if (current_char(lex) == '\'')
    return false;
  if (char_at(lex, 1) == '\0')
    lexical_error(lex, "unclosed rune literal");
  if (char_at(lex, 2) != '\'')
    return false;
  lex->token = token(lex, TOKEN_KIND_RUNE, 1, &lex->curr[1]);
  next_chars(lex, 3);
  return true;
}

static inline bool match_string(Lexer *lex)
{
  if (current_char(lex) != '\"')
    return false;
  int length = 1;
  for (;;)
  {
    if (char_at(lex, length) == '\"')
    {
      ++length;
      break;
    }
    if (char_at(lex, length) == '\0')
      lexical_error(lex, "unclosed string literal");
    ++length;
  }
  lex->token = token(lex, TOKEN_KIND_STRING, length - 2, &lex->curr[1]);
  next_chars(lex, length);
  return true;
}

static inline bool match_ident(Lexer *lex)
{
  if (current_char(lex) != '_' && !isalpha(current_char(lex)))
    return false;
  int length = 1;
  while (char_at(lex, length) == '_' || isalnum(char_at(lex, length)))
    ++length;
  lex->token = token(lex, TOKEN_KIND_IDENT, length, lex->curr);
  next_chars(lex, length);
  return true;
}

static inline Token token(Lexer *lex, TokenKind kind, int length,
  char *chars)
{
  return (Token) {
    .kind = kind,
    .ln = lex->ln,
    .col = lex->col,
    .length = length,
    .chars = chars
  };
}

static inline void lexical_error(Lexer *lex, const char *fmt, ...)
{
  fprintf(stderr, "\nERROR: ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n--> %s:%d:%d\n", lex->file, lex->ln, lex->col);
  exit(EXIT_FAILURE);
}

const char *token_kind_name(TokenKind kind)
{
  char *name = NULL;
  switch (kind)
  {
  case TOKEN_KIND_EOF:          name = "Eof";         break;
  case TOKEN_KIND_COMMA:        name = "Comma";       break;
  case TOKEN_KIND_COLON:        name = "Colon";       break;
  case TOKEN_KIND_SEMICOLON:    name = "Semicolon";   break;
  case TOKEN_KIND_LPAREN:       name = "LParen";      break;
  case TOKEN_KIND_RPAREN:       name = "RParen";      break;
  case TOKEN_KIND_LBRACKET:     name = "LBracket";    break;
  case TOKEN_KIND_RBRACKET:     name = "RBracket";    break;
  case TOKEN_KIND_LBRACE:       name = "LBrace";      break;
  case TOKEN_KIND_RBRACE:       name = "RBrace";      break;
  case TOKEN_KIND_QMARK:        name = "QMark";       break;
  case TOKEN_KIND_PIPEEQ:       name = "PipeEq";      break;
  case TOKEN_KIND_PIPEPIPE:     name = "PipePipe";    break;
  case TOKEN_KIND_PIPE:         name = "Pipe";        break;
  case TOKEN_KIND_AMPEQ:        name = "AmpEq";       break;
  case TOKEN_KIND_AMPAMP:       name = "AmpAmp";      break;
  case TOKEN_KIND_AMP:          name = "Amp";         break;
  case TOKEN_KIND_CARETEQ:      name = "CaretEq";     break;
  case TOKEN_KIND_CARET:        name = "Caret";       break;
  case TOKEN_KIND_EQEQ:         name = "EqEq";        break;
  case TOKEN_KIND_EQ:           name = "Eq";          break;
  case TOKEN_KIND_BANGEQ:       name = "BangEq";      break;
  case TOKEN_KIND_BANG:         name = "Bang";        break;
  case TOKEN_KIND_TILDE:        name = "Tilde";       break;
  case TOKEN_KIND_LE:           name = "Le";          break;
  case TOKEN_KIND_LTLTEQ:       name = "LtLtEq";      break;
  case TOKEN_KIND_LTLT:         name = "LtLt";        break;
  case TOKEN_KIND_LT:           name = "Lt";          break;
  case TOKEN_KIND_GE:           name = "Ge";          break;
  case TOKEN_KIND_GTGTEQ:       name = "GtGtEq";      break;
  case TOKEN_KIND_GTGT:         name = "GtGt";        break;
  case TOKEN_KIND_GT:           name = "Gt";          break;
  case TOKEN_KIND_DOTDOT:       name = "DotDot";      break;
  case TOKEN_KIND_DOT:          name = "Dot";         break;
  case TOKEN_KIND_PLUSEQ:       name = "PlusEq";      break;
  case TOKEN_KIND_PLUS:         name = "Plus";        break;
  case TOKEN_KIND_ARROW:        name = "Arrow";       break;
  case TOKEN_KIND_MINUSEQ:      name = "MinusEq";     break;
  case TOKEN_KIND_MINUS:        name = "Minus";       break;
  case TOKEN_KIND_STAREQ:       name = "StarEq";      break;
  case TOKEN_KIND_STAR:         name = "Star";        break;
  case TOKEN_KIND_SLASHEQ:      name = "SlashEq";     break;
  case TOKEN_KIND_SLASH:        name = "Slash";       break;
  case TOKEN_KIND_PERCENTEQ:    name = "PercentEq";   break;
  case TOKEN_KIND_PERCENT:      name = "Percent";     break;
  case TOKEN_KIND_INT:          name = "Int";         break;
  case TOKEN_KIND_FLOAT:        name = "Float";       break;
  case TOKEN_KIND_RUNE:         name = "Rune";        break;
  case TOKEN_KIND_STRING:       name = "String";      break;
  case TOKEN_KIND_AS_KW:        name = "AsKw";        break;
  case TOKEN_KIND_BREAK_KW:     name = "BreakKw";     break;
  case TOKEN_KIND_CONTINUE_KW:  name = "ContinueKw";  break;
  case TOKEN_KIND_DO_KW:        name = "DoKw";        break;
  case TOKEN_KIND_ELSE_KW:      name = "ElseKw";      break;
  case TOKEN_KIND_FALSE_KW:     name = "FalseKw";     break;
  case TOKEN_KIND_FN_KW:        name = "FnKw";        break;
  case TOKEN_KIND_FOR_KW:       name = "ForKw";       break;
  case TOKEN_KIND_IF_KW:        name = "IfKw";        break;
  case TOKEN_KIND_IMPORT_KW:    name = "ImportKw";    break;
  case TOKEN_KIND_IN_KW:        name = "InKw";        break;
  case TOKEN_KIND_INOUT_KW:     name = "InoutKw";     break;
  case TOKEN_KIND_INTERFACE_KW: name = "InterfaceKw"; break;
  case TOKEN_KIND_LET_KW:       name = "LetKw";       break;
  case TOKEN_KIND_LOOP_KW:      name = "LoopKw";      break;
  case TOKEN_KIND_NEW_KW:       name = "NewKw";       break;
  case TOKEN_KIND_NULL_KW:      name = "NullKw";      break;
  case TOKEN_KIND_RETURN_KW:    name = "ReturnKw";    break;
  case TOKEN_KIND_STRUCT_KW:    name = "StructKw";    break;
  case TOKEN_KIND_TRUE_KW:      name = "TrueKw";      break;
  case TOKEN_KIND_TRY_KW:       name = "TryKw";       break;
  case TOKEN_KIND_TYPE_KW:      name = "TypeKw";      break;
  case TOKEN_KIND_VAR_KW:       name = "VarKw";       break;
  case TOKEN_KIND_WHILE_KW:     name = "WhileKw";     break;
  case TOKEN_KIND_IDENT:        name = "Ident";       break;
  }
  assert(name);
  return name;
}

void lexer_init(Lexer *lex, char *file, char *source)
{
  lex->file = file;
  lex->source = source;
  lex->curr = source;
  lex->ln = 1;
  lex->col = 1;
  lexer_next(lex);
}

void lexer_next(Lexer *lex)
{
  while (skip_space(lex) || skip_comment(lex));
  if (match_char(lex, 0, TOKEN_KIND_EOF)) return;
  if (match_char(lex, ',', TOKEN_KIND_COMMA)) return;
  if (match_char(lex, ';', TOKEN_KIND_SEMICOLON)) return;
  if (match_char(lex, ':', TOKEN_KIND_COLON)) return;
  if (match_char(lex, '(', TOKEN_KIND_LPAREN)) return;
  if (match_char(lex, ')', TOKEN_KIND_RPAREN)) return;
  if (match_char(lex, '[', TOKEN_KIND_LBRACKET)) return;
  if (match_char(lex, ']', TOKEN_KIND_RBRACKET)) return;
  if (match_char(lex, '{', TOKEN_KIND_LBRACE)) return;
  if (match_char(lex, '}', TOKEN_KIND_RBRACE)) return;
  if (match_char(lex, '?', TOKEN_KIND_QMARK)) return;
  if (match_chars(lex, "|=", TOKEN_KIND_PIPEEQ)) return;
  if (match_chars(lex, "||", TOKEN_KIND_PIPEPIPE)) return;
  if (match_char(lex, '|', TOKEN_KIND_PIPE)) return;
  if (match_chars(lex, "&=", TOKEN_KIND_AMPEQ)) return;
  if (match_chars(lex, "&&", TOKEN_KIND_AMPAMP)) return;
  if (match_char(lex, '&', TOKEN_KIND_AMP)) return;
  if (match_chars(lex, "^=", TOKEN_KIND_CARETEQ)) return;
  if (match_char(lex, '^', TOKEN_KIND_CARET)) return;
  if (match_chars(lex, "==", TOKEN_KIND_EQEQ)) return;
  if (match_char(lex, '=', TOKEN_KIND_EQ)) return;
  if (match_chars(lex, "!=", TOKEN_KIND_BANGEQ)) return;
  if (match_char(lex, '!', TOKEN_KIND_BANG)) return;
  if (match_char(lex, '~', TOKEN_KIND_TILDE)) return;
  if (match_chars(lex, "<=", TOKEN_KIND_LE)) return;
  if (match_chars(lex, "<<=", TOKEN_KIND_LTLTEQ)) return;
  if (match_chars(lex, "<<", TOKEN_KIND_LTLT)) return;
  if (match_char(lex, '<', TOKEN_KIND_LT)) return;
  if (match_chars(lex, ">=", TOKEN_KIND_GE)) return;
  if (match_chars(lex, ">>=", TOKEN_KIND_GTGTEQ)) return;
  if (match_chars(lex, ">>", TOKEN_KIND_GTGT)) return;
  if (match_char(lex, '>', TOKEN_KIND_GT)) return;
  if (match_chars(lex, "..", TOKEN_KIND_DOTDOT)) return;
  if (match_char(lex, '.', TOKEN_KIND_DOT)) return;
  if (match_chars(lex, "+=", TOKEN_KIND_PLUSEQ)) return;
  if (match_char(lex, '+', TOKEN_KIND_PLUS)) return;
  if (match_chars(lex, "->", TOKEN_KIND_ARROW)) return;
  if (match_chars(lex, "-=", TOKEN_KIND_MINUSEQ)) return;
  if (match_char(lex, '-', TOKEN_KIND_MINUS)) return;
  if (match_chars(lex, "*=", TOKEN_KIND_STAREQ)) return;
  if (match_char(lex, '*', TOKEN_KIND_STAR)) return;
  if (match_chars(lex, "/=", TOKEN_KIND_SLASHEQ)) return;
  if (match_char(lex, '/', TOKEN_KIND_SLASH)) return;
  if (match_chars(lex, "%=", TOKEN_KIND_PERCENTEQ)) return;
  if (match_char(lex, '%', TOKEN_KIND_PERCENT)) return;
  if (match_number(lex)) return;
  if (match_rune(lex)) return;
  if (match_string(lex)) return;
  if (match_keyword(lex, "as", TOKEN_KIND_AS_KW)) return;
  if (match_keyword(lex, "break", TOKEN_KIND_BREAK_KW)) return;
  if (match_keyword(lex, "continue", TOKEN_KIND_CONTINUE_KW)) return;
  if (match_keyword(lex, "do", TOKEN_KIND_DO_KW)) return;
  if (match_keyword(lex, "else", TOKEN_KIND_ELSE_KW)) return;
  if (match_keyword(lex, "false", TOKEN_KIND_FALSE_KW)) return;
  if (match_keyword(lex, "fn", TOKEN_KIND_FN_KW)) return;
  if (match_keyword(lex, "for", TOKEN_KIND_FOR_KW)) return;
  if (match_keyword(lex, "if", TOKEN_KIND_IF_KW)) return;
  if (match_keyword(lex, "import", TOKEN_KIND_IMPORT_KW)) return;
  if (match_keyword(lex, "in", TOKEN_KIND_IN_KW)) return;
  if (match_keyword(lex, "inout", TOKEN_KIND_INOUT_KW)) return;
  if (match_keyword(lex, "interface", TOKEN_KIND_INTERFACE_KW)) return;
  if (match_keyword(lex, "let", TOKEN_KIND_LET_KW)) return;
  if (match_keyword(lex, "loop", TOKEN_KIND_LOOP_KW)) return;
  if (match_keyword(lex, "new", TOKEN_KIND_NEW_KW)) return;
  if (match_keyword(lex, "null", TOKEN_KIND_NULL_KW)) return;
  if (match_keyword(lex, "return", TOKEN_KIND_RETURN_KW)) return;
  if (match_keyword(lex, "struct", TOKEN_KIND_STRUCT_KW)) return;
  if (match_keyword(lex, "true", TOKEN_KIND_TRUE_KW)) return;
  if (match_keyword(lex, "try", TOKEN_KIND_TRY_KW)) return;
  if (match_keyword(lex, "type", TOKEN_KIND_TYPE_KW)) return;
  if (match_keyword(lex, "var", TOKEN_KIND_VAR_KW)) return;
  if (match_keyword(lex, "while", TOKEN_KIND_WHILE_KW)) return;
  if (match_ident(lex)) return;
  char c = current_char(lex);
  c = isprint(c) ? c : '?';
  lexical_error(lex, "unexpected character '%c' found", c);
}
