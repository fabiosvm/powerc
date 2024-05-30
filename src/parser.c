//
// parser.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

#define current(p) ((p)->lex.token)

#define match(p, t) (current(p).kind == (t))

#define next(p) \
  do { \
    lexer_next(&(p)->lex); \
  } while (0)

#define consume(p, t) \
  do { \
    if (!match((p), (t))) \
      unexpected_token_error(p); \
    next(p); \
  } while (0)

static inline void unexpected_token_error(Parser *parser);
static inline void parse_module(Parser *parser);
static inline void parse_fn_decl(Parser *parser);
static inline void parse_param(Parser *parser);
static inline void parse_type(Parser *parser);
static inline void parse_stmt(Parser *parser);
static inline void parse_var_decl(Parser *parser);
static inline void parse_return_stmt(Parser *parser);
static inline void parse_expr(Parser *parser);
static inline void parse_or_expr(Parser *parser);
static inline void parse_and_expr(Parser *parser);
static inline void parse_eq_expr(Parser *parser);
static inline void parse_comp_expr(Parser *parser);
static inline void parse_add_expr(Parser *parser);
static inline void parse_mul_expr(Parser *parser);
static inline void parse_unary_expr(Parser *parser);
static inline void parse_call_expr(Parser *parser);
static inline void parse_prim_expr(Parser *parser);

static inline void unexpected_token_error(Parser *parser)
{
  Lexer *lex = &parser->lex;
  Token *token = &lex->token;
  if (token->kind == TOKEN_KIND_EOF)
  {
    fprintf(stderr, "\nERROR: unexpected end of file\n");
    goto end;
  }
  fprintf(stderr, "\nERROR: unexpected token '%.*s'\n", token->length, token->chars);
end:
  fprintf(stderr, "--> %s:%d:%d\n", lex->file, token->ln, token->col);
  exit(EXIT_FAILURE);
}

static inline void parse_module(Parser *parser)
{
  while (!match(parser, TOKEN_KIND_EOF))
    parse_fn_decl(parser);
}

static inline void parse_fn_decl(Parser *parser)
{
  consume(parser, TOKEN_KIND_FN_KW);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  (void) token;
  consume(parser, TOKEN_KIND_LPAREN);
  if (match(parser, TOKEN_KIND_RPAREN))
  {
    next(parser);
    goto end;
  }
  parse_param(parser);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    parse_param(parser);
  }
  consume(parser, TOKEN_KIND_RPAREN);
end:
  parse_type(parser);
  consume(parser, TOKEN_KIND_LBRACE);
  while (!match(parser, TOKEN_KIND_RBRACE))
    parse_stmt(parser);
  next(parser);
}

static inline void parse_param(Parser *parser)
{
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  (void) token;
  parse_type(parser);
}

static inline void parse_type(Parser *parser)
{
  if (match(parser, TOKEN_KIND_VOID_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_BOOL_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_INT_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_UINT_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_FLOAT_KW))
  {
    next(parser);
    return;
  }
  unexpected_token_error(parser);
}

static inline void parse_stmt(Parser *parser)
{
  if (match(parser, TOKEN_KIND_VAR_KW))
  {
    parse_var_decl(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_RETURN_KW))
  {
    parse_return_stmt(parser);
    return;
  }
  parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
}

static inline void parse_var_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  (void) token;
  parse_type(parser);
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    parse_expr(parser);
  }
  consume(parser, TOKEN_KIND_SEMICOLON);
}

static inline void parse_return_stmt(Parser *parser)
{
  next(parser);
  if (match(parser, TOKEN_KIND_SEMICOLON))
  {
    next(parser);
    return;
  }
  parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
}

static inline void parse_expr(Parser *parser)
{
  parse_or_expr(parser);
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    parse_expr(parser);
  }
}

static inline void parse_or_expr(Parser *parser)
{
  parse_and_expr(parser);
  while (match(parser, TOKEN_KIND_PIPEPIPE))
  {
    next(parser);
    parse_and_expr(parser);
  }
}

static inline void parse_and_expr(Parser *parser)
{
  parse_eq_expr(parser);
  while (match(parser, TOKEN_KIND_AMPAMP))
  {
    next(parser);
    parse_eq_expr(parser);
  }
}

static inline void parse_eq_expr(Parser *parser)
{
  parse_comp_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_EQEQ))
    {
      next(parser);
      parse_comp_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_BANGEQ))
    {
      next(parser);
      parse_comp_expr(parser);
      continue;
    }
    break;
  }
}

static inline void parse_comp_expr(Parser *parser)
{
  parse_add_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_LT))
    {
      next(parser);
      parse_add_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_LE))
    {
      next(parser);
      parse_add_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_GT))
    {
      next(parser);
      parse_add_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_GE))
    {
      next(parser);
      parse_add_expr(parser);
      continue;
    }
    break;
  }
}

static inline void parse_add_expr(Parser *parser)
{
  parse_mul_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_PLUS))
    {
      next(parser);
      parse_mul_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_MINUS))
    {
      next(parser);
      parse_mul_expr(parser);
      continue;
    }
    break;
  }
}

static inline void parse_mul_expr(Parser *parser)
{
  parse_unary_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_STAR))
    {
      next(parser);
      parse_unary_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_SLASH))
    {
      next(parser);
      parse_unary_expr(parser);
      continue;
    }
    if (match(parser, TOKEN_KIND_PERCENT))
    {
      next(parser);
      parse_unary_expr(parser);
      continue;
    }
    break;
  }
}

static inline void parse_unary_expr(Parser *parser)
{
  if (match(parser, TOKEN_KIND_BANG))
  {
    next(parser);
    parse_unary_expr(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_MINUS))
  {
    next(parser);
    parse_unary_expr(parser);
    return;
  }
  parse_call_expr(parser);
}

static inline void parse_call_expr(Parser *parser)
{
  parse_prim_expr(parser);
  while (match(parser, TOKEN_KIND_LPAREN))
  {
    next(parser);
    if (match(parser, TOKEN_KIND_RPAREN))
    {
      next(parser);
      continue;
    }
    parse_expr(parser);
    while (match(parser, TOKEN_KIND_COMMA))
    {
      next(parser);
      parse_expr(parser);
    }
    consume(parser, TOKEN_KIND_RPAREN);
  }
}

static inline void parse_prim_expr(Parser *parser)
{
  if (match(parser, TOKEN_KIND_FALSE_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_TRUE_KW))
  {
    next(parser);
    return;
  }
  if (match(parser, TOKEN_KIND_INT))
  {
    Token token = current(parser);
    next(parser);
    (void) token;
    return;
  }
  if (match(parser, TOKEN_KIND_FLOAT))
  {
    Token token = current(parser);
    next(parser);
    (void) token;
    return;
  }
  if (match(parser, TOKEN_KIND_IDENT))
  {
    Token token = current(parser);
    next(parser);
    (void) token;
    return;
  }
  if (match(parser, TOKEN_KIND_LPAREN))
  {
    next(parser);
    parse_expr(parser);
    consume(parser, TOKEN_KIND_RPAREN);
    return;
  }
  unexpected_token_error(parser);
}

void parser_init(Parser *parser, char *file, char *source)
{
  lexer_init(&parser->lex, file, source);
}

void parser_parse(Parser *parser)
{
  parse_module(parser);
}
