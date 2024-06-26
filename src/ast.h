//
// ast.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef AST_H
#define AST_H

#include "lexer.h"

#define AST_NODE_HEADER AstNodeKind kind;

typedef enum
{
  AST_NODE_KIND_MODULE,    AST_NODE_KIND_TYPE_DECL,   AST_NODE_KIND_VAR_DECL,
  AST_NODE_KIND_FUNC_DECL, AST_NODE_KIND_PARAMS,      AST_NODE_KIND_INOUT_PARAM,
  AST_NODE_KIND_REF,       AST_NODE_KIND_BOOL_TYPE,   AST_NODE_KIND_BYTE_TYPE,
  AST_NODE_KIND_INT_TYPE,  AST_NODE_KIND_UINT_TYPE,   AST_NODE_KIND_FLOAT_TYPE,
  AST_NODE_KIND_RUNE_TYPE, AST_NODE_KIND_STRING_TYPE, AST_NODE_KIND_FUNC_TYPE,
  AST_NODE_KIND_BLOCK,     AST_NODE_KIND_ASSIGN,      AST_NODE_KIND_IF,
  AST_NODE_KIND_RETURN,    AST_NODE_KIND_OR,          AST_NODE_KIND_AND,
  AST_NODE_KIND_EQ,        AST_NODE_KIND_NE,          AST_NODE_KIND_LT,
  AST_NODE_KIND_LE,        AST_NODE_KIND_GT,          AST_NODE_KIND_GE,
  AST_NODE_KIND_ADD,       AST_NODE_KIND_SUB,         AST_NODE_KIND_MUL,
  AST_NODE_KIND_DIV,       AST_NODE_KIND_MOD,         AST_NODE_KIND_NOT,
  AST_NODE_KIND_NEG,       AST_NODE_KIND_CALL,        AST_NODE_KIND_FALSE,
  AST_NODE_KIND_TRUE,      AST_NODE_KIND_INT,         AST_NODE_KIND_FLOAT,
  AST_NODE_KIND_RUNE,      AST_NODE_KIND_STRING,      AST_NODE_KIND_IDENT
} AstNodeKind;

typedef struct
{
  AST_NODE_HEADER
} AstNode;

typedef struct
{
  AST_NODE_HEADER
  Token token;
} AstLeafNode;

typedef struct
{
  AST_NODE_HEADER
  int     capacity;
  int     count;
  AstNode **children;
} AstNonLeafNode;

const char *ast_node_kind_name(AstNodeKind kind);
AstLeafNode *ast_leaf_node_new(AstNodeKind kind, Token token);
AstNonLeafNode *ast_nonleaf_node_new(AstNodeKind kind);
void ast_nonleaf_node_append_child(AstNonLeafNode *node, AstNode *child);
void ast_print(AstNode *ast);

#endif // AST_H
