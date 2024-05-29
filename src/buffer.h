//
// buffer.h
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

#define BUFFER_MIN_CAPACITY (1 << 3)

#define buffer_is_empty(b) (!(b)->count)

#define buffer_clear(b) \
  do { \
    (b)->count = 0; \
  } while (0)

typedef struct
{
  size_t capacity;
  size_t count;
  char   *data;
} Buffer;

void buffer_init(Buffer *buf);
void buffer_init_with_capacity(Buffer *buf, size_t capacity);
void buffer_ensure_capacity(Buffer *buf, size_t capacity);
void buffer_write(Buffer *buf, size_t count, void *ptr);

#endif // BUFFER_H
