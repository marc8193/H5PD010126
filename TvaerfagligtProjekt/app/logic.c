#include <stddef.h>
#include <string.h>

#include <logic.h>

void note_init(Note* note) {
  memset(note->buffer, 0, sizeof(note->buffer));

  note->previous = NULL;
  note->next = NULL;
}
