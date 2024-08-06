//
// codegen.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "buffer.h"

void generate(AstNode *ast, Buffer *code);

#endif // CODEGEN_H
