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
#include "codegen.h"
#include "parser.h"

static inline void print_usage(char *cmd);
static inline void ensure_directory(char *dir);
static inline void load_from_file(Buffer *buf, char *file);
static inline void save_to_file(Buffer *buf, char *file);
static inline FILE *open_file(char *file, char *mode);
static inline size_t file_size(FILE *fp);
static inline void compile_source(char *input, char *output);
static inline void link_objects(char *output);
static inline void run_binary(char *file);

static inline void print_usage(char *cmd)
{
  printf("\nUsage: %s <input-file>\n", cmd);
}

static inline void ensure_directory(char *dir)
{
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", dir);
  system(cmd);
}

static inline void load_from_file(Buffer *buf, char *file)
{
  FILE *fp = open_file(file, "r");
  size_t size = file_size(fp);
  size_t count = size + 1;
  buffer_init_with_capacity(buf, count);
  buf->count = count;
  memset(buf->data, 0, count);
  fread(buf->data, 1, size, fp);
  fclose(fp);
}

static inline void save_to_file(Buffer *buf, char *file)
{
  FILE *fp = open_file(file, "w");
  fwrite(buf->data, 1, buf->count, fp);
  fclose(fp);
}

static inline FILE *open_file(char *file, char *mode)
{
  FILE *fp = NULL;
#ifdef _WIN32
  fopen_s(&fp, file, mode);
#else
  fp = fopen(file, mode);
#endif
  if (!fp)
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

static inline void compile_source(char *input, char *output)
{
  char cmd[1024];
  char *flags = "-Iinclude";
  sprintf(cmd, "gcc -c %s -o %s.o %s", input, output, flags);
  system(cmd);
}

static inline void link_objects(char *output)
{
  char cmd[1024];
  sprintf(cmd, "gcc target/*.o -o %s", output);
  system(cmd);
}

static inline void run_binary(char *file)
{
  system(file);
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
  Buffer source;
  load_from_file(&source, file);

  Parser parser;
  parser_init(&parser, file, source.data);
  AstNode *ast = parser_parse(&parser);
  ast_print(ast);

  Buffer code;
  buffer_init(&code);
  generate(ast, &code);

  char *input = "target/out.c";
  char *output = "target/out";

  ensure_directory("target");
  save_to_file(&code, input);
  compile_source(input, output);
  compile_source("src/runtime.c", "target/runtime");
  link_objects(output);
  run_binary(output);

  return EXIT_SUCCESS;
}
