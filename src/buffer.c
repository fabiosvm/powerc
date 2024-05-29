//
// buffer.c
// 
// Copyright 2024 The PowerC Authors and Contributors.
// 
// This file is part of the PowerC Project.
// For detailed license information, please refer to the LICENSE file
// located in the root directory of this project.
//

#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void buffer_init(Buffer *buf)
{
  size_t capacity = BUFFER_MIN_CAPACITY;
  char *data = malloc(capacity);
  buf->capacity = capacity;
  buf->count = 0;
  buf->data = data;
}

void buffer_init_with_capacity(Buffer *buf, size_t capacity)
{
  size_t realCapacity = BUFFER_MIN_CAPACITY;
  while (realCapacity < capacity)
    realCapacity <<= 1;
  char *data = malloc(realCapacity);
  buf->capacity = realCapacity;
  buf->count = 0;
  buf->data = data;
}

void buffer_ensure_capacity(Buffer *buf, size_t capacity)
{
  if (capacity <= buf->capacity) return;
  size_t newCapacity = buf->capacity;
  while (newCapacity < capacity)
    newCapacity <<= 1;
  char *newData = realloc(buf->data, newCapacity);
  buf->capacity = newCapacity;
  buf->data = newData;
}

void buffer_write(Buffer *buf, size_t count, void *ptr)
{
  buffer_ensure_capacity(buf, buf->count + count);
  memcpy(&buf->data[buf->count], ptr, count);
  buf->count += count;
}
