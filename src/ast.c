//
// ast.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static inline void node_print(AstNode *node, int level);

static inline void node_print(AstNode *node, int level)
{
  for (int i = 0; i < level; ++i) printf("  ");
  if (!node)
  {
    printf("(null)\n");
    return;
  }
  AstNodeKind kind = node->kind;
  const char *name = ast_node_kind_name(kind);
  switch (kind)
  {
  case AST_NODE_KIND_MODULE:
  case AST_NODE_KIND_TYPE_DECL:
  case AST_NODE_KIND_INTERSECT:
  case AST_NODE_KIND_FUNC_DECL:
  case AST_NODE_KIND_PARAMS:
  case AST_NODE_KIND_INOUT_PARAM:
  case AST_NODE_KIND_STRUCT_DECL:
  case AST_NODE_KIND_INTERFACE_DECL:
  case AST_NODE_KIND_VAR_DECL:
  case AST_NODE_KIND_LET_DECL:
  case AST_NODE_KIND_REF:
  case AST_NODE_KIND_FUNC_TYPE:
  case AST_NODE_KIND_TYPE:
  case AST_NODE_KIND_NULLABLE:
  case AST_NODE_KIND_BLOCK:
  case AST_NODE_KIND_ASSIGN:
  case AST_NODE_KIND_BOR_ASSIGN:
  case AST_NODE_KIND_BXOR_ASSIGN:
  case AST_NODE_KIND_BAND_ASSIGN:
  case AST_NODE_KIND_SHL_ASSIGN:
  case AST_NODE_KIND_SHR_ASSIGN:
  case AST_NODE_KIND_ADD_ASSIGN:
  case AST_NODE_KIND_SUB_ASSIGN:
  case AST_NODE_KIND_MUL_ASSIGN:
  case AST_NODE_KIND_DIV_ASSIGN:
  case AST_NODE_KIND_MOD_ASSIGN:
  case AST_NODE_KIND_IF:
  case AST_NODE_KIND_LOOP:
  case AST_NODE_KIND_WHILE:
  case AST_NODE_KIND_DO_WHILE:
  case AST_NODE_KIND_FOR:
  case AST_NODE_KIND_RETURN:
  case AST_NODE_KIND_OR:
  case AST_NODE_KIND_AND:
  case AST_NODE_KIND_EQ:
  case AST_NODE_KIND_NE:
  case AST_NODE_KIND_LT:
  case AST_NODE_KIND_LE:
  case AST_NODE_KIND_GT:
  case AST_NODE_KIND_GE:
  case AST_NODE_KIND_BOR:
  case AST_NODE_KIND_BXOR:
  case AST_NODE_KIND_BAND:
  case AST_NODE_KIND_SHL:
  case AST_NODE_KIND_SHR:
  case AST_NODE_KIND_RANGE:
  case AST_NODE_KIND_ADD:
  case AST_NODE_KIND_SUB:
  case AST_NODE_KIND_MUL:
  case AST_NODE_KIND_DIV:
  case AST_NODE_KIND_MOD:
  case AST_NODE_KIND_NOT:
  case AST_NODE_KIND_NEG:
  case AST_NODE_KIND_BNOT:
  case AST_NODE_KIND_CALL:
  case AST_NODE_KIND_ARRAY:
  case AST_NODE_KIND_ELEMENT:
  case AST_NODE_KIND_FIELD:
    {
      AstNonLeafNode *nonleaf = (AstNonLeafNode *) node;
      printf("%s:\n", name);
      for (int i = 0; i < nonleaf->count; ++i)
      {
        AstNode *child = nonleaf->children[i];
        node_print(child, level + 1);
      }
    }
    break;
  case AST_NODE_KIND_BOOL_TYPE:
  case AST_NODE_KIND_BYTE_TYPE:
  case AST_NODE_KIND_INT_TYPE:
  case AST_NODE_KIND_UINT_TYPE:
  case AST_NODE_KIND_FLOAT_TYPE:
  case AST_NODE_KIND_RUNE_TYPE:
  case AST_NODE_KIND_STRING_TYPE:
  case AST_NODE_KIND_BREAK:
  case AST_NODE_KIND_CONTINUE:
  case AST_NODE_KIND_NULL:
  case AST_NODE_KIND_FALSE:
  case AST_NODE_KIND_TRUE:
    printf("%s\n", name);
    break;
  case AST_NODE_KIND_INT:
  case AST_NODE_KIND_FLOAT:
  case AST_NODE_KIND_RUNE:
  case AST_NODE_KIND_STRING:
  case AST_NODE_KIND_IDENT:
    {
      AstLeafNode *leaf = (AstLeafNode *) node;
      Token *token = &leaf->token;
      printf("%s: %.*s\n",name, token->length, token->chars);
    }
    break;
  }
}

const char *ast_node_kind_name(AstNodeKind kind)
{
  char *name = NULL;
  switch (kind)
  {
  case AST_NODE_KIND_MODULE:         name = "Module";        break;
  case AST_NODE_KIND_TYPE_DECL:      name = "TypeDecl";      break;
  case AST_NODE_KIND_INTERSECT:      name = "Intersect";     break;
  case AST_NODE_KIND_FUNC_DECL:      name = "FuncDecl";      break;
  case AST_NODE_KIND_PARAMS:         name = "Params";        break;
  case AST_NODE_KIND_INOUT_PARAM:    name = "InoutParam";    break;
  case AST_NODE_KIND_STRUCT_DECL:    name = "StructDecl";    break;
  case AST_NODE_KIND_INTERFACE_DECL: name = "InterfaceDecl"; break;
  case AST_NODE_KIND_VAR_DECL:       name = "VarDecl";       break;
  case AST_NODE_KIND_LET_DECL:       name = "LetDecl";       break;
  case AST_NODE_KIND_REF:            name = "Ref";           break;
  case AST_NODE_KIND_BOOL_TYPE:      name = "BoolType";      break;
  case AST_NODE_KIND_BYTE_TYPE:      name = "ByteType";      break;
  case AST_NODE_KIND_INT_TYPE:       name = "IntType";       break;
  case AST_NODE_KIND_UINT_TYPE:      name = "UIntType";      break;
  case AST_NODE_KIND_FLOAT_TYPE:     name = "FloatType";     break;
  case AST_NODE_KIND_RUNE_TYPE:      name = "RuneType";      break;
  case AST_NODE_KIND_STRING_TYPE:    name = "StringType";    break;
  case AST_NODE_KIND_FUNC_TYPE:      name = "FuncType";      break;
  case AST_NODE_KIND_TYPE:           name = "Type";          break;
  case AST_NODE_KIND_NULLABLE:       name = "Nullable";      break;
  case AST_NODE_KIND_BLOCK:          name = "Block";         break;
  case AST_NODE_KIND_ASSIGN:         name = "Assign";        break;
  case AST_NODE_KIND_BOR_ASSIGN:     name = "BitOrAssign";   break;
  case AST_NODE_KIND_BXOR_ASSIGN:    name = "BitXorAssign";  break;
  case AST_NODE_KIND_BAND_ASSIGN:    name = "BitAndAssign";  break;
  case AST_NODE_KIND_SHL_ASSIGN:     name = "ShlAssign";     break;
  case AST_NODE_KIND_SHR_ASSIGN:     name = "ShrAssign";     break;
  case AST_NODE_KIND_ADD_ASSIGN:     name = "AddAssign";     break;
  case AST_NODE_KIND_SUB_ASSIGN:     name = "SubAssign";     break;
  case AST_NODE_KIND_MUL_ASSIGN:     name = "MulAssign";     break;
  case AST_NODE_KIND_DIV_ASSIGN:     name = "DivAssign";     break;
  case AST_NODE_KIND_MOD_ASSIGN:     name = "ModAssign";     break;
  case AST_NODE_KIND_IF:             name = "If";            break;
  case AST_NODE_KIND_LOOP:           name = "Loop";          break;
  case AST_NODE_KIND_WHILE:          name = "While";         break;
  case AST_NODE_KIND_DO_WHILE:       name = "DoWhile";       break;
  case AST_NODE_KIND_FOR:            name = "For";           break;
  case AST_NODE_KIND_BREAK:          name = "Break";         break;
  case AST_NODE_KIND_CONTINUE:       name = "Continue";      break;
  case AST_NODE_KIND_RETURN:         name = "Return";        break;
  case AST_NODE_KIND_OR:             name = "Or";            break;
  case AST_NODE_KIND_AND:            name = "And";           break;
  case AST_NODE_KIND_EQ:             name = "Eq";            break;
  case AST_NODE_KIND_NE:             name = "Ne";            break;
  case AST_NODE_KIND_LT:             name = "Lt";            break;
  case AST_NODE_KIND_LE:             name = "Le";            break;
  case AST_NODE_KIND_GT:             name = "Gt";            break;
  case AST_NODE_KIND_GE:             name = "Ge";            break;
  case AST_NODE_KIND_BOR:            name = "BitOr";         break;
  case AST_NODE_KIND_BXOR:           name = "BitXor";        break;
  case AST_NODE_KIND_BAND:           name = "BitAnd";        break;
  case AST_NODE_KIND_SHL:            name = "Shl";           break;
  case AST_NODE_KIND_SHR:            name = "Shr";           break;
  case AST_NODE_KIND_RANGE:          name = "Range";         break;
  case AST_NODE_KIND_ADD:            name = "Add";           break;
  case AST_NODE_KIND_SUB:            name = "Sub";           break;
  case AST_NODE_KIND_MUL:            name = "Mul";           break;
  case AST_NODE_KIND_DIV:            name = "Div";           break;
  case AST_NODE_KIND_MOD:            name = "Mod";           break;
  case AST_NODE_KIND_NOT:            name = "Not";           break;
  case AST_NODE_KIND_NEG:            name = "Neg";           break;
  case AST_NODE_KIND_BNOT:           name = "BitNot";        break;
  case AST_NODE_KIND_CALL:           name = "Call";          break;
  case AST_NODE_KIND_NULL:           name = "Null";          break;
  case AST_NODE_KIND_FALSE:          name = "False";         break;
  case AST_NODE_KIND_TRUE:           name = "True";          break;
  case AST_NODE_KIND_INT:            name = "Int";           break;
  case AST_NODE_KIND_FLOAT:          name = "Float";         break;
  case AST_NODE_KIND_RUNE:           name = "Rune";          break;
  case AST_NODE_KIND_STRING:         name = "String";        break;
  case AST_NODE_KIND_ARRAY:          name = "Array";         break;
  case AST_NODE_KIND_ELEMENT:        name = "Element";       break;
  case AST_NODE_KIND_FIELD:          name = "Field";         break;
  case AST_NODE_KIND_IDENT:          name = "Ident";         break;
  }
  assert(name);
  return name;
}

AstLeafNode *ast_leaf_node_new(AstNodeKind kind, Token token)
{
  AstLeafNode *node = malloc(sizeof(*node));
  node->kind = kind;
  node->token = token;
  return node;
}

AstNonLeafNode *ast_nonleaf_node_new(AstNodeKind kind)
{
  int capacity = 1;
  AstNode **children = malloc(sizeof(*children) * capacity);
  AstNonLeafNode *node = malloc(sizeof(*node));
  node->kind = kind;
  node->capacity = capacity;
  node->count = 0;
  node->children = children;
  return node;
}

void ast_nonleaf_node_append_child(AstNonLeafNode *node, AstNode *child)
{
  if (node->count == node->capacity)
  {
    int newCapacity = node->capacity << 1;
    AstNode **newChildren = realloc(node->children, sizeof(*newChildren) * newCapacity);
    node->capacity = newCapacity;
    node->children = newChildren;
  }
  node->children[node->count] = child;
  ++node->count;
}

void ast_print(AstNode *ast)
{
  node_print(ast, 0);
}
