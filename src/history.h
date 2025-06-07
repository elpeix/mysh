#ifndef HISTORY_H
#define HISTORY_H

#include "constants.h"

extern char *history[MAX_HIST];
extern int history_len;
extern int history_pos;

void add_to_history(const char *input);
void free_history(void);
void save_history(void);
void load_history(void);
void print_history(void);

#endif
