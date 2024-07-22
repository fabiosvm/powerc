//
// compiler.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "parser.h"

static inline void print_usage(char *cmd);
static inline void load_file(Buffer *buf, char *file);
static inline FILE *open_file(char *file);
static inline size_t file_size(FILE *fp);

static inline void print_usage(char *cmd)
{
  printf("\nUsage: %s <input-file>\n", cmd);
}

static inline void load_file(Buffer *buf, char *file)
{
  FILE *fp = open_file(file);
  size_t size = file_size(fp);
  size_t count = size + 1;
  buffer_init_with_capacity(buf, count);
  buf->count = count;
  memset(buf->data, 0, count);
  fread(buf->data, 1, size, fp);
  fclose(fp);
}

static inline FILE *open_file(char *file)
{
  FILE *fp = NULL;
#ifdef _WIN32
  fopen_s(&fp, file, "r");
#else
  fp = fopen(file, "r");
#endif
  if (!file)
  {
    fprintf(stderr, "\nERROR: cannot open file %s\n", file);
    exit(EXIT_FAILURE);
  }
  return fp;
}

static inline size_t file_size(FILE *fp)
{
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  return size;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "\nERROR: no input file\n");
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }
  char *file = argv[1];
  Buffer buf;
  load_file(&buf, argv[1]);
  Parser parser;
  parser_init(&parser, file, buf.data);
  AstNode *ast = parser_parse(&parser);
  ast_print(ast);
  return EXIT_SUCCESS;
}
