//
// codegen.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "codegen.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define assert(c) if (!(c)) { fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #c, __FILE__, __LINE__); exit(EXIT_FAILURE); }

static inline void generate_module(AstNode *node, Buffer *code);
static inline void generate_decl(AstNode *node, Buffer *code);
static inline bool generate_import_decl(AstNode *node, Buffer *code);
static inline bool generate_typealias_decl(AstNode *node, Buffer *code);
static inline bool generate_func_decl(AstNode *node, Buffer *code);
static inline bool generate_ident(AstNode *node, Buffer *code);
static inline void generate_params(AstNode *node, Buffer *code);
static inline void generate_param(AstNode *node, Buffer *code);
static inline void generate_type(AstNode *node, Buffer *code);
static inline bool generate_block(AstNode *node, Buffer *code);
static inline bool generate_struct_decl(AstNode *node, Buffer *code);
static inline bool generate_const_decl(AstNode *node, Buffer *code);
static inline bool generate_var_decl(AstNode *node, Buffer *code);
static inline void generate_stmt(AstNode *node, Buffer *code);
static inline bool generate_if_stmt(AstNode *node, Buffer *code);
static inline bool generate_switch_stmt(AstNode *node, Buffer *code);
static inline bool generate_while_stmt(AstNode *node, Buffer *code);
static inline bool generate_do_while_stmt(AstNode *node, Buffer *code);
static inline bool generate_for_stmt(AstNode *node, Buffer *code);
static inline bool generate_break_stmt(AstNode *node, Buffer *code);
static inline bool generate_continue_stmt(AstNode *node, Buffer *code);
static inline bool generate_return_stmt(AstNode *node, Buffer *code);
static inline void generate_expr(AstNode *node, Buffer *code);
static inline bool generate_binary_expr(AstNode *node, Buffer *code);
static inline bool generate_unary_expr(AstNode *node, Buffer *code);
static inline bool generate_call(AstNode *node, Buffer *code);
static inline bool generate_subscr(AstNode *node, Buffer *code);
static inline bool generate_bool(AstNode *node, Buffer *code);
static inline bool generate_number(AstNode *node, Buffer *code);
static inline bool generate_char(AstNode *node, Buffer *code);
static inline bool generate_string(AstNode *node, Buffer *code);
static inline bool generate_if_expr(AstNode *node, Buffer *code);

static inline void generate_module(AstNode *node, Buffer *code)
{
  char *include = "#include \"runtime.h\"\n";
  buffer_write(code, strlen(include), include);
  assert(node->kind == AST_NODE_KIND_MODULE);
  AstNonLeafNode *module = (AstNonLeafNode *) node;
  for (int i = 0; i < module->count; ++i)
  {
    AstNode *node = module->children[i];
    generate_decl(node, code);
  }
  buffer_write(code, 1, "\n");
}

static inline void generate_decl(AstNode *node, Buffer *code)
{
  if (generate_import_decl(node, code)) return;
  if (generate_typealias_decl(node, code)) return;
  if (generate_func_decl(node, code)) return;
  if (generate_struct_decl(node, code)) return;
  if (generate_const_decl(node, code)) return;
  assert(false);
}

static inline bool generate_import_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_IMPORT_DECL) return false;
  AstNonLeafNode *importDecl = (AstNonLeafNode *) node;
  AstNode *ident = importDecl->children[0];
  buffer_write(code, 10, "#include \"");
  assert(generate_ident(ident, code));
  buffer_write(code, 2, "\"\n");
  return true;
}

static inline bool generate_typealias_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_TYPEALIAS_DECL) return false;
  AstNonLeafNode *typeDecl = (AstNonLeafNode *) node;
  AstNode *ident = typeDecl->children[0];
  // TODO: Implement code generation for polymorphic type declaration.
  AstNode *polyParams = typeDecl->children[1];
  (void) polyParams;
  AstNode *type = typeDecl->children[2];
  buffer_write(code, 8, "typedef ");
  generate_type(type, code);
  buffer_write(code, 1, " ");
  assert(generate_ident(ident, code));
  buffer_write(code, 1, ";");
  return true;
}

static inline bool generate_func_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_FUNC_DECL) return false;
  AstNonLeafNode *funcDecl = (AstNonLeafNode *) node;
  AstNode *retType = funcDecl->children[0];
  AstNode *ident = funcDecl->children[1];
  AstNode *params = funcDecl->children[2];
  AstNode *block = funcDecl->children[3];
  generate_type(retType, code);
  buffer_write(code, 1, " ");
  assert(generate_ident(ident, code));
  generate_params(params, code);
  assert(generate_block(block, code));
  buffer_write(code, 1, "\n");
  return true;
}

static inline bool generate_ident(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_IDENT) return false;
  Token *token = &((AstLeafNode *) node)->token;
  buffer_write(code, token->length, token->chars);
  return true;
}

static inline void generate_params(AstNode *node, Buffer *code)
{
  assert(node->kind == AST_NODE_KIND_PARAMS);
  AstNonLeafNode *params = (AstNonLeafNode *) node;
  buffer_write(code, 1, "(");
  for (int i = 0; i < params->count; ++i)
  {
    if (i > 0) buffer_write(code, 1, ",");
    AstNode *param = params->children[i];
    generate_param(param, code);
  }
  buffer_write(code, 1, ")");
}

static inline void generate_param(AstNode *node, Buffer *code)
{
  assert(node->kind == AST_NODE_KIND_VAR_DECL);
  AstNonLeafNode *param = (AstNonLeafNode *) node;
  AstNode *type = param->children[0];
  AstNode *ident = param->children[1];
  generate_type(type, code);
  buffer_write(code, 1, " ");
  assert(generate_ident(ident, code));
}

static inline void generate_type(AstNode *node, Buffer *code)
{
  if (node->kind == AST_NODE_KIND_IDENT)
  {
    assert(generate_ident(node, code));
    return;
  }
  // TODO: Implement other types.
  assert(false);
}

static inline bool generate_block(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_BLOCK) return false;
  AstNonLeafNode *block = (AstNonLeafNode *) node;
  buffer_write(code, 1, "{");
  for (int i = 0; i < block->count; ++i)
  {
    AstNode *stmt = block->children[i];
    generate_stmt(stmt, code);
  }
  buffer_write(code, 1, "}");
  return true;
}

static inline bool generate_struct_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_STRUCT_DECL) return false;
  AstNonLeafNode *structDecl = (AstNonLeafNode *) node;
  AstNode *ident = structDecl->children[0];
  // TODO: Implement code generation for polymorphic struct declaration.
  AstNode *polyParams = structDecl->children[1];
  (void) polyParams;
  AstNode **fields = &structDecl->children[2];
  int numFields = structDecl->count - 2;
  buffer_write(code, 15, "typedef struct ");
  assert(generate_ident(ident, code));
  buffer_write(code, 1, "{");
  for (int i = 0; i < numFields; ++i)
  {
    assert(fields[i]->kind == AST_NODE_KIND_VAR_DECL);
    AstNonLeafNode *field = (AstNonLeafNode *) fields[i];
    AstNode *type = field->children[0];
    AstNode *ident = field->children[1];
    generate_type(type, code);
    buffer_write(code, 1, " ");
    assert(generate_ident(ident, code));
    buffer_write(code, 1, ";");
  }
  buffer_write(code, 1, "}");
  assert(generate_ident(ident, code));
  buffer_write(code, 1, ";");
  return true;
}

static inline bool generate_const_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_CONST_DECL) return false;
  AstNonLeafNode *constDecl = (AstNonLeafNode *) node;
  AstNode *ident = constDecl->children[0];
  AstNode *expr = constDecl->children[1];
  // TODO: Implement code generation for const declaration.
  buffer_write(code, 6, "const ");
  assert(generate_ident(ident, code));
  buffer_write(code, 1, "=");
  generate_expr(expr, code);
  buffer_write(code, 1, ";");
  return true;
}

static inline bool generate_var_decl(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_VAR_DECL) return false;
  AstNonLeafNode *varDecl = (AstNonLeafNode *) node;
  AstNode *type = varDecl->children[0];
  AstNode *ident = varDecl->children[1];
  AstNode *expr = varDecl->children[2];
  generate_type(type, code);
  buffer_write(code, 1, " ");
  assert(generate_ident(ident, code));
  if (expr)
  {
    buffer_write(code, 1, "=");
    generate_expr(expr, code);
  }
  buffer_write(code, 1, ";");
  return true;
}

static inline void generate_stmt(AstNode *node, Buffer *code)
{
  if (generate_const_decl(node, code)) return;
  if (generate_var_decl(node, code)) return;
  if (generate_block(node, code)) return;
  if (generate_if_stmt(node, code)) return;
  if (generate_switch_stmt(node, code)) return;
  if (generate_while_stmt(node, code)) return;
  if (generate_do_while_stmt(node, code)) return;
  if (generate_for_stmt(node, code)) return;
  if (generate_break_stmt(node, code)) return;
  if (generate_continue_stmt(node, code)) return;
  if (generate_return_stmt(node, code)) return;
  // TODO: Implement other statements.
  generate_expr(node, code);
  buffer_write(code, 1, ";");
}

static inline bool generate_if_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_IF) return false;
  AstNonLeafNode *ifStmt = (AstNonLeafNode *) node;
  AstNode *expr = ifStmt->children[0];
  AstNode *thenBlock = ifStmt->children[1];
  AstNode *elseBlock = ifStmt->children[2];
  buffer_write(code, 3, "if(");
  generate_expr(expr, code);
  buffer_write(code, 1, ")");
  assert(generate_block(thenBlock, code));
  if (elseBlock)
  {
    buffer_write(code, 4, "else");
    assert(generate_block(elseBlock, code));
  }
  return true;
}

static inline bool generate_switch_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_SWITCH) return false;
  AstNonLeafNode *switchStmt = (AstNonLeafNode *) node;
  AstNode *expr = switchStmt->children[0];
  AstNode **cases = &switchStmt->children[1];
  AstNode *switchDefault = switchStmt->children[switchStmt->count - 1];
  int numCases = switchStmt->count - 2;
  buffer_write(code, 7, "switch(");
  generate_expr(expr, code);
  buffer_write(code, 2, "){");
  for (int i = 0; i < numCases; ++i)
  {
    assert(cases[i]->kind == AST_NODE_KIND_CASE);
    AstNonLeafNode *switchCase = (AstNonLeafNode *) cases[i];
    AstNode *expr = switchCase->children[0];
    AstNode **stmts = &switchCase->children[1];
    int numStmts = switchCase->count - 1;
    buffer_write(code, 5, "case ");
    generate_expr(expr, code);
    buffer_write(code, 1, ":");
    for (int j = 0; j < numStmts; ++j)
    {
      AstNode *stmt = stmts[j];
      generate_stmt(stmt, code);
    }
    buffer_write(code, 6, "break;");
  }
  if (switchDefault)
  {
    assert(switchDefault->kind == AST_NODE_KIND_DEFAULT);
    AstNonLeafNode *defaulCase = (AstNonLeafNode *) switchDefault;
    buffer_write(code, 8, "default:");
    for (int i = 0; i < defaulCase->count; ++i)
    {
      AstNode *stmt = defaulCase->children[i];
      generate_stmt(stmt, code);
    }
    buffer_write(code, 6, "break;");
  }
  buffer_write(code, 1, "}");
  return true;
}

static inline bool generate_while_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_WHILE) return false;
  AstNonLeafNode *whileStmt = (AstNonLeafNode *) node;
  AstNode *expr = whileStmt->children[0];
  AstNode *block = whileStmt->children[1];
  buffer_write(code, 6, "while(");
  generate_expr(expr, code);
  buffer_write(code, 1, ")");
  assert(generate_block(block, code));
  return true;
}

static inline bool generate_do_while_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_DO_WHILE) return false;
  AstNonLeafNode *doWhileStmt = (AstNonLeafNode *) node;
  AstNode *block = doWhileStmt->children[0];
  AstNode *expr = doWhileStmt->children[1];
  buffer_write(code, 2, "do");
  assert(generate_block(block, code));
  buffer_write(code, 6, "while(");
  generate_expr(expr, code);
  buffer_write(code, 2, ");");
  return true;
}

static inline bool generate_for_stmt(AstNode *node, Buffer *code)
{
  // TODO: Implement code generation for for statement.
  (void) node;
  (void) code;
  return false;
}

static inline bool generate_break_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_BREAK) return false;
  buffer_write(code, 6, "break;");
  return true;
}

static inline bool generate_continue_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_CONTINUE) return false;
  buffer_write(code, 9, "continue;");
  return true;
}

static inline bool generate_return_stmt(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_RETURN) return false;
  AstNonLeafNode *retStmt = (AstNonLeafNode *) node;
  AstNode *expr = retStmt->children[0];
  buffer_write(code, 6, "return");
  if (expr)
  {
    buffer_write(code, 1, " ");
    generate_expr(expr, code);
  }
  buffer_write(code, 1, ";");
  return true;
}

static inline void generate_expr(AstNode *node, Buffer *code)
{
  if (generate_binary_expr(node, code)) return;
  if (generate_unary_expr(node, code)) return;
  if (generate_call(node, code)) return;
  if (generate_subscr(node, code)) return;
  if (generate_ident(node, code)) return;
  if (generate_bool(node, code)) return;
  if (generate_number(node, code)) return;
  if (generate_char(node, code)) return;
  if (generate_string(node, code)) return;
  if (generate_if_expr(node, code)) return;
  // TODO: Implement other expressions.
  assert(false);
}

static inline bool generate_binary_expr(AstNode *node, Buffer *code)
{
  char *op = NULL;
  if (node->kind == AST_NODE_KIND_ASSIGN)           op = "=";
  else if (node->kind == AST_NODE_KIND_BOR_ASSIGN)  op = "|=";
  else if (node->kind == AST_NODE_KIND_BXOR_ASSIGN) op = "^=";
  else if (node->kind == AST_NODE_KIND_BAND_ASSIGN) op = "&=";
  else if (node->kind == AST_NODE_KIND_SHL_ASSIGN)  op = "<<=";
  else if (node->kind == AST_NODE_KIND_SHR_ASSIGN)  op = ">>=";
  else if (node->kind == AST_NODE_KIND_ADD_ASSIGN)  op = "+=";
  else if (node->kind == AST_NODE_KIND_SUB_ASSIGN)  op = "-=";
  else if (node->kind == AST_NODE_KIND_MUL_ASSIGN)  op = "*=";
  else if (node->kind == AST_NODE_KIND_DIV_ASSIGN)  op = "/=";
  else if (node->kind == AST_NODE_KIND_MOD_ASSIGN)  op = "%=";
  else if (node->kind == AST_NODE_KIND_OR)          op = "||";
  else if (node->kind == AST_NODE_KIND_AND)         op = "&&";
  else if (node->kind == AST_NODE_KIND_BOR)         op = "|";
  else if (node->kind == AST_NODE_KIND_BXOR)        op = "^";
  else if (node->kind == AST_NODE_KIND_BAND)        op = "&";
  else if (node->kind == AST_NODE_KIND_EQ)          op = "==";
  else if (node->kind == AST_NODE_KIND_NE)          op = "!=";
  else if (node->kind == AST_NODE_KIND_LT)          op = "<";
  else if (node->kind == AST_NODE_KIND_LE)          op = "<=";
  else if (node->kind == AST_NODE_KIND_GT)          op = ">";
  else if (node->kind == AST_NODE_KIND_GE)          op = ">=";
  else if (node->kind == AST_NODE_KIND_SHL)         op = "<<";
  else if (node->kind == AST_NODE_KIND_SHR)         op = ">>";
  else if (node->kind == AST_NODE_KIND_ADD)         op = "+";
  else if (node->kind == AST_NODE_KIND_SUB)         op = "-";
  else if (node->kind == AST_NODE_KIND_MUL)         op = "*";
  else if (node->kind == AST_NODE_KIND_DIV)         op = "/";
  else if (node->kind == AST_NODE_KIND_MOD)         op = "%";
  if (!op) return false;
  AstNonLeafNode *binary = (AstNonLeafNode *) node;
  AstNode *lhs = binary->children[0];
  AstNode *rhs = binary->children[1];
  generate_expr(lhs, code);
  buffer_write(code, strlen(op), op);
  generate_expr(rhs, code);
  return true;
}

static inline bool generate_unary_expr(AstNode *node, Buffer *code)
{
  char *op = NULL;
  if (node->kind == AST_NODE_KIND_NOT)       op = "!";
  else if (node->kind == AST_NODE_KIND_NEG)  op = "-";
  else if (node->kind == AST_NODE_KIND_BNOT) op = "~";
  if (!op) return false;
  AstNonLeafNode *unary = (AstNonLeafNode *) node;
  AstNode *expr = unary->children[0];
  buffer_write(code, strlen(op), op);
  generate_expr(expr, code);
  return true;
}

static inline bool generate_call(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_CALL) return false;
  AstNonLeafNode *call = (AstNonLeafNode *) node;
  AstNode *expr = call->children[0];
  AstNode **args = &call->children[1];
  int numArgs = call->count - 1;
  generate_expr(expr, code);
  buffer_write(code, 1, "(");
  for (int i = 0; i < numArgs; ++i)
  {
    if (i > 0) buffer_write(code, 1, ",");
    generate_expr(args[i], code);
  }
  buffer_write(code, 1, ")");
  return true;
}

static inline bool generate_subscr(AstNode *node, Buffer *code)
{
  if (node->kind == AST_NODE_KIND_ELEMENT)
  {
    AstNonLeafNode *subscr = (AstNonLeafNode *) node;
    AstNode *lhs = subscr->children[0];
    AstNode *expr = subscr->children[1];
    generate_expr(lhs, code);
    buffer_write(code, 1, "[");
    generate_expr(expr, code);
    buffer_write(code, 1, "]");
    return true;
  }
  if (node->kind == AST_NODE_KIND_FIELD)
  {
    AstNonLeafNode *subscr = (AstNonLeafNode *) node;
    AstNode *lhs = subscr->children[0];
    AstNode *ident = subscr->children[1];
    generate_expr(lhs, code);
    buffer_write(code, 1, ".");
    assert(generate_ident(ident, code));
    return true;
  }
  return false;
}

static inline bool generate_bool(AstNode *node, Buffer *code)
{
  char *str = NULL;
  if (node->kind == AST_NODE_KIND_FALSE)     str = "false";
  else if (node->kind == AST_NODE_KIND_TRUE) str = "true";
  if (!str) return false;
  buffer_write(code, strlen(str), str);
  return true;
}

static inline bool generate_number(AstNode *node, Buffer *code)
{
  if (node->kind == AST_NODE_KIND_INT
   || node->kind == AST_NODE_KIND_FLOAT)
  {
    Token *token = &((AstLeafNode *) node)->token;
    buffer_write(code, token->length, token->chars);
    return true;
  }
  return false;
}

static inline bool generate_char(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_CHAR) return false;
  Token *token = &((AstLeafNode *) node)->token;
  buffer_write(code, 1, "'");
  buffer_write(code, token->length, token->chars);
  buffer_write(code, 1, "'");
  return false;
}

static inline bool generate_string(AstNode *node, Buffer *code)
{
  if (node->kind != AST_NODE_KIND_STRING) return false;
  Token *token = &((AstLeafNode *) node)->token;
  buffer_write(code, 18, "string_from_cstr(\"");
  buffer_write(code, token->length, token->chars);
  buffer_write(code, 2, "\")");
  return true;
}

static inline bool generate_if_expr(AstNode *node, Buffer *code)
{
  assert(node->kind == AST_NODE_KIND_IF);
  AstNonLeafNode *ifExpr = (AstNonLeafNode *) node;
  AstNode *expr = ifExpr->children[0];
  AstNode *thenExpr = ifExpr->children[1];
  AstNode *elseExpr = ifExpr->children[2];
  buffer_write(code, 1, "(");
  generate_expr(expr, code);
  buffer_write(code, 1, "?");
  generate_expr(thenExpr, code);
  buffer_write(code, 1, ":");
  generate_expr(elseExpr, code);
  buffer_write(code, 1, ")");
  return true;
}

void generate(AstNode *ast, Buffer *code)
{
  generate_module(ast, code);
}
