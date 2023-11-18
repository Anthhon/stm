#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "termui.h"
#include "static.h"

void termuiInit(terminalInfo *terminal)
{
        getPosition(terminal);
        getTermSize(terminal);
        terminal->initialized = true;
        system("clear");
}

// Stores the cursor position
void getPosition(terminalInfo *terminal)
{
        int t = STDOUT_FILENO;
        struct termios old_terminal_settings;

        // Get the current terminal settings
        if (tcgetattr(t, &old_terminal_settings) == -1){
                (void)fprintf(stderr, "ERROR: failed to get terminal settings");
                exit(EXIT_FAILURE);
        }

        // Disable echoing and canonical mode
        struct termios new_terminal_settings = old_terminal_settings;
        new_terminal_settings.c_lflag &= ~(ECHO | ICANON);

        // Apply the modified terminal settings
        if (tcsetattr(t, TCSANOW, &new_terminal_settings) == -1){
                (void)fprintf(stderr, "ERROR: failed to change terminal settings");
                exit(EXIT_FAILURE);
        }

        // Request cursor position
        printf("\033[6n"); 
        fflush(stdout);

        // Store cursor position
        (void)scanf("\033[%d;%dR", &terminal->y_cursor_coord, &terminal->x_cursor_coord);
        terminal->x_cursor_coord--; // Corrects offset
        terminal->y_cursor_coord--; // Corrects offset

        // Restore the original terminal settings
        if (tcsetattr(t, TCSANOW, &old_terminal_settings) == -1){
                (void)fprintf(stderr, "ERROR: failed to restore terminal settings");
                exit(EXIT_FAILURE);
        }
}

// Retrieves the window size of the terminal
void getTermSize(terminalInfo *terminal)
{
        struct winsize size;

        if (ioctl(0, TIOCGWINSZ, &size) == -1){
                (void)fprintf(stderr, "ERROR: ioctl call failed while getting terminal size");
                exit(EXIT_FAILURE);
        }; 

        // Uses ioctl system call to update terminal size values
        terminal->x_term_size = size.ws_col;
        terminal->y_term_size = size.ws_row;
}
