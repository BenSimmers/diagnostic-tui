#include "display.h"
#include <ncurses.h>
#include <unistd.h>

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);

    init_colors();
    WINDOW *win = newwin(0, 0, 0, 0);
    box(win, 0, 0);

    while (1) {
        update_window(win);

        if (wgetch(win) == 'q') {
            break;
        }

        usleep(500000);
    }

    delwin(win);
    endwin();

    return 0;
}
