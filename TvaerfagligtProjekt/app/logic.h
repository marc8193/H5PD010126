#ifndef LOGIC_H
#define LOGIC_H

typedef struct {
  char buffer[256];
  struct Note* previous;
  struct Note* next;
} Note;

void note_init(Note* note);

#endif
