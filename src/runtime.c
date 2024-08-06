//
// runtime.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

String string_from_cstr(const char *cstr)
{
  size_t len = strlen(cstr);
  String str = malloc(len + 1);
  memcpy(str, cstr, len);
  str[len] = '\0';
  return str;
}

String string_from_int(Int num)
{
  char buf[16];
  sprintf(buf, "%d", num);
  return string_from_cstr(buf);
}

String string_from_float(Float num)
{
  char buf[32];
  sprintf(buf, "%g", num);
  return string_from_cstr(buf);
}

void println(String str)
{
  printf("%s\n", str);
}
