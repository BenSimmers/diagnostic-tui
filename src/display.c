#include "display.h"
#include "system_info.h"
#include <ncurses.h>

#define BAR_WIDTH 50

const int ROW_HEIGHT = 1;
const int COLUMN_WIDTH = 30;
const int X_OFFSET = 2;
const int Y_OFFSET = 2;

void init_colors()
{
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Green
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Yellow
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Red
}

void place_content(WINDOW *win, int grid_row, int grid_col, void (*draw_func)(WINDOW *, int, int, int))
{
    int x = X_OFFSET + grid_col * COLUMN_WIDTH;
    int y = Y_OFFSET + grid_row * ROW_HEIGHT;
    draw_func(win, y, x, BAR_WIDTH);
}

void update_window(WINDOW *win)
{
    werase(win);
    box(win, 0, 0);

    place_content(win, 0, 0, print_battery_life);
    place_content(win, 0, 1, print_cpu_usage);
    place_content(win, 1, 0, draw_battery_life_bar);
    place_content(win, 2, 0, draw_cpu_usage_bar);
    place_content(win, 3, 0, print_uname);
    place_content(win, 4, 0, print_memory_usage);
    place_content(win, 5, 0, print_disk_usage);
    place_content(win, 6, 0, print_network_usage);
    place_content(win, 7, 0, print_system_uptime);
    place_content(win, 8, 0, print_processes_and_threads);

    wrefresh(win);
}
