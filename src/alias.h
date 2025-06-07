#ifndef ALIAS_H
#define ALIAS_H

void load_aliases();
char *list_aliases();
char *substitute_alias(const char *command);
void free_aliases();

#endif
