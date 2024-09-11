#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <ncurses.h>

void print_cpu_usage(WINDOW *win, int start_y, int start_x);
void print_memory_usage(WINDOW *win, int start_y, int start_x);
void print_disk_usage(WINDOW *win, int start_y, int start_x);
void print_network_usage(WINDOW *win, int start_y, int start_x);
void print_system_uptime(WINDOW *win, int start_y, int start_x);
void print_uname(WINDOW *win, int start_y, int start_x);
void print_processes_and_threads(WINDOW *win, int start_y, int start_x);
void print_battery_life(WINDOW *win, int start_y, int start_x);
double get_cpu_usage();
void draw_cpu_usage_bar(WINDOW *win, int start_y, int start_x, int width);
double get_battery_life();
void draw_battery_life_bar(WINDOW *win, int start_y, int start_x, int width);

#endif // SYSTEM_INFO_H
