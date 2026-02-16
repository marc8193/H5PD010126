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

/*
  Arena allocator
*/

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

/*
  Stack allocator
*/

size_t calculate_padding_with_header(uintptr_t ptr, uintptr_t alignment, size_t header_size) {
  uintptr_t p, a, modulo, padding, needed_space;

  assert(is_power_of_two(alignment));

  p = ptr;
  a = alignment;
  modulo = p & (a - 1);
  padding = 0;

  if (modulo != 0) {
	padding = a - modulo;
  }

  needed_space = (uintptr_t)header_size;

  if (padding < needed_space) {
	needed_space -= padding;

	if ((needed_space & (a - 1)) != 0) {
	  padding += a * (1 + (needed_space / a));
	} else {
	  padding += a * (needed_space / a);
	}
  }

  return (size_t)padding;
}

typedef struct {
  uint8_t padding;
} Stack_Allocation_Header;

void* stack_allocate_aligned(Stack* stack, size_t size, size_t alignment) {
  uintptr_t current_address, next_address;
  size_t padding;
  Stack_Allocation_Header* header;

  assert(is_power_of_two(alignment));

  if (alignment > 128) {
	alignment = 128;
  }

  current_address = (uintptr_t)stack->buffer + (uintptr_t)stack->offset;
  padding = calculate_padding_with_header(current_address, (uintptr_t)alignment,
										  sizeof(Stack_Allocation_Header));

  if (stack->offset + padding + size > stack->buffer_length) {
	return NULL;
  }

  stack->offset += padding;

  next_address = current_address + (uintptr_t)padding;
  header = (Stack_Allocation_Header*)
	(next_address - sizeof(Stack_Allocation_Header));

  header->padding = (uint8_t)padding;

  stack->offset += size;

  return memset((void*)next_address, 0, size);
}

void* stack_allocate(Stack* stack, size_t size) {
  return stack_allocate_aligned(stack, size, 2);
}

void stack_free(Stack* stack, void* ptr) {
  uintptr_t start, end, current_address;
  size_t previous_offset;
  Stack_Allocation_Header* header;

  if (ptr == NULL) {
	return;
  }

  start = (uintptr_t)stack->buffer;
  end = start + (uintptr_t)stack->buffer_length;
  current_address = (uintptr_t)ptr;

  if (!(start <= current_address && current_address < end)) {
	assert(0 && "Out of bounds memory address passed to stack allocator (free)");
	return;
  }

  if (current_address >= start + (uintptr_t)stack->offset) {
	return;
  }

  header = (Stack_Allocation_Header*)
	(current_address - sizeof(Stack_Allocation_Header));

  previous_offset = (size_t)
	(current_address - (uintptr_t)header->padding - start);

  stack->offset = previous_offset;
}

void stack_free_all(Stack* stack) {
  stack->offset = 0;
}
  
void stack_init(Stack* stack, void* buffer, size_t buffer_length) {
  stack->buffer = (unsigned char*)buffer;
  stack->buffer_length = buffer_length;
  stack->offset = 0;
}
