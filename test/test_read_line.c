// test_read_line.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "read_line.h"
#include "constants.h"

// Expose these for testing
extern char *history[MAX_HIST];
extern int history_len;
extern void add_to_hisstory(const char *input);
extern void clear_line(int len);

void test_add_to_hisstory_basic() {
  // Reset history
  for (int i = 0; i < MAX_HIST; i++) {
    if (history[i]) free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;

  add_to_hisstory("first");
  assert(history_len == 1);
  assert(strcmp(history[0], "first") == 0);

  add_to_hisstory("second");
  assert(history_len == 2);
  assert(strcmp(history[1], "second") == 0);

  // Clean up
  for (int i = 0; i < history_len; i++) free(history[i]);
}

void test_add_to_hisstory_overflow() {
  // Reset history
  for (int i = 0; i < MAX_HIST; i++) {
    if (history[i]) free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;

  // Fill history
  char buf[32];
  for (int i = 0; i < MAX_HIST; i++) {
    snprintf(buf, sizeof(buf), "cmd%d", i);
    add_to_hisstory(buf);
  }
  assert(history_len == MAX_HIST);
  for (int i = 0; i < MAX_HIST; i++) {
    snprintf(buf, sizeof(buf), "cmd%d", i);
    assert(strcmp(history[i], buf) == 0);
  }

  // Add one more, should evict the first
  add_to_hisstory("overflow");
  assert(history_len == MAX_HIST);
  assert(strcmp(history[0], "cmd1") == 0);
  assert(strcmp(history[MAX_HIST-1], "overflow") == 0);

  // Clean up
  for (int i = 0; i < history_len; i++) free(history[i]);
}

void test_clear_line() {
  // Redirect stdout to a buffer
  FILE *orig = stdout;
  FILE *tmp = tmpfile();
  stdout = tmp;

  clear_line(5);

  fflush(stdout);
  fseek(tmp, 0, SEEK_SET);
  char buf[32] = {0};
  fread(buf, 1, sizeof(buf)-1, tmp);

  // Each clear prints "\b \b" (3 chars), 5 times = 15 chars
  int count = 0;
  for (int i = 0; buf[i]; i++) count++;
  assert(count == 15);

  fclose(tmp);
  stdout = orig;
}

int main() {
  test_add_to_hisstory_basic();
  test_add_to_hisstory_overflow();
  test_clear_line();
  printf("All tests passed!\n");
  return 0;
}
void test_add_to_hisstory_empty_string() {
  // Reset history
  for (int i = 0; i < MAX_HIST; i++) {
    if (history[i]) free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;

  add_to_hisstory("");
  assert(history_len == 1);
  assert(strcmp(history[0], "") == 0);

  // Clean up
  for (int i = 0; i < history_len; i++) {
    free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;
}

void test_add_to_hisstory_long_string() {
  // Reset history
  for (int i = 0; i < MAX_HIST; i++) {
    if (history[i]) free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;

  char long_input[MAX_LINE * 2];
  memset(long_input, 'a', sizeof(long_input) - 1);
  long_input[sizeof(long_input) - 1] = '\0';

  add_to_hisstory(long_input);
  assert(history_len == 1);
  assert(strcmp(history[0], long_input) == 0);

  // Clean up
  for (int i = 0; i < history_len; i++) {
    free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;
}

void test_add_to_hisstory_multiple_evictions() {
  // Reset history
  for (int i = 0; i < MAX_HIST; i++) {
    if (history[i]) free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;

  // Add 2*MAX_HIST entries
  char buf[32];
  for (int i = 0; i < 2 * MAX_HIST; i++) {
    snprintf(buf, sizeof(buf), "cmd%d", i);
    add_to_hisstory(buf);
  }
  assert(history_len == MAX_HIST);
  // The first entry should be "cmd%d", where d = MAX_HIST
  snprintf(buf, sizeof(buf), "cmd%d", MAX_HIST);
  assert(strcmp(history[0], buf) == 0);

  // Clean up
  for (int i = 0; i < history_len; i++) {
    free(history[i]);
    history[i] = NULL;
  }
  history_len = 0;
}
