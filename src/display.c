#include "display.h"
#include "system_info.h"
#include <ncurses.h>

void init_colors() {
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
}

void update_window(WINDOW *win) {
    werase(win);
    box(win, 0, 0);

    print_battery_life(win);
    print_cpu_usage(win);
    print_uname(win);
    print_memory_usage(win);
    print_disk_usage(win);
    print_network_usage(win);
    print_system_uptime(win);
    print_processes_and_threads(win);

    wrefresh(win);
}
