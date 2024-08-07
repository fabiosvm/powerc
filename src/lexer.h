//
// lexer.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef LEXER_H
#define LEXER_H

typedef enum
{
  TOKEN_KIND_EOF,          TOKEN_KIND_COMMA,      TOKEN_KIND_COLON,
  TOKEN_KIND_SEMICOLON,    TOKEN_KIND_LPAREN,     TOKEN_KIND_RPAREN,
  TOKEN_KIND_LBRACKET,     TOKEN_KIND_RBRACKET,   TOKEN_KIND_LBRACE,
  TOKEN_KIND_RBRACE,       TOKEN_KIND_PIPEEQ,     TOKEN_KIND_PIPEPIPE,
  TOKEN_KIND_PIPE,         TOKEN_KIND_AMPEQ,      TOKEN_KIND_AMPAMP,
  TOKEN_KIND_AMP,          TOKEN_KIND_CARETEQ,    TOKEN_KIND_CARET,
  TOKEN_KIND_EQEQ,         TOKEN_KIND_EQ,         TOKEN_KIND_BANGEQ,
  TOKEN_KIND_BANG,         TOKEN_KIND_TILDE,      TOKEN_KIND_LE,
  TOKEN_KIND_LTLTEQ,       TOKEN_KIND_LTLT,       TOKEN_KIND_LT,
  TOKEN_KIND_GE,           TOKEN_KIND_GTGTEQ,     TOKEN_KIND_GTGT,
  TOKEN_KIND_GT,           TOKEN_KIND_DOTDOT,     TOKEN_KIND_DOT,
  TOKEN_KIND_PLUSEQ,       TOKEN_KIND_PLUS,       TOKEN_KIND_MINUSEQ,
  TOKEN_KIND_MINUS,        TOKEN_KIND_STAREQ,     TOKEN_KIND_STAR,
  TOKEN_KIND_SLASHEQ,      TOKEN_KIND_SLASH,      TOKEN_KIND_PERCENTEQ,
  TOKEN_KIND_PERCENT,      TOKEN_KIND_INT,        TOKEN_KIND_FLOAT,
  TOKEN_KIND_CHAR,         TOKEN_KIND_STRING,     TOKEN_KIND_AS_KW,
  TOKEN_KIND_BREAK_KW,     TOKEN_KIND_CASE_KW,    TOKEN_KIND_CONST_KW,
  TOKEN_KIND_CONTINUE_KW,  TOKEN_KIND_DEFAULT_KW, TOKEN_KIND_DO_KW,
  TOKEN_KIND_ELSE_KW,      TOKEN_KIND_FALSE_KW,   TOKEN_KIND_FN_KW,
  TOKEN_KIND_FOR_KW,       TOKEN_KIND_IF_KW,      TOKEN_KIND_IMPORT_KW,
  TOKEN_KIND_IN_KW,        TOKEN_KIND_INOUT_KW,   TOKEN_KIND_INTERFACE_KW,
  TOKEN_KIND_NEW_KW,       TOKEN_KIND_RETURN_KW,  TOKEN_KIND_STRUCT_KW,
  TOKEN_KIND_SWITCH_KW,    TOKEN_KIND_TRUE_KW,    TOKEN_KIND_TRY_KW,
  TOKEN_KIND_TYPEALIAS_KW, TOKEN_KIND_VAR_KW,     TOKEN_KIND_VOID_KW,
  TOKEN_KIND_WHILE_KW,     TOKEN_KIND_IDENT
} TokenKind;

typedef struct
{
  TokenKind kind;
  int       ln;
  int       col;
  int       length;
  char      *chars;
} Token;

typedef struct
{
  char  *file;
  char  *source;
  char  *curr;
  int   ln;
  int   col;
  Token token;
} Lexer;

const char *token_kind_name(TokenKind kind);
void lexer_init(Lexer *lex, char *file, char *source);
void lexer_next(Lexer *lex);

#endif // LEXER_H
