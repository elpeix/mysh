// test_history.c
#include "../src/history.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_add_to_history_basic() {
  free_history();
  add_to_history("primera");
  add_to_history("segona");
  assert(history_len == 2);
  assert(strcmp(history[0], "primera") == 0);
  assert(strcmp(history[1], "segona") == 0);
}

void test_add_to_history_overflow() {
  free_history();
  for (int i = 0; i < MAX_HIST + 5; i++) {
    char buf[32];
    snprintf(buf, sizeof(buf), "cmd%d", i);
    add_to_history(buf);
  }
  assert(history_len == MAX_HIST);
  assert(strcmp(history[0], "cmd5") == 0);
}

void test_no_duplicate_consecutive() {
  free_history();
  add_to_history("hola");
  add_to_history("hola");
  assert(history_len == 1);
  add_to_history("adeu");
  assert(history_len == 2);
}

void test_save_and_load_history() {
  free_history();
  add_to_history("foo");
  add_to_history("bar");
  save_history();
  free_history();
  load_history();
  assert(history_len >= 2);
  assert(strcmp(history[history_len - 2], "foo") == 0);
  assert(strcmp(history[history_len - 1], "bar") == 0);
}

int main() {
  test_add_to_history_basic();
  test_add_to_history_overflow();
  test_no_duplicate_consecutive();
  test_save_and_load_history();
  printf("All history tests passed!\n");
  return 0;
}
