#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <ncurses.h>

void print_cpu_usage(WINDOW *win);
void print_memory_usage(WINDOW *win);
void print_disk_usage(WINDOW *win);
void print_network_usage(WINDOW *win);
void print_system_uptime(WINDOW *win);
void print_uname(WINDOW *win);
void print_processes_and_threads(WINDOW *win);
void print_battery_life(WINDOW *win);

#endif // SYSTEM_INFO_H
