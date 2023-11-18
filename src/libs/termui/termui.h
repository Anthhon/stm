#ifndef TERMUI_H
#define TERMUI_H

#include <stdbool.h>

#define MoveTo(x, y) (void)printf("\e[%d;%dH", y, x)
#define ClearLine(y) (void)printf("\e[%d;1H", y); \
		     (void)printf("\e[K\r");
//#define GoLeft() MoveTo(--CURSOR_X_COORD, CURSOR_Y_COORD)
//#define GoRight() MoveTo(++CURSOR_X_COORD, CURSOR_Y_COORD)
//#define GoUp() MoveTo(CURSOR_X_COORD, --CURSOR_Y_COORD)
//#define GoDown() MoveTo(CURSOR_X_COORD, ++CURSOR_Y_COORD)

typedef struct terminalInfo {
	bool initialized;
	int x_term_size; // Horizontal window size
	int y_term_size; // Vertical window size
	int x_cursor_coord; // X cursor coordinates
	int y_cursor_coord; // Y cursor coordinates
}terminalInfo;

void termuiInit(terminalInfo *terminal);
void getPosition(terminalInfo *terminal);
void getTermSize(terminalInfo *terminal);

#include "static.h"

#endif /* TERMUI_H */
