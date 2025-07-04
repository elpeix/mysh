#include "read_line.h"
#include "autocomplete.h"
#include "constants.h"
#include "history.h"
#include "prompt.h"
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

volatile sig_atomic_t ctrl_c_pressed = 0;
struct termios term_original;

void handle_sigint(int sig) {
  (void)(sig);
  ctrl_c_pressed = 1; // Set flag to indicate Ctrl+C was pressed
}

void active_no_canonical_mode() {
  struct termios term_new;
  tcgetattr(STDIN_FILENO, &term_original);
  term_new = term_original;
  term_new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term_new);
}

void restore_terminal() { tcsetattr(STDIN_FILENO, TCSANOW, &term_original); }

void redraw_line(const char *line, int pos) {
  static int prev_len = 0;
  int len = strlen(line);
  int len_prompt = strlen(get_prompt());

  // Move the cursor to the beginning of the line
  for (int i = 0; i < prev_len + len_prompt; i++) {
    printf("\b");
  }

  // Clear the line
  for (int i = 0; i < prev_len + len_prompt; i++) {
    printf(" ");
  }

  for (int i = 0; i < prev_len + len_prompt; i++) {
    printf("\b");
  }

  // Redraw the prompt and the line
  printf("%s%s", get_prompt(), line);

  // If the current position is less than the length of the line, move the
  // cursor to the correct position
  if (prev_len > len) {

    for (int i = 0; i < prev_len - len; i++) {
      printf(" ");
    }

    for (int i = 0; i < prev_len - len; i++) {
      printf("\b");
    }
  }

  // Move the cursor to the correct position
  for (int i = 0; i < len - pos; i++) {
    printf("\b");
  }

  fflush(stdout);

  // Update the previous length
  prev_len = len;
}

void clear_line(int len) {
  for (int i = 0; i < len; i++) {
    printf("\b \b"); // Move back and clear the character
  }

  fflush(stdout);
}

char *read_line() {
  char line[MAX_LINE];
  size_t pos;

  // Print the prompt
  printf("%s", get_prompt());
  fflush(stdout); // Ensure the prompt is printed immediately

  // Capture ctrl+C to prevent it from terminating the program
  struct sigaction sa, old_sa;
  sa.sa_handler = handle_sigint;
  sa.sa_flags = 0; // No special flags
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, &old_sa); // Set the new signal handler

  // Activate non-canonical mode
  // This allows us to read input character by character without waiting for a
  // newline and to handle special keys and control characters.
  active_no_canonical_mode();

  // Initialize the line buffer and position
  memset(line, 0, sizeof(line));
  pos = 0;

  // Set history position to the end of the history
  history_pos = history_len;

  // Wait for input
  while (1) {
    if (ctrl_c_pressed) {
      // If Ctrl+C was pressed, empty the line and redraw it
      clear_line(pos);
      line[0] = '\0';
      pos = 0;
      redraw_line(line, pos); // Redraw the line
      ctrl_c_pressed = 0;     // Reset the flag
      continue;
    }

    int c = getchar();

    if (c == 27) { // Escape
      // Handle escape sequences for arrow keys
      int c1 = getchar();
      int c2 = getchar();
      if (c1 == 91) {   // CSI (Control Sequence Introducer)
        if (c2 == 65) { // up arrow
          if (history_pos > 0) {
            history_pos--;
            clear_line(pos);
            strcpy(line, history[history_pos]);
            pos = strlen(line);
            redraw_line(line, pos);
          }

        } else if (c2 == 66) { // down arrow
          if (history_pos < history_len - 1) {
            history_pos++;
            clear_line(pos);
            strcpy(line, history[history_pos]);
            pos = strlen(line);
            redraw_line(line, pos);
          } else if (history_pos == history_len - 1) {
            history_pos++;
            clear_line(pos);
            line[0] = '\0';
            pos = 0;
          }

        } else if (c2 == 67) { // right arrow
          if (pos < strlen(line)) {
            pos++;
            printf("\033[C"); // Move cursor right
            fflush(stdout);
            redraw_line(line, pos);
          }

        } else if (c2 == 68) { // left arrow
          if (pos > 0) {
            pos--;
            printf("\033[D"); // Move cursor left
            fflush(stdout);
            redraw_line(line, pos);
          }

        } else if (c2 == 51) { // Delete key
          int c3 = getchar();
          if (c3 == 126) { // 126 is the Delete key in the escape sequence. LOL
            if (pos < strlen(line)) {
              int len = strlen(line);
              memmove(line + pos, line + pos + 1, len - pos);
              redraw_line(line, pos);
            }
          }

        } else if (c2 == 72) { // Home key
          pos = 0;             // Move cursor to the beginning
          redraw_line(line, pos);

        } else if (c2 == 70) { // End key
          pos = strlen(line);  // Move cursor to the end
          redraw_line(line, pos);
        }
      }
    } else if (c == 23) { // Ctrl+W (delete word)
      restore_terminal();

      while (pos > 0 && (line[pos - 1] == ' ' || line[pos - 1] == '\t')) {
        pos--;
        printf("\b \b");
      }

      while (pos > 0 && line[pos - 1] != ' ' && line[pos - 1] != '\t') {
        pos--;
        printf("\b \b");
      }
      line[pos] = '\0';
      redraw_line(line, pos);
      fflush(stdout);

    } else if (c == 1) { // Ctrl+A (move to beginning of line)

      pos = 0; // Move cursor to the beginning
      redraw_line(line, pos);

    } else if (c == 5) { // Ctrl+E (move to end of line)

      pos = strlen(line); // Move cursor to the end
      redraw_line(line, pos);

    } else if (c == 2) { // Ctrl+B (move cursor left)

      if (pos > 0) {
        pos--;
        printf("\033[D"); // Move cursor left
        fflush(stdout);
        redraw_line(line, pos);
      }

    } else if (c == 6) { // Ctrl+F (move cursor right)

      if (pos < strlen(line)) {
        pos++;
        printf("\033[C"); // Move cursor right
        fflush(stdout);
        redraw_line(line, pos);
      }

    } else if (c == 4) { // Ctrl+D (EOF)

      // EXIT
      // Restore terminal
      restore_terminal();

      // Restore default signal handler for Ctrl+C
      sigaction(SIGINT, &old_sa, NULL);

      return NULL; // Indicate EOF

    } else if (c == 12) { // Ctrl+L (clear screen)

      // Clear the screen and redraw the prompt
      printf("\033[H\033[J"); // Clear screen
      printf("%s", get_prompt());
      fflush(stdout);
      pos = 0;        // Reset position
      line[0] = '\0'; // Clear the line

    } else if (c == 11) { // Ctrl+K (clear to end of line)

      // Clear from the current position to the end of the line
      size_t len = strlen(line);
      if (pos < len) {
        for (size_t i = pos; i < len; i++) {
          printf("\b \b"); // Move back and clear the character
        }
        line[pos] = '\0'; // Null-terminate the string
      }
      redraw_line(line, pos);

    } else if (c == 21) { // Ctrl+U (clear line)

      // Clear the current line
      clear_line(pos);
      pos = 0;
      line[0] = '\0';
      redraw_line(line, pos);

    } else if (c == 9) { // Tab key (autocomplete)

      if (pos == 0) {
        // If the line is empty, just beep
        printf("\a");
        continue;
      }

      // If all characters are spaces, do not autocomplete
      int all_spaces = 1;
      for (size_t i = 0; i < pos; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
          all_spaces = 0;
          break;
        }
      }
      if (all_spaces) {
        printf("\a"); // Beep if all characters are spaces
        continue;
      }

      int new_pos;

      if (autocomplete_in_first_word(line, pos) == 1) { // TODO: first word
        new_pos = autocomplete_exec(line, pos);
      } else {
        new_pos = autocomplete_path(line, pos);
      }

      // If a completion was found, update the position and redraw the line
      if (new_pos > 0) {
        pos = new_pos;
        redraw_line(line, pos);
      } else {
        printf("\a"); // Beep if no completion found
      }

    } else if (c == '\n') { // Enter key

      // go to the end of the line
      while (pos < strlen(line)) {
        printf("\033[C"); // Move cursor right
        pos++;
      }
      fflush(stdout);
      line[pos] = '\0';
      printf("\n");

      // Restore terminal
      restore_terminal();

      // Restore default signal handler for Ctrl+C
      sigaction(SIGINT, &old_sa, NULL);

      char *response = strdup(line);
      if (response == NULL) {
        perror("strdup");
        return NULL; // Return NULL on memory allocation failure
      }
      return response;

    } else if (c == 127 || c == 8) { // Backspace

      if (pos > 0) {
        int len = strlen(line);
        pos--;
        memmove(line + pos, line + pos + 1, len - pos);
        redraw_line(line, pos);
      }

    } else if (pos < MAX_LINE - 1) {

      // Insert character at current position
      size_t len = strlen(line);
      if (pos < len) {
        // Move the rest of the line to the right
        memmove(line + pos + 1, line + pos, len - pos + 1);
        line[pos] = c;
      } else {
        line[pos] = c;
        line[pos + 1] = '\0';
      }
      pos++;
      redraw_line(line, pos);
    }
  }
}
