#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>

void init_colors();
void update_window(WINDOW *win);
void place_content(WINDOW *win, int grid_row, int grid_col, void (*draw_func)(WINDOW *, int, int, int));

#endif // DISPLAY_H
