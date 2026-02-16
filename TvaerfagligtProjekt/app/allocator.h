#ifndef ALLOCATOR_H
#define ALLOCATOR_H

/*
  Arena allocator
*/

typedef struct {
  unsigned char* buffer;
  size_t buffer_length;
  size_t previous_offset;
  size_t current_offset;
} Arena;

void* arena_allocate(Arena* arena, size_t size);
void arena_init(Arena* arena, void* buffer, size_t buffer_length);
void arena_free(Arena* arena);

/*
  Stack allocator
*/

typedef struct {
  unsigned char* buffer;
  size_t buffer_length;
  size_t offset;
} Stack;

void* stack_allocate(Stack* stack, size_t size);
void stack_init(Stack* stack, void* buffer, size_t buffer_length);
void stack_free(Stack* stack, void* ptr);
void stack_free_all(Stack* stack);

#endif
