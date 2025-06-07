#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

int autocomplete_in_first_word(char *line, int pos);
int autocomplete_exec(char *line, int pos);
int autocomplete_path(char *line, int pos);

#endif
