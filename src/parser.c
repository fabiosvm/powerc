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
#include <stdbool.h>
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
static inline AstNode *parse_decl(Parser *parser);
static inline AstNode *parse_import_decl(Parser *parser);
static inline AstNode *parse_type_decl(Parser *parser);
static inline AstNode *parse_type_params(Parser *parser);
static inline AstNode *parse_type_param(Parser *parser);
static inline AstNode *parse_type(Parser *parser);
static inline AstNode *parse_prim_type(Parser *parser);
static inline AstNode *parse_func_type(Parser *parser);
static inline AstNode *parse_param_type(Parser *parser);
static inline AstNode *parse_type_def(Parser *parser);
static inline AstNode *parse_func_decl(Parser *parser, bool isAnon);
static inline AstNode *parse_param(Parser *parser);
static inline AstNode *parse_block(Parser *parser);
static inline AstNode *parse_struct_decl(Parser *parser);
static inline AstNode *parse_field(Parser *parser);
static inline AstNode *parse_interface_decl(Parser *parser);
static inline AstNode *parse_method_proto(Parser *parser);
static inline AstNode *parse_let_decl(Parser *parser);
static inline AstNode *parse_var_decl(Parser *parser);
static inline AstNode *parse_stmt(Parser *parser);
static inline AstNode *parse_if_stmt(Parser *parser);
static inline AstNode *parse_loop_stmt(Parser *parser);
static inline AstNode *parse_while_stmt(Parser *parser);
static inline AstNode *parse_do_while_stmt(Parser *parser);
static inline AstNode *parse_for_stmt(Parser *parser);
static inline AstNode *parse_break_stmt(Parser *parser);
static inline AstNode *parse_continue_stmt(Parser *parser);
static inline AstNode *parse_return_stmt(Parser *parser);
static inline AstNode *parse_expr(Parser *parser);
static inline AstNode *parse_or_expr(Parser *parser);
static inline AstNode *parse_and_expr(Parser *parser);
static inline AstNode *parse_bor_expr(Parser *parser);
static inline AstNode *parse_bxor_expr(Parser *parser);
static inline AstNode *parse_band_expr(Parser *parser);
static inline AstNode *parse_eq_expr(Parser *parser);
static inline AstNode *parse_comp_expr(Parser *parser);
static inline AstNode *parse_shift_expr(Parser *parser);
static inline AstNode *parse_range_expr(Parser *parser);
static inline AstNode *parse_add_expr(Parser *parser);
static inline AstNode *parse_mul_expr(Parser *parser);
static inline AstNode *parse_unary_expr(Parser *parser);
static inline AstNode *parse_prim_expr(Parser *parser);
static inline AstNode *parse_array_expr(Parser *parser);
static inline AstNode *parse_new_expr(Parser *parser);
static inline AstNode *parse_ref_expr(Parser *parser);
static inline AstNode *parse_ident_expr(Parser *parser);
static inline AstNode *parse_try_expr(Parser *parser);
static inline AstNode *parse_call(Parser *parser, AstNode *lhs);
static inline AstNode *parse_subscr(Parser *parser, AstNode *lhs);
static inline AstNode *parse_if_expr(Parser *parser);

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
    AstNode *decl = parse_decl(parser);
    ast_nonleaf_node_append_child(module, decl);
  }
  return (AstNode *) module;
}

static inline AstNode *parse_decl(Parser *parser)
{
  if (match(parser, TOKEN_KIND_IMPORT_KW))
    return parse_import_decl(parser);
  if (match(parser, TOKEN_KIND_TYPE_KW))
    return parse_type_decl(parser);
  if (match(parser, TOKEN_KIND_FN_KW))
    return parse_func_decl(parser, false);
  if (match(parser, TOKEN_KIND_STRUCT_KW))
    return parse_struct_decl(parser);
  if (match(parser, TOKEN_KIND_INTERFACE_KW))
    return parse_interface_decl(parser);
  if (match(parser, TOKEN_KIND_LET_KW))
    return parse_let_decl(parser);
  unexpected_token_error(parser);
  return NULL;
}

static inline AstNode *parse_import_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_STRING))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNonLeafNode *importDecl = ast_nonleaf_node_new(AST_NODE_KIND_IMPORT_DECL);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  ast_nonleaf_node_append_child(importDecl, ident);
  if (!match(parser, TOKEN_KIND_AS_KW))
  {
    consume(parser, TOKEN_KIND_SEMICOLON);
    return (AstNode *) importDecl;
  }
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  token = current(parser);
  next(parser);
  ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNonLeafNode *rename = ast_nonleaf_node_new(AST_NODE_KIND_RENAME);
  ast_nonleaf_node_append_child(rename, (AstNode *) importDecl);
  ast_nonleaf_node_append_child(rename, ident);
  consume(parser, TOKEN_KIND_SEMICOLON);
  return (AstNode *) rename;
}

static inline AstNode *parse_type_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNonLeafNode *typeDecl = ast_nonleaf_node_new(AST_NODE_KIND_TYPE_DECL);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNode *typeParams = parse_type_params(parser);
  ast_nonleaf_node_append_child(typeDecl, ident);
  ast_nonleaf_node_append_child(typeDecl, typeParams);
  consume(parser, TOKEN_KIND_EQ);
  AstNode *type = parse_type(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  ast_nonleaf_node_append_child(typeDecl, type);
  return (AstNode *) typeDecl;
}

static inline AstNode *parse_type_params(Parser *parser)
{
  if (!match(parser, TOKEN_KIND_LT))
    return NULL;
  next(parser);
  AstNonLeafNode *typeParams = ast_nonleaf_node_new(AST_NODE_KIND_TYPE_PARAMS);
  AstNode *typeParam = parse_type_param(parser);
  ast_nonleaf_node_append_child(typeParams, typeParam);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    typeParam = parse_type_param(parser);
    ast_nonleaf_node_append_child(typeParams, typeParam);
  }
  consume(parser, TOKEN_KIND_GT);
  return (AstNode *) typeParams;
}

static inline AstNode *parse_type_param(Parser *parser)
{
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  if (!match(parser, TOKEN_KIND_COLON))
    return ident;
  next(parser);
  AstNode *type = parse_type(parser);
  AstNonLeafNode *constraint = ast_nonleaf_node_new(AST_NODE_KIND_CONSTRAINT);
  ast_nonleaf_node_append_child(constraint, ident);
  ast_nonleaf_node_append_child(constraint, type);
  return (AstNode *) constraint;
}

static inline AstNode *parse_type(Parser *parser)
{
  AstNode *lhs = parse_prim_type(parser);
  while (match(parser, TOKEN_KIND_PLUS))
  {
    next(parser);
    AstNode *rhs = parse_prim_type(parser);
    AstNonLeafNode *type = ast_nonleaf_node_new(AST_NODE_KIND_INTERSECT);
    ast_nonleaf_node_append_child(type, lhs);
    ast_nonleaf_node_append_child(type, rhs);
    lhs = (AstNode *) type;
  }
  return lhs;
}

static inline AstNode *parse_prim_type(Parser *parser)
{
  if (match(parser, TOKEN_KIND_FN_KW))
    return parse_func_type(parser);
  if (match(parser, TOKEN_KIND_IDENT))
    return parse_type_def(parser);
  unexpected_token_error(parser);
  return NULL;
}

static inline AstNode *parse_func_type(Parser *parser)
{
  next(parser);
  AstNode *retType = parse_type(parser);
  consume(parser, TOKEN_KIND_LPAREN);
  AstNonLeafNode *params = ast_nonleaf_node_new(AST_NODE_KIND_PARAMS);
  if (match(parser, TOKEN_KIND_RPAREN))
  {
    next(parser);
    goto end;
  }
  AstNode *param = parse_param_type(parser);
  ast_nonleaf_node_append_child(params, param);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    param = parse_param_type(parser);
    ast_nonleaf_node_append_child(params, param);
  }
  consume(parser, TOKEN_KIND_RPAREN);
AstNonLeafNode *funcType;
end:
  funcType = ast_nonleaf_node_new(AST_NODE_KIND_FUNC_TYPE);
  ast_nonleaf_node_append_child(funcType, retType);
  ast_nonleaf_node_append_child(funcType, (AstNode *) params);
  return (AstNode *) funcType;
}

static inline AstNode *parse_param_type(Parser *parser)
{
  if (match(parser, TOKEN_KIND_INOUT_KW))
  {
    next(parser);
    AstNode *type = parse_type(parser);
    AstNonLeafNode *inout = ast_nonleaf_node_new(AST_NODE_KIND_INOUT_PARAM);
    ast_nonleaf_node_append_child(inout, type);
    return (AstNode *) inout;
  }
  return parse_type(parser);
}

static inline AstNode *parse_type_def(Parser *parser)
{
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  if (!match(parser, TOKEN_KIND_LT))
    return (AstNode *) ident;
  next(parser);
  AstNonLeafNode *typeDef = ast_nonleaf_node_new(AST_NODE_KIND_TYPE);
  ast_nonleaf_node_append_child(typeDef, ident);
  if (match(parser, TOKEN_KIND_GT))
  {
    next(parser);
    return (AstNode *) typeDef;
  }
  AstNode *type = parse_type(parser);
  ast_nonleaf_node_append_child(typeDef, type);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    type = parse_type(parser);
    ast_nonleaf_node_append_child(typeDef, type);
  }
  consume(parser, TOKEN_KIND_GT);
  return (AstNode *) typeDef;
}

static inline AstNode *parse_func_decl(Parser *parser, bool isAnon)
{
  next(parser);
  AstNode *retType = parse_type(parser);
  AstNode *ident = NULL;
  if (!isAnon)
  {
    if (!match(parser, TOKEN_KIND_IDENT))
      unexpected_token_error(parser);
    Token token = current(parser);
    next(parser);
    ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  }
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
end:
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *block = parse_block(parser);
  AstNonLeafNode *funcDecl = ast_nonleaf_node_new(AST_NODE_KIND_FUNC_DECL);
  ast_nonleaf_node_append_child(funcDecl, retType);
  ast_nonleaf_node_append_child(funcDecl, ident);
  ast_nonleaf_node_append_child(funcDecl, (AstNode *) params);
  ast_nonleaf_node_append_child(funcDecl, block);
  return (AstNode *) funcDecl;
}

static inline AstNode *parse_param(Parser *parser)
{
  AstNode *type = parse_param_type(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNonLeafNode *param = ast_nonleaf_node_new(AST_NODE_KIND_VAR_DECL);
  ast_nonleaf_node_append_child(param, type);
  ast_nonleaf_node_append_child(param, ident);
  return (AstNode *) param;
}

static inline AstNode *parse_block(Parser *parser)
{
  next(parser);
  AstNonLeafNode *block = ast_nonleaf_node_new(AST_NODE_KIND_BLOCK);
  while (!match(parser, TOKEN_KIND_RBRACE))
  {
    AstNode *stmt = parse_stmt(parser);
    ast_nonleaf_node_append_child(block, stmt);
  }
  next(parser);
  return (AstNode *) block;
}

static inline AstNode *parse_struct_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNonLeafNode *structDecl = ast_nonleaf_node_new(AST_NODE_KIND_STRUCT_DECL);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNode *typeParams = parse_type_params(parser);
  ast_nonleaf_node_append_child(structDecl, ident);
  ast_nonleaf_node_append_child(structDecl, typeParams);
  consume(parser, TOKEN_KIND_LBRACE);
  if (match(parser, TOKEN_KIND_RBRACE))
  {
    next(parser);
    return (AstNode *) structDecl;
  }
  AstNode *field = parse_field(parser);
  ast_nonleaf_node_append_child(structDecl, field);
  while (!match(parser, TOKEN_KIND_RBRACE))
  {
    field = parse_field(parser);
    ast_nonleaf_node_append_child(structDecl, field);
  }
  next(parser);
  return (AstNode *) structDecl;
}

static inline AstNode *parse_field(Parser *parser)
{
  AstNode *type = parse_type(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  consume(parser, TOKEN_KIND_SEMICOLON);
  AstNonLeafNode *field = ast_nonleaf_node_new(AST_NODE_KIND_VAR_DECL);
  ast_nonleaf_node_append_child(field, type);
  ast_nonleaf_node_append_child(field, ident);
  return (AstNode *) field;
}

static inline AstNode *parse_interface_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNonLeafNode *interfaceDecl = ast_nonleaf_node_new(AST_NODE_KIND_INTERFACE_DECL);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNode *typeParams = parse_type_params(parser);
  ast_nonleaf_node_append_child(interfaceDecl, ident);
  ast_nonleaf_node_append_child(interfaceDecl, typeParams);
  AstNode *type = NULL;
  if (match(parser, TOKEN_KIND_COLON))
  {
    next(parser);
    type = parse_type(parser);
  }
  ast_nonleaf_node_append_child(interfaceDecl, type);
  consume(parser, TOKEN_KIND_LBRACE);
  if (match(parser, TOKEN_KIND_RBRACE))
  {
    next(parser);
    return (AstNode *) interfaceDecl;
  }
  AstNode *methodProto = parse_method_proto(parser);
  ast_nonleaf_node_append_child(interfaceDecl, methodProto);
  while (!match(parser, TOKEN_KIND_RBRACE))
  {
    methodProto = parse_method_proto(parser);
    ast_nonleaf_node_append_child(interfaceDecl, methodProto);
  }
  next(parser);
  return (AstNode *) interfaceDecl;
}

static inline AstNode *parse_method_proto(Parser *parser)
{
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *retType = parse_type(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
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
end:
  consume(parser, TOKEN_KIND_SEMICOLON);
  AstNonLeafNode *methodProto = ast_nonleaf_node_new(AST_NODE_KIND_FUNC_DECL);
  ast_nonleaf_node_append_child(methodProto, retType);
  ast_nonleaf_node_append_child(methodProto, ident);
  ast_nonleaf_node_append_child(methodProto, (AstNode *) params);
  ast_nonleaf_node_append_child(methodProto, NULL);
  return (AstNode *) methodProto;
}

static inline AstNode *parse_let_decl(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  consume(parser, TOKEN_KIND_EQ);
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  AstNonLeafNode *letDecl = ast_nonleaf_node_new(AST_NODE_KIND_LET_DECL);
  ast_nonleaf_node_append_child(letDecl, ident);
  ast_nonleaf_node_append_child(letDecl, expr);
  return (AstNode *) letDecl;
}

static inline AstNode *parse_var_decl(Parser *parser)
{
  next(parser);
  AstNode *type = parse_type(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNode *expr = NULL;
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    expr = parse_expr(parser);
  }
  consume(parser, TOKEN_KIND_SEMICOLON);
  AstNonLeafNode *varDecl = ast_nonleaf_node_new(AST_NODE_KIND_VAR_DECL);
  ast_nonleaf_node_append_child(varDecl, type);
  ast_nonleaf_node_append_child(varDecl, ident);
  ast_nonleaf_node_append_child(varDecl, expr);
  return (AstNode *) varDecl;
}

static inline AstNode *parse_stmt(Parser *parser)
{
  if (match(parser, TOKEN_KIND_TYPE_KW))
    return parse_type_decl(parser);
  if (match(parser, TOKEN_KIND_FN_KW))
    return parse_func_decl(parser, false);
  if (match(parser, TOKEN_KIND_STRUCT_KW))
    return parse_struct_decl(parser);
  if (match(parser, TOKEN_KIND_INTERFACE_KW))
    return parse_interface_decl(parser);
  if (match(parser, TOKEN_KIND_LET_KW))
    return parse_let_decl(parser);
  if (match(parser, TOKEN_KIND_VAR_KW))
    return parse_var_decl(parser);
  if (match(parser, TOKEN_KIND_LBRACE))
    return parse_block(parser);
  if (match(parser, TOKEN_KIND_IF_KW))
    return parse_if_stmt(parser);
  if (match(parser, TOKEN_KIND_LOOP_KW))
    return parse_loop_stmt(parser);
  if (match(parser, TOKEN_KIND_WHILE_KW))
    return parse_while_stmt(parser);
  if (match(parser, TOKEN_KIND_DO_KW))
    return parse_do_while_stmt(parser);
  if (match(parser, TOKEN_KIND_FOR_KW))
    return parse_for_stmt(parser);
  if (match(parser, TOKEN_KIND_BREAK_KW))
    return parse_break_stmt(parser);
  if (match(parser, TOKEN_KIND_CONTINUE_KW))
    return parse_continue_stmt(parser);
  if (match(parser, TOKEN_KIND_RETURN_KW))
    return parse_return_stmt(parser);
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  return expr;
}

static inline AstNode *parse_if_stmt(Parser *parser)
{
  next(parser);
  AstNode *expr = parse_expr(parser);
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *thenBlock = parse_block(parser);
  AstNode *elseBlock = NULL;
  if (match(parser, TOKEN_KIND_ELSE_KW))
  {
    next(parser);
    if (!match(parser, TOKEN_KIND_LBRACE))
      unexpected_token_error(parser);
    elseBlock = parse_block(parser);
  }
  AstNonLeafNode *ifStmt = ast_nonleaf_node_new(AST_NODE_KIND_IF);
  ast_nonleaf_node_append_child(ifStmt, expr);
  ast_nonleaf_node_append_child(ifStmt, thenBlock);
  ast_nonleaf_node_append_child(ifStmt, elseBlock);
  return (AstNode *) ifStmt;
}

static inline AstNode *parse_loop_stmt(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *block = parse_block(parser);
  AstNonLeafNode *loopStmt = ast_nonleaf_node_new(AST_NODE_KIND_LOOP);
  ast_nonleaf_node_append_child(loopStmt, block);
  return (AstNode *) loopStmt;
}

static inline AstNode *parse_while_stmt(Parser *parser)
{
  next(parser);
  AstNode *expr = parse_expr(parser);
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *block = parse_block(parser);
  AstNonLeafNode *whileStmt = ast_nonleaf_node_new(AST_NODE_KIND_WHILE);
  ast_nonleaf_node_append_child(whileStmt, expr);
  ast_nonleaf_node_append_child(whileStmt, block);
  return (AstNode *) whileStmt;
}

static inline AstNode *parse_do_while_stmt(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *block = parse_block(parser);
  consume(parser, TOKEN_KIND_WHILE_KW);
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  AstNonLeafNode *doWhileStmt = ast_nonleaf_node_new(AST_NODE_KIND_DO_WHILE);
  ast_nonleaf_node_append_child(doWhileStmt, block);
  ast_nonleaf_node_append_child(doWhileStmt, expr);
  return (AstNode *) doWhileStmt;
}

static inline AstNode *parse_for_stmt(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  consume(parser, TOKEN_KIND_IN_KW);
  AstNode *expr = parse_expr(parser);
  if (!match(parser, TOKEN_KIND_LBRACE))
    unexpected_token_error(parser);
  AstNode *block = parse_block(parser);
  AstNonLeafNode *forStmt = ast_nonleaf_node_new(AST_NODE_KIND_FOR);
  ast_nonleaf_node_append_child(forStmt, ident);
  ast_nonleaf_node_append_child(forStmt, expr);
  ast_nonleaf_node_append_child(forStmt, block);
  return (AstNode *) forStmt;
}

static inline AstNode *parse_break_stmt(Parser *parser)
{
  Token token = current(parser);
  next(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_BREAK, token);
}

static inline AstNode *parse_continue_stmt(Parser *parser)
{
  Token token = current(parser);
  next(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
  return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_CONTINUE, token);
}

static inline AstNode *parse_return_stmt(Parser *parser)
{
  next(parser);
  AstNonLeafNode *retStmt = ast_nonleaf_node_new(AST_NODE_KIND_RETURN);
  AstNode *expr = NULL;
  if (match(parser, TOKEN_KIND_SEMICOLON))
  {
    next(parser);
    goto end;
  }
  expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_SEMICOLON);
end:
  ast_nonleaf_node_append_child(retStmt, expr);
  return (AstNode *) retStmt;
}

static inline AstNode *parse_expr(Parser *parser)
{
  AstNode *lhs = parse_or_expr(parser);
  AstNodeKind kind = AST_NODE_KIND_ASSIGN;
  if (match(parser, TOKEN_KIND_EQ))
  {
    next(parser);
    goto end;
  }
  if (match(parser, TOKEN_KIND_PIPEEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_BOR_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_CARETEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_BXOR_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_AMPEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_BAND_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_LTLTEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_SHL_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_GTGTEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_SHR_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_PLUSEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_ADD_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_MINUSEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_SUB_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_STAREQ))
  {
    next(parser);
    kind = AST_NODE_KIND_MUL_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_SLASHEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_DIV_ASSIGN;
    goto end;
  }
  if (match(parser, TOKEN_KIND_PERCENTEQ))
  {
    next(parser);
    kind = AST_NODE_KIND_MOD_ASSIGN;
    goto end;
  }
  return lhs;
  AstNode *rhs;
end:
  rhs = parse_expr(parser);
  AstNonLeafNode *assign = ast_nonleaf_node_new(kind);
  ast_nonleaf_node_append_child(assign, lhs);
  ast_nonleaf_node_append_child(assign, rhs);
  return (AstNode *) assign;
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
  AstNode *lhs = parse_bor_expr(parser);
  while (match(parser, TOKEN_KIND_AMPAMP))
  {
    next(parser);
    AstNode *rhs = parse_bor_expr(parser);
    AstNonLeafNode *and = ast_nonleaf_node_new(AST_NODE_KIND_AND);
    ast_nonleaf_node_append_child(and, lhs);
    ast_nonleaf_node_append_child(and, rhs);
    lhs = (AstNode *) and;
  }
  return lhs;
}

static inline AstNode *parse_bor_expr(Parser *parser)
{
  AstNode *lhs = parse_bxor_expr(parser);
  while (match(parser, TOKEN_KIND_PIPE))
  {
    next(parser);
    AstNode *rhs = parse_bxor_expr(parser);
    AstNonLeafNode *bor = ast_nonleaf_node_new(AST_NODE_KIND_BOR);
    ast_nonleaf_node_append_child(bor, lhs);
    ast_nonleaf_node_append_child(bor, rhs);
    lhs = (AstNode *) bor;
  }
  return lhs;
}

static inline AstNode *parse_bxor_expr(Parser *parser)
{
  AstNode *lhs = parse_band_expr(parser);
  while (match(parser, TOKEN_KIND_CARET))
  {
    next(parser);
    AstNode *rhs = parse_band_expr(parser);
    AstNonLeafNode *bxor = ast_nonleaf_node_new(AST_NODE_KIND_BXOR);
    ast_nonleaf_node_append_child(bxor, lhs);
    ast_nonleaf_node_append_child(bxor, rhs);
    lhs = (AstNode *) bxor;
  }
  return lhs;
}

static inline AstNode *parse_band_expr(Parser *parser)
{
  AstNode *lhs = parse_eq_expr(parser);
  while (match(parser, TOKEN_KIND_AMP))
  {
    next(parser);
    AstNode *rhs = parse_eq_expr(parser);
    AstNonLeafNode *band = ast_nonleaf_node_new(AST_NODE_KIND_BAND);
    ast_nonleaf_node_append_child(band, lhs);
    ast_nonleaf_node_append_child(band, rhs);
    lhs = (AstNode *) band;
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
  AstNode *lhs = parse_shift_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_LT))
    {
      next(parser);
      AstNode *rhs = parse_shift_expr(parser);
      AstNonLeafNode *lt = ast_nonleaf_node_new(AST_NODE_KIND_LT);
      ast_nonleaf_node_append_child(lt, lhs);
      ast_nonleaf_node_append_child(lt, rhs);
      lhs = (AstNode *) lt;
      continue;
    }
    if (match(parser, TOKEN_KIND_LE))
    {
      next(parser);
      AstNode *rhs = parse_shift_expr(parser);
      AstNonLeafNode *le = ast_nonleaf_node_new(AST_NODE_KIND_LE);
      ast_nonleaf_node_append_child(le, lhs);
      ast_nonleaf_node_append_child(le, rhs);
      lhs = (AstNode *) le;
      continue;
    }
    if (match(parser, TOKEN_KIND_GT))
    {
      next(parser);
      AstNode *rhs = parse_shift_expr(parser);
      AstNonLeafNode *gt = ast_nonleaf_node_new(AST_NODE_KIND_GT);
      ast_nonleaf_node_append_child(gt, lhs);
      ast_nonleaf_node_append_child(gt, rhs);
      lhs = (AstNode *) gt;
      continue;
    }
    if (match(parser, TOKEN_KIND_GE))
    {
      next(parser);
      AstNode *rhs = parse_shift_expr(parser);
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

static inline AstNode *parse_shift_expr(Parser *parser)
{
  AstNode *lhs = parse_range_expr(parser);
  for (;;)
  {
    if (match(parser, TOKEN_KIND_LTLT))
    {
      next(parser);
      AstNode *rhs = parse_range_expr(parser);
      AstNonLeafNode *shl = ast_nonleaf_node_new(AST_NODE_KIND_SHL);
      ast_nonleaf_node_append_child(shl, lhs);
      ast_nonleaf_node_append_child(shl, rhs);
      lhs = (AstNode *) shl;
      continue;
    }
    if (match(parser, TOKEN_KIND_GTGT))
    {
      next(parser);
      AstNode *rhs = parse_range_expr(parser);
      AstNonLeafNode *shr = ast_nonleaf_node_new(AST_NODE_KIND_SHR);
      ast_nonleaf_node_append_child(shr, lhs);
      ast_nonleaf_node_append_child(shr, rhs);
      lhs = (AstNode *) shr;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_range_expr(Parser *parser)
{
  AstNode *lhs = parse_add_expr(parser);
  if (match(parser, TOKEN_KIND_DOTDOT))
  {
    next(parser);
    AstNode *rhs = parse_add_expr(parser);
    AstNonLeafNode *range = ast_nonleaf_node_new(AST_NODE_KIND_RANGE);
    ast_nonleaf_node_append_child(range, lhs);
    ast_nonleaf_node_append_child(range, rhs);
    return (AstNode *) range;
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
  if (match(parser, TOKEN_KIND_TILDE))
  {
    next(parser);
    AstNode *expr = parse_unary_expr(parser);
    AstNonLeafNode *bnot = ast_nonleaf_node_new(AST_NODE_KIND_BNOT);
    ast_nonleaf_node_append_child(bnot, expr);
    return (AstNode *) bnot;
  }
  return parse_prim_expr(parser);
}

static inline AstNode *parse_prim_expr(Parser *parser)
{
  if (match(parser, TOKEN_KIND_VOID_KW))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_VOID, token);
  }
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
  if (match(parser, TOKEN_KIND_RUNE))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_RUNE, token);
  }
  if (match(parser, TOKEN_KIND_STRING))
  {
    Token token = current(parser);
    next(parser);
    return (AstNode *) ast_leaf_node_new(AST_NODE_KIND_STRING, token);
  }
  if (match(parser, TOKEN_KIND_LBRACKET))
    return parse_array_expr(parser);
  if (match(parser, TOKEN_KIND_FN_KW))
    return parse_func_decl(parser, true);
  if (match(parser, TOKEN_KIND_NEW_KW))
    return parse_new_expr(parser);
  if (match(parser, TOKEN_KIND_AMP))
    return parse_ref_expr(parser);
  if (match(parser, TOKEN_KIND_IDENT))
    return parse_ident_expr(parser);
  if (match(parser, TOKEN_KIND_TRY_KW))
    return parse_try_expr(parser);
  if (match(parser, TOKEN_KIND_IF_KW))
    return parse_if_expr(parser);
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

static inline AstNode *parse_array_expr(Parser *parser)
{
  next(parser);
  AstNonLeafNode *array = ast_nonleaf_node_new(AST_NODE_KIND_ARRAY);
  if (!match(parser, TOKEN_KIND_RBRACKET))
  {
    AstNode *expr = parse_expr(parser);
    ast_nonleaf_node_append_child(array, expr);
    while (match(parser, TOKEN_KIND_COMMA))
    {
      next(parser);
      expr = parse_expr(parser);
      ast_nonleaf_node_append_child(array, expr);
    }
  }
  consume(parser, TOKEN_KIND_RBRACKET);
  return (AstNode *) array;
}

static inline AstNode *parse_new_expr(Parser *parser)
{
  next(parser);
  AstNode *type = parse_type(parser);
  AstNonLeafNode *newExpr = ast_nonleaf_node_new(AST_NODE_KIND_NEW);
  ast_nonleaf_node_append_child(newExpr, type);
  consume(parser, TOKEN_KIND_LPAREN);
  if (match(parser, TOKEN_KIND_RPAREN))
  {
    next(parser);
    return (AstNode *) newExpr;
  }
  AstNode *expr = parse_expr(parser);
  ast_nonleaf_node_append_child(newExpr, expr);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    expr = parse_expr(parser);
    ast_nonleaf_node_append_child(newExpr, expr);
  }
  consume(parser, TOKEN_KIND_RPAREN);
  return (AstNode *) newExpr;
}

static inline AstNode *parse_ref_expr(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  Token token = current(parser);
  next(parser);
  AstNode *lhs = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  AstNonLeafNode *ref = ast_nonleaf_node_new(AST_NODE_KIND_REF);
  AstNode *subscr = parse_subscr(parser, lhs);
  while (subscr)
  {
    lhs = subscr;
    subscr = parse_subscr(parser, lhs);
  }
  ast_nonleaf_node_append_child(ref, lhs);
  return (AstNode *) ref;
}

static inline AstNode *parse_ident_expr(Parser *parser)
{
  Token token = current(parser);
  next(parser);
  AstNode *lhs = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
  for (;;)
  {
    AstNode *subscr = parse_subscr(parser, lhs);
    if (subscr)
    {
      lhs = subscr;
      continue;
    }
    AstNode *call = parse_call(parser, lhs);
    if (call)
    {
      lhs = call;
      continue;
    }
    break;
  }
  return lhs;
}

static inline AstNode *parse_try_expr(Parser *parser)
{
  next(parser);
  if (!match(parser, TOKEN_KIND_IDENT))
    unexpected_token_error(parser);
  AstNode *expr = parse_expr(parser);
  AstNonLeafNode *tryExpr = ast_nonleaf_node_new(AST_NODE_KIND_TRY);
  ast_nonleaf_node_append_child(tryExpr, expr);
  return (AstNode *) tryExpr;
}

static inline AstNode *parse_call(Parser *parser, AstNode *lhs)
{
  if (!match(parser, TOKEN_KIND_LPAREN))
    return NULL;
  next(parser);
  AstNonLeafNode *call = ast_nonleaf_node_new(AST_NODE_KIND_CALL);
  ast_nonleaf_node_append_child(call, lhs);
  if (match(parser, TOKEN_KIND_RPAREN))
  {
    next(parser);
    return (AstNode *) call;
  }
  AstNode *expr = parse_expr(parser);
  ast_nonleaf_node_append_child(call, expr);
  while (match(parser, TOKEN_KIND_COMMA))
  {
    next(parser);
    expr = parse_expr(parser);
    ast_nonleaf_node_append_child(call, expr);
  }
  consume(parser, TOKEN_KIND_RPAREN);
  return (AstNode *) call;
}

static inline AstNode *parse_subscr(Parser *parser, AstNode *lhs)
{
  if (match(parser, TOKEN_KIND_LBRACKET))
  {
    next(parser);
    AstNode *expr = parse_expr(parser);
    consume(parser, TOKEN_KIND_RBRACKET);
    AstNonLeafNode *subscr = ast_nonleaf_node_new(AST_NODE_KIND_ELEMENT);
    ast_nonleaf_node_append_child(subscr, lhs);
    ast_nonleaf_node_append_child(subscr, expr);
    return (AstNode *) subscr;
  }
  if (match(parser, TOKEN_KIND_DOT))
  {
    next(parser);
    if (!match(parser, TOKEN_KIND_IDENT))
      unexpected_token_error(parser);
    Token token = current(parser);
    next(parser);
    AstNode *ident = (AstNode *) ast_leaf_node_new(AST_NODE_KIND_IDENT, token);
    AstNonLeafNode *subscr = ast_nonleaf_node_new(AST_NODE_KIND_FIELD);
    ast_nonleaf_node_append_child(subscr, lhs);
    ast_nonleaf_node_append_child(subscr, ident);
    return (AstNode *) subscr;
  }
  return NULL;
}

static inline AstNode *parse_if_expr(Parser *parser)
{
  next(parser);
  AstNode *expr = parse_expr(parser);
  consume(parser, TOKEN_KIND_LBRACE);
  AstNode *thenExpr = parse_expr(parser);
  consume(parser, TOKEN_KIND_RBRACE);
  consume(parser, TOKEN_KIND_ELSE_KW);
  consume(parser, TOKEN_KIND_LBRACE);
  AstNode *elseExpr = parse_expr(parser);
  consume(parser, TOKEN_KIND_RBRACE);
  AstNonLeafNode *ifExpr = ast_nonleaf_node_new(AST_NODE_KIND_IF);
  ast_nonleaf_node_append_child(ifExpr, expr);
  ast_nonleaf_node_append_child(ifExpr, thenExpr);
  ast_nonleaf_node_append_child(ifExpr, elseExpr);
  return (AstNode *) ifExpr;
}

void parser_init(Parser *parser, char *file, char *source)
{
  lexer_init(&parser->lex, file, source);
}

AstNode *parser_parse(Parser *parser)
{
  return parse_module(parser);
}
