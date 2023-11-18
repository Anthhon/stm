#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "termui.h"
#include "static.h"

// Draws a border using the specified character
void drawBorder(terminalInfo *terminal, const char border_char)
{
        int x = terminal->x_term_size;
        int y = terminal->y_term_size;

        // Draw border
        for (int i = 1; i <= x; ++i){
                for (int j = 1; j <= y; ++j){
                        if (j != 1 && j != y && i != 1 && i != x){
                                continue; }
                        MoveTo(i, j);
                        printf("%c", border_char);
                }
        }
        getPosition(terminal); // Updates cursor pos
}

// Creates a box with the specified dimensions and border character
void boxBuild(terminalInfo *terminal, const int x, const int y, const char border_char)
{
        for (int i = 1; i <= x; ++i){
                for (int j = 1; j <= y; ++j){
                        if (j != 1 && j != y && i != 1 && i != x){
                                continue;
                        }
                        MoveTo(i + terminal->x_cursor_coord, j + terminal->y_cursor_coord);
                        printf("%c", border_char);
                }
        }
        getPosition(terminal); // Updates cursor pos
}

// Creates a box with the specified dimensions, border character, and starting coordinates
void boxBuildAt(terminalInfo *terminal, const int x, const int y, const int x_coord, const int y_coord, const char border_char)
{
        for (int i = 1; i <= x; ++i){
                for (int j = 1; j <= y; ++j){
                        if (j != 1 && j != y && i != 1 && i != x){
                                continue;
                        }
                        MoveTo(i + x_coord, j + y_coord);
                        printf("%c", border_char);
                }
        }
        getPosition(terminal); // Updates cursor pos
}

// Creates a text box with a message inside and a border
void boxText(terminalInfo *terminal, char *message, const int offset, const char border_char)
{
        int x = 0;
        const int y = offset * 2 + 3;
        
        // Check minimum 'x' size due to newline characters
        char *tmp_ptr = &message[0];
        while(*tmp_ptr++ != '\0') {
                // Little ternary operation hack with pointer arithmetic to
                // calculate longest sequence before a newline character
                if (*tmp_ptr == '\n') {
                        x = ((int)(tmp_ptr - &message[0]) > x) ? (int)(tmp_ptr - &message[0]) : x;
                }
        }

        // Build box
        getPosition(terminal); // Updates cursor pos
        int last_x = terminal->x_cursor_coord;
        int last_y = terminal->x_cursor_coord;
        boxBuild(terminal, x, y, border_char);

        // Count newlines at message to calculate offset
        tmp_ptr = &message[0];
        int newline_offset = 0;
        while (*tmp_ptr != '\0'){
                if (*tmp_ptr == '\n'){
                        ++newline_offset;
                }
                ++tmp_ptr;
        }
        if (newline_offset != 0) --newline_offset;

        // Print message
        tmp_ptr = &message[0];

        MoveTo(last_x + offset + 2, last_y + offset + 2 - newline_offset);
        getPosition(terminal);
        while(*tmp_ptr != '\0'){
                if (*tmp_ptr == '\n'){
                        MoveTo(terminal->x_cursor_coord + 1, ++terminal->y_cursor_coord);
                        printf("\v"); // Break line
                        ++tmp_ptr;
                        continue;
                }
                printf("%c", *tmp_ptr);
                ++tmp_ptr;
        }
        MoveTo(terminal->x_cursor_coord, terminal->y_cursor_coord);
        printf("\n");
}

// Creates a text box with a message inside, a border, and starting coordinates
void boxTextAt(terminalInfo *terminal, char *message, const int offset, const int x_coord, const int y_coord, const char border_char)
{
        const size_t x = strlen(message) + offset * 2 + 2;
        const size_t y = offset * 2 + 3;

        // Build box
        getPosition(terminal); // Updates cursor pos
        boxBuildAt(terminal, x, y, x_coord, y_coord, border_char);

        // Print message at given coordinate
        MoveTo(x_coord + offset + 2, y_coord + offset + 2);
        printf("%s", message);
        MoveTo(terminal->x_cursor_coord, terminal->y_cursor_coord);
}
