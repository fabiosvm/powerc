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
static inline AstNode *parse_module(Parser *parser);
static inline AstNode *parse_fn_decl(Parser *parser);
static inline AstNode *parse_param(Parser *parser);
static inline AstNode *parse_type(Parser *parser);
static inline AstNode *parse_stmt(Parser *parser);
static inline AstNode *parse_var_decl(Parser *parser);
static inline AstNode *parse_return_stmt(Parser *parser);
static inline AstNode *parse_expr(Parser *parser);
static inline AstNode *parse_or_expr(Parser *parser);
static inline AstNode *parse_and_expr(Parser *parser);
static inline AstNode *parse_eq_expr(Parser *parser);
static inline AstNode *parse_comp_expr(Parser *parser);
static inline AstNode *parse_add_expr(Parser *parser);
static inline AstNode *parse_mul_expr(Parser *parser);
static inline AstNode *parse_unary_expr(Parser *parser);
static inline AstNode *parse_call_expr(Parser *parser);
static inline AstNode *parse_prim_expr(Parser *parser);

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

static inline AstNode *parse_module(Parser *parser)
{
  AstNonLeafNode *module = ast_nonleaf_node_new(AST_NODE_KIND_MODULE);
  while (!match(parser, TOKEN_KIND_EOF))
  {
    AstNode *decl = parse_fn_decl(parser);
    ast_nonleaf_node_append_child(module, decl);
  }
  return (AstNode *) module;
}

static inline AstNode *parse_fn_decl(Parser *parser)
{
  consume(parser, TOKEN_KIND_FN_KW);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  consume(parser, TOKEN_KIND_LPAREN);
  AstNonLeafNode *params = ast_nonleaf_node_new(AST_NODE_KIND_PARAMS);
  if (match(parser, TOKEN_KIND_RPAREN))
  {
    next(parser);
    goto end;
  }
  AstNode *param = parse_param(parser);
  ast_nonleaf_node_append_child(params, param);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    param = parse_param(parser);
    ast_nonleaf_node_append_child(params, param);
  }
  consume(parser, TOKEN_KIND_RPAREN);
  AstNode *type;
end:
  type = parse_type(parser);
  consume(parser, TOKEN_KIND_LBRACE);
  AstNonLeafNode *block = ast_nonleaf_node_new(AST_NODE_KIND_BLOCK);
  while (!match(parser, TOKEN_KIND_RBRACE))
  {
    AstNode *stmt = parse_stmt(parser);
    ast_nonleaf_node_append_child(block, stmt);
  }
  next(parser);
  AstNonLeafNode *func = ast_nonleaf_node_new(AST_NODE_KIND_FUNC);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  ast_nonleaf_node_append_child(func, ident);
  ast_nonleaf_node_append_child(func, (AstNode *) params);
  ast_nonleaf_node_append_child(func, type);
  ast_nonleaf_node_append_child(func, (AstNode *) block);
  return (AstNode *) func;
}

static inline AstNode *parse_param(Parser *parser)
{
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNode *type = parse_type(parser);
  AstNonLeafNode *param = ast_nonleaf_node_new(AST_NODE_KIND_VAR_DECL);
  ast_nonleaf_node_append_child(param, ident);
  ast_nonleaf_node_append_child(param, type);
  return (AstNode *) param;
}

static inline AstNode *parse_type(Parser *parser)
{
  if (match(parser, TOKEN_KIND_VOID_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_VOID_TYPE, token);
  }
  if (match(parser, TOKEN_KIND_BOOL_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_BOOL_TYPE, token);
  }
  if (match(parser, TOKEN_KIND_INT_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_INT_TYPE, token);
  }
  if (match(parser, TOKEN_KIND_UINT_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_UINT_TYPE, token);
  }
  if (match(parser, TOKEN_KIND_FLOAT_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_FLOAT_TYPE, token);
  }
  unexpected_token_error(parser);
  return NULL;
}

static inline AstNode *parse_stmt(Parser *parser)
{
  if (match(parser, TOKEN_KIND_VAR_KW))
    return parse_var_decl(parser);
  if (match(parser, TOKEN_KIND_RETURN_KW))
    return parse_return_stmt(parser);
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  return expr;
}

static inline AstNode *parse_var_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *type = parse_type(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNonLeafNode *var = ast_nonleaf_node_new(AST_NODE_KIND_VAR_DECL);
  ast_nonleaf_node_append_child(var, ident);
  ast_nonleaf_node_append_child(var, type);
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    AstNode *expr = parse_expr(parser);
    consume(parser, TOKEN_KIND_SEMICOLON);
    AstNonLeafNode *assign = ast_nonleaf_node_new(AST_NODE_KIND_ASSIGN);
    ast_nonleaf_node_append_child(assign, (AstNode *) var);
    ast_nonleaf_node_append_child(assign, expr);
    return (AstNode *) assign;
  }
  consume(parser, TOKEN_KIND_SEMICOLON);
  return (AstNode *) var;
}

static inline AstNode *parse_return_stmt(Parser *parser)
{
  next(parser);
  AstNonLeafNode *ret = ast_nonleaf_node_new(AST_NODE_KIND_RETURN);
  if (match(parser, TOKEN_KIND_SEMICOLON))
  {
    next(parser);
    return (AstNode *) ret;
  }
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  ast_nonleaf_node_append_child(ret, expr);
  return (AstNode *) ret;
}

static inline AstNode *parse_expr(Parser *parser)
{
  AstNode *lhs = parse_or_expr(parser);
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    AstNode *rhs = parse_expr(parser);
    AstNonLeafNode *assign = ast_nonleaf_node_new(AST_NODE_KIND_ASSIGN);
    ast_nonleaf_node_append_child(assign, lhs);
    ast_nonleaf_node_append_child(assign, rhs);
    lhs = (AstNode *) assign;
  }
  return lhs;
}

static inline AstNode *parse_or_expr(Parser *parser)
{
  AstNode *lhs = parse_and_expr(parser);
  while (match(parser, TOKEN_KIND_PIPEPIPE))
  {
    next(parser);
    AstNode *rhs = parse_and_expr(parser);
    AstNonLeafNode *or = ast_nonleaf_node_new(AST_NODE_KIND_OR);
    ast_nonleaf_node_append_child(or, lhs);
    ast_nonleaf_node_append_child(or, rhs);
    lhs = (AstNode *) or;
  }
  return lhs;
}

static inline AstNode *parse_and_expr(Parser *parser)
{
  AstNode *lhs = parse_eq_expr(parser);
  while (match(parser, TOKEN_KIND_AMPAMP))
  {
    next(parser);
    AstNode *rhs = parse_eq_expr(parser);
    AstNonLeafNode *and = ast_nonleaf_node_new(AST_NODE_KIND_AND);
    ast_nonleaf_node_append_child(and, lhs);
    ast_nonleaf_node_append_child(and, rhs);
    lhs = (AstNode *) and;
  }
  return lhs;
}

static inline AstNode *parse_eq_expr(Parser *parser)
{
  AstNode *lhs = parse_comp_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_EQEQ))
    {
      next(parser);
      AstNode *rhs = parse_comp_expr(parser);
      AstNonLeafNode *eq = ast_nonleaf_node_new(AST_NODE_KIND_EQ);
      ast_nonleaf_node_append_child(eq, lhs);
      ast_nonleaf_node_append_child(eq, rhs);
      lhs = (AstNode *) eq;
      continue;
    }
    if (match(parser, TOKEN_KIND_BANGEQ))
    {
      next(parser);
      AstNode *rhs = parse_comp_expr(parser);
      AstNonLeafNode *ne = ast_nonleaf_node_new(AST_NODE_KIND_NE);
      ast_nonleaf_node_append_child(ne, lhs);
      ast_nonleaf_node_append_child(ne, rhs);
      lhs = (AstNode *) ne;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_comp_expr(Parser *parser)
{
  AstNode *lhs = parse_add_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_LT))
    {
      next(parser);
      AstNode *rhs = parse_add_expr(parser);
      AstNonLeafNode *lt = ast_nonleaf_node_new(AST_NODE_KIND_LT);
      ast_nonleaf_node_append_child(lt, lhs);
      ast_nonleaf_node_append_child(lt, rhs);
      lhs = (AstNode *) lt;
      continue;
    }
    if (match(parser, TOKEN_KIND_LE))
    {
      next(parser);
      AstNode *rhs = parse_add_expr(parser);
      AstNonLeafNode *le = ast_nonleaf_node_new(AST_NODE_KIND_LE);
      ast_nonleaf_node_append_child(le, lhs);
      ast_nonleaf_node_append_child(le, rhs);
      lhs = (AstNode *) le;
      continue;
    }
    if (match(parser, TOKEN_KIND_GT))
    {
      next(parser);
      AstNode *rhs = parse_add_expr(parser);
      AstNonLeafNode *gt = ast_nonleaf_node_new(AST_NODE_KIND_GT);
      ast_nonleaf_node_append_child(gt, lhs);
      ast_nonleaf_node_append_child(gt, rhs);
      lhs = (AstNode *) gt;
      continue;
    }
    if (match(parser, TOKEN_KIND_GE))
    {
      next(parser);
      AstNode *rhs = parse_add_expr(parser);
      AstNonLeafNode *ge = ast_nonleaf_node_new(AST_NODE_KIND_GE);
      ast_nonleaf_node_append_child(ge, lhs);
      ast_nonleaf_node_append_child(ge, rhs);
      lhs = (AstNode *) ge;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_add_expr(Parser *parser)
{
  AstNode *lhs = parse_mul_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_PLUS))
    {
      next(parser);
      AstNode *rhs = parse_mul_expr(parser);
      AstNonLeafNode *add = ast_nonleaf_node_new(AST_NODE_KIND_ADD);
      ast_nonleaf_node_append_child(add, lhs);
      ast_nonleaf_node_append_child(add, rhs);
      lhs = (AstNode *) add;
      continue;
    }
    if (match(parser, TOKEN_KIND_MINUS))
    {
      next(parser);
      AstNode *rhs = parse_mul_expr(parser);
      AstNonLeafNode *sub = ast_nonleaf_node_new(AST_NODE_KIND_SUB);
      ast_nonleaf_node_append_child(sub, lhs);
      ast_nonleaf_node_append_child(sub, rhs);
      lhs = (AstNode *) sub;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_mul_expr(Parser *parser)
{
  AstNode *lhs = parse_unary_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_STAR))
    {
      next(parser);
      AstNode *rhs = parse_unary_expr(parser);
      AstNonLeafNode *mul = ast_nonleaf_node_new(AST_NODE_KIND_MUL);
      ast_nonleaf_node_append_child(mul, lhs);
      ast_nonleaf_node_append_child(mul, rhs);
      lhs = (AstNode *) mul;
      continue;
    }
    if (match(parser, TOKEN_KIND_SLASH))
    {
      next(parser);
      AstNode *rhs = parse_unary_expr(parser);
      AstNonLeafNode *div = ast_nonleaf_node_new(AST_NODE_KIND_DIV);
      ast_nonleaf_node_append_child(div, lhs);
      ast_nonleaf_node_append_child(div, rhs);
      lhs = (AstNode *) div;
      continue;
    }
    if (match(parser, TOKEN_KIND_PERCENT))
    {
      next(parser);
      AstNode *rhs = parse_unary_expr(parser);
      AstNonLeafNode *mod = ast_nonleaf_node_new(AST_NODE_KIND_MOD);
      ast_nonleaf_node_append_child(mod, lhs);
      ast_nonleaf_node_append_child(mod, rhs);
      lhs = (AstNode *) mod;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_unary_expr(Parser *parser)
{
  if (match(parser, TOKEN_KIND_BANG))
  {
    next(parser);
    AstNode *expr = parse_unary_expr(parser);
    AstNonLeafNode *not = ast_nonleaf_node_new(AST_NODE_KIND_NOT);
    ast_nonleaf_node_append_child(not, expr);
    return (AstNode *) not;
  }
  if (match(parser, TOKEN_KIND_MINUS))
  {
    next(parser);
    AstNode *expr = parse_unary_expr(parser);
    AstNonLeafNode *neg = ast_nonleaf_node_new(AST_NODE_KIND_NEG);
    ast_nonleaf_node_append_child(neg, expr);
    return (AstNode *) neg;
  }
  return parse_call_expr(parser);
}

static inline AstNode *parse_call_expr(Parser *parser)
{
  AstNode *lhs = parse_prim_expr(parser);
  while (match(parser, TOKEN_KIND_LPAREN))
  {
    next(parser);
    AstNonLeafNode *call = ast_nonleaf_node_new(AST_NODE_KIND_CALL);
    if (match(parser, TOKEN_KIND_RPAREN))
    {
      next(parser);
      continue;
    }
    AstNode *arg = parse_expr(parser);
    ast_nonleaf_node_append_child(call, arg);
    while (match(parser, TOKEN_KIND_COMMA))
    {
      next(parser);
      arg = parse_expr(parser);
      ast_nonleaf_node_append_child(call, arg);
    }
    consume(parser, TOKEN_KIND_RPAREN);
    ast_nonleaf_node_append_child(call, lhs);
    lhs = (AstNode *) call;
  }
  return lhs;
}

static inline AstNode *parse_prim_expr(Parser *parser)
{
  if (match(parser, TOKEN_KIND_FALSE_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_FALSE, token);
  }
  if (match(parser, TOKEN_KIND_TRUE_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_TRUE, token);
  }
  if (match(parser, TOKEN_KIND_INT))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_INT, token);
  }
  if (match(parser, TOKEN_KIND_FLOAT))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_FLOAT, token);
  }
  if (match(parser, TOKEN_KIND_IDENT))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  }
  if (match(parser, TOKEN_KIND_LPAREN))
  {
    next(parser);
    AstNode *expr = parse_expr(parser);
    consume(parser, TOKEN_KIND_RPAREN);
    return expr;
  }
  unexpected_token_error(parser);
  return NULL;
}

void parser_init(Parser *parser, char *file, char *source)
{
  lexer_init(&parser->lex, file, source);
}

AstNode *parser_parse(Parser *parser)
{
  return parse_module(parser);
}
