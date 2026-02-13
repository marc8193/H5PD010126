#ifndef ALLOCATOR_H
#define ALLOCATOR_H

typedef struct {
  unsigned char* buffer;
  size_t buffer_length;
  size_t previous_offset;
  size_t current_offset;
} Arena;

void* arena_allocate(Arena* arena, size_t size);
void arena_init(Arena* arena, void* buffer, size_t buffer_length);
void arena_free(Arena* arena);

#endif
