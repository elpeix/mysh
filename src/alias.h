#ifndef ALIAS_H
#define ALIAS_H

void load_aliases();
char *list_aliases();
int subsitute_command(char ***args_ptr);
void free_aliases();

#endif
