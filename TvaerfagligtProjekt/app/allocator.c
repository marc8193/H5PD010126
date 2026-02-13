#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <allocator.h>

bool is_power_of_two(uintptr_t ptr) {
  return (ptr & (ptr - 1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
  uintptr_t p, a, modulo;

  assert(is_power_of_two(align));

  p = ptr;
  a = (uintptr_t)align;
  modulo = p & (a - 1);

  if (modulo != 0) {
	p += a - modulo;
  }

  return p;
}

void* arena_allocate_aligned(Arena* arena, size_t size, size_t align) {
  uintptr_t current_ptr = (uintptr_t)arena->buffer +
	(uintptr_t)arena->current_offset;

  uintptr_t offset = align_forward(current_ptr, align);
  offset -= (uintptr_t)arena->buffer;

  if (offset + size <= arena->buffer_length) {
	void* ptr = &arena->buffer[offset];
	arena->previous_offset = offset;
	arena->current_offset = offset + size;

	memset(ptr, 0, size);

	return ptr;
  }

  return NULL;
}

void* arena_allocate(Arena* arena, size_t size) {
  return arena_allocate_aligned(arena, size, 2 * sizeof(void*));
}

void arena_init(Arena* arena, void* buffer, size_t buffer_length) {
  arena->buffer = (unsigned char*)buffer;
  arena->buffer_length = buffer_length;
  arena->previous_offset = 0;
  arena->current_offset = 0;
}

void arena_free(Arena* arena) {
  arena->previous_offset = 0;
  arena->current_offset = 0;
}
