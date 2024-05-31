//
// parser.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"

typedef struct
{
  Lexer lex;
} Parser;

void parser_init(Parser *parser, char *file, char *source);
AstNode *parser_parse(Parser *parser);

#endif // PARSER_H
