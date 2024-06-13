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
  case TOKEN_KIND_EOF:       name = "Eof";       break;
  case TOKEN_KIND_COMMA:     name = "Comma";     break;
  case TOKEN_KIND_COLON:     name = "Colon";     break;
  case TOKEN_KIND_SEMICOLON: name = "Semicolon"; break;
  case TOKEN_KIND_LPAREN:    name = "LParen";    break;
  case TOKEN_KIND_RPAREN:    name = "RParen";    break;
  case TOKEN_KIND_LBRACE:    name = "LBrace";    break;
  case TOKEN_KIND_RBRACE:    name = "RBrace";    break;
  case TOKEN_KIND_PIPEPIPE:  name = "PipePipe";  break;
  case TOKEN_KIND_AMPAMP:    name = "AmpAmp";    break;
  case TOKEN_KIND_AMP:       name = "Amp";       break;
  case TOKEN_KIND_EQEQ:      name = "EqEq";      break;
  case TOKEN_KIND_EQ:        name = "Eq";        break;
  case TOKEN_KIND_BANGEQ:    name = "BangEq";    break;
  case TOKEN_KIND_BANG:      name = "Bang";      break;
  case TOKEN_KIND_LE:        name = "Le";        break;
  case TOKEN_KIND_LT:        name = "Lt";        break;
  case TOKEN_KIND_GE:        name = "Ge";        break;
  case TOKEN_KIND_GT:        name = "Gt";        break;
  case TOKEN_KIND_PLUS:      name = "Plus";      break;
  case TOKEN_KIND_ARROW:     name = "Arrow";     break;
  case TOKEN_KIND_MINUS:     name = "Minus";     break;
  case TOKEN_KIND_STAR:      name = "Star";      break;
  case TOKEN_KIND_SLASH:     name = "Slash";     break;
  case TOKEN_KIND_PERCENT:   name = "Percent";   break;
  case TOKEN_KIND_INT:       name = "Int";       break;
  case TOKEN_KIND_FLOAT:     name = "Float";     break;
  case TOKEN_KIND_RUNE:      name = "Rune";      break;
  case TOKEN_KIND_STRING:    name = "String";    break;
  case TOKEN_KIND_BOOL_KW:   name = "BoolKw";    break;
  case TOKEN_KIND_BYTE_KW:   name = "ByteKw";    break;
  case TOKEN_KIND_ELSE_KW:   name = "ElseKw";    break;
  case TOKEN_KIND_FALSE_KW:  name = "FalseKw";   break;
  case TOKEN_KIND_FLOAT_KW:  name = "FloatKw";   break;
  case TOKEN_KIND_FN_KW:     name = "FnKw";      break;
  case TOKEN_KIND_IF_KW:     name = "IfKw";      break;
  case TOKEN_KIND_INOUT_KW:  name = "InoutKw";   break;
  case TOKEN_KIND_INT_KW:    name = "IntKw";     break;
  case TOKEN_KIND_RETURN_KW: name = "ReturnKw";  break;
  case TOKEN_KIND_RUNE_KW:   name = "RuneKw";    break;
  case TOKEN_KIND_STRING_KW: name = "StringKw";  break;
  case TOKEN_KIND_TRUE_KW:   name = "TrueKw";    break;
  case TOKEN_KIND_UINT_KW:   name = "UIntKw";    break;
  case TOKEN_KIND_VAR_KW:    name = "VarKw";     break;
  case TOKEN_KIND_IDENT:     name = "Ident";     break;
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
  if (match_char(lex, '{', TOKEN_KIND_LBRACE)) return;
  if (match_char(lex, '}', TOKEN_KIND_RBRACE)) return;
  if (match_chars(lex, "||", TOKEN_KIND_PIPEPIPE)) return;
  if (match_chars(lex, "&&", TOKEN_KIND_AMPAMP)) return;
  if (match_char(lex, '&', TOKEN_KIND_AMP)) return;
  if (match_chars(lex, "==", TOKEN_KIND_EQEQ)) return;
  if (match_char(lex, '=', TOKEN_KIND_EQ)) return;
  if (match_chars(lex, "!=", TOKEN_KIND_BANGEQ)) return;
  if (match_char(lex, '!', TOKEN_KIND_BANG)) return;
  if (match_chars(lex, "<=", TOKEN_KIND_LE)) return;
  if (match_char(lex, '<', TOKEN_KIND_LT)) return;
  if (match_chars(lex, ">=", TOKEN_KIND_GE)) return;
  if (match_char(lex, '>', TOKEN_KIND_GT)) return;
  if (match_char(lex, '+', TOKEN_KIND_PLUS)) return;
  if (match_chars(lex, "->", TOKEN_KIND_ARROW)) return;
  if (match_char(lex, '-', TOKEN_KIND_MINUS)) return;
  if (match_char(lex, '*', TOKEN_KIND_STAR)) return;
  if (match_char(lex, '/', TOKEN_KIND_SLASH)) return;
  if (match_char(lex, '%', TOKEN_KIND_PERCENT)) return;
  if (match_number(lex)) return;
  if (match_rune(lex)) return;
  if (match_string(lex)) return;
  if (match_keyword(lex, "Bool", TOKEN_KIND_BOOL_KW)) return;
  if (match_keyword(lex, "Byte", TOKEN_KIND_BYTE_KW)) return;
  if (match_keyword(lex, "else", TOKEN_KIND_ELSE_KW)) return;
  if (match_keyword(lex, "false", TOKEN_KIND_FALSE_KW)) return;
  if (match_keyword(lex, "Float", TOKEN_KIND_FLOAT_KW)) return;
  if (match_keyword(lex, "fn", TOKEN_KIND_FN_KW)) return;
  if (match_keyword(lex, "if", TOKEN_KIND_IF_KW)) return;
  if (match_keyword(lex, "inout", TOKEN_KIND_INOUT_KW)) return;
  if (match_keyword(lex, "Int", TOKEN_KIND_INT_KW)) return;
  if (match_keyword(lex, "return", TOKEN_KIND_RETURN_KW)) return;
  if (match_keyword(lex, "Rune", TOKEN_KIND_RUNE_KW)) return;
  if (match_keyword(lex, "String", TOKEN_KIND_STRING_KW)) return;
  if (match_keyword(lex, "true", TOKEN_KIND_TRUE_KW)) return;
  if (match_keyword(lex, "UInt", TOKEN_KIND_UINT_KW)) return;
  if (match_keyword(lex, "var", TOKEN_KIND_VAR_KW)) return;
  if (match_ident(lex)) return;
  char c = current_char(lex);
  c = isprint(c) ? c : '?';
  lexical_error(lex, "unexpected character '%c' found", c);
}
