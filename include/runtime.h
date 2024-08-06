//
// runtime.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

typedef void     Void;
typedef bool     Bool;
typedef int8_t   Byte;
typedef int16_t  Short;
typedef int32_t  Int;
typedef int64_t  Long;
typedef uint8_t  UByte;
typedef uint16_t UShort;
typedef uint32_t UInt;
typedef uint64_t ULong;
typedef float    Float;
typedef double   Double;
typedef char     Char;
typedef Char*    String;

String string_from_cstr(const char *cstr);
String string_from_int(Int num);
String string_from_float(Float num);
void println(String str);

#endif // RUNTIME_H
