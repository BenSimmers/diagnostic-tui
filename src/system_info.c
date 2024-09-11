#include "system_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#include <net/if.h>
#elif __linux__
#include <sys/sysinfo.h>
#include <string.h>
#include <sys/statfs.h>
#include <unistd.h>
#endif

// Cross-platform CPU usage
void print_cpu_usage(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t status = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count);

    if (status == KERN_SUCCESS)
    {
        long total_ticks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++)
        {
            total_ticks += cpuinfo.cpu_ticks[i];
        }

        double idle_ticks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
        double cpu_usage = 100.0 * (1.0 - (idle_ticks / total_ticks));

        int color_pair = (cpu_usage > 75.0) ? 1 : 2;
        wattron(win, COLOR_PAIR(color_pair));
        mvwprintw(win, start_y, start_x, "CPU Usage: %.2f%%", cpu_usage);
        wattroff(win, COLOR_PAIR(color_pair));
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get CPU usage");
    }
#elif __linux__

    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        double total = info.totalram;
        total *= info.mem_unit;
        double free = info.freeram;
        free *= info.mem_unit;
        double used = total - free;

        double cpu_usage = 100.0 * (1.0 - ((double)info.freeram / (double)info.totalram));
        int color_pair = (cpu_usage > 75.0) ? 1 : 2;
        wattron(win, COLOR_PAIR(color_pair));
        mvwprintw(win, start_y, start_x, "CPU Usage: %.2f%%", cpu_usage);
        wattroff(win, COLOR_PAIR(color_pair));
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get CPU usage");
    }

#else
    mvwprintw(win, start_y, start_x, "CPU Usage: Unsupported platform");

#endif
}

void print_memory_usage(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    int64_t total_memory;
    size_t length = sizeof(total_memory);
    sysctlbyname("hw.memsize", &total_memory, &length, NULL, 0);

    mach_port_t host_port = mach_host_self();
    vm_size_t page_size;
    vm_statistics_data_t vm_stat;
    mach_msg_type_number_t host_size = sizeof(vm_stat) / sizeof(integer_t);

    host_page_size(host_port, &page_size);
    host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size);

    int64_t free_memory = vm_stat.free_count * page_size;
    int64_t used_memory = total_memory - free_memory;

    mvwprintw(win, start_y, start_x, "Memory Usage: %llu MB used / %llu MB free",
              used_memory / (1024 * 1024), free_memory / (1024 * 1024));

#elif __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        double total = info.totalram;
        total *= info.mem_unit;
        double free = info.freeram;
        free *= info.mem_unit;
        double used = total - free;

        mvwprintw(win, start_y, start_x, "Memory Usage: %.2f MB used / %.2f MB free",
                  used / (1024 * 1024), free / (1024 * 1024));
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get memory usage");
    }

#else
    mvwprintw(win, start_y, start_x, "Memory Usage: Unsupported platform");
#endif
}

void print_disk_usage(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    FILE *fp = popen("df -h / | tail -n 1 | awk '{print $3, $4}'", "r");
    if (fp != NULL)
    {
        unsigned long long used, free;
        fscanf(fp, "%llu %llu", &used, &free);
        pclose(fp);

        mvwprintw(win, start_y, start_x, "Disk Usage: %llu MB used / %llu MB free",
                  used / (1024 * 1024), free / (1024 * 1024));
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get disk usage");
    }

#elif __linux__
    struct statfs stats;
    if (statfs("/", &stats) == 0)
    {
        unsigned long long total = stats.f_blocks * stats.f_bsize;
        unsigned long long free = stats.f_bfree * stats.f_bsize;
        unsigned long long used = total - free;

        mvwprintw(win, start_y, start_x, "Disk Usage: %llu MB used / %llu MB free",
                  used / (1024 * 1024), free / (1024 * 1024));
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get disk usage");
    }

#else
    mvwprintw(win, start_y, start_x, "Disk Usage: Unsupported platform");
#endif
}

void print_network_usage(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        mvwprintw(win, start_y, start_x, "Failed to get network usage");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_LINK)
        {
            struct if_data *if_data = (struct if_data *)ifa->ifa_data;
            mvwprintw(win, start_y, start_x, "Network Usage: %llu bytes sent / %llu bytes received",
                      if_data->ifi_obytes, if_data->ifi_ibytes);
            break;
        }
    }

    freeifaddrs(ifaddr);

#elif __linux__
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        mvwprintw(win, start_y, start_x, "Failed to get network usage");
        return;
    }

    char buffer[256];
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (strstr(line, "eth0") != NULL)
        {
            unsigned long long rx_bytes, tx_bytes;
            sscanf(line + 6, "%llu %*u %*u %*u %*u %*u %*u %*u %*u %llu", &rx_bytes, &tx_bytes);
            mvwprintw(win, start_y, start_x, "Network Usage: %llu bytes sent / %llu bytes received", tx_bytes, rx_bytes);
            break;
        }
    }

    fclose(fp);
    if (line)
        free(line);

#else
    mvwprintw(win, start_y, start_x, "Network Usage: Unsupported platform");
#endif
}

void print_system_uptime(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    struct timespec ts;
    size_t len = sizeof(ts);
    sysctlbyname("kern.boottime", &ts, &len, NULL, 0);

    time_t boot_time = ts.tv_sec;
    time_t current_time = time(NULL);
    time_t uptime = current_time - boot_time;

    int days = uptime / (60 * 60 * 24);
    uptime %= (60 * 60 * 24);
    int hours = uptime / (60 * 60);
    uptime %= (60 * 60);
    int minutes = uptime / 60;

    mvwprintw(win, start_y, start_x, "System Uptime: %d days, %d hours, %d minutes", days, hours, minutes);
#elif __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        int days = info.uptime / (60 * 60 * 24);
        int hours = (info.uptime % (60 * 60 * 24)) / (60 * 60);
        int minutes = (info.uptime % (60 * 60)) / 60;

        mvwprintw(win, start_y, start_x, "System Uptime: %d days, %d hours, %d minutes", days, hours, minutes);
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get system uptime");
    }
#else
    mvwprintw(win, start_y, start_x, "System Uptime: Unsupported platform");
#endif
}

void print_uname(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        mvwprintw(win, start_y, start_x, "System: %s %s", uts.sysname, uts.release);
        mvwprintw(win, start_y + 1, start_x, "Version: %s", uts.version);
        mvwprintw(win, start_y + 2, start_x, "Machine: %s", uts.machine);
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get system information");
    }

#elif __linux__
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        mvwprintw(win, start_y, start_x, "System: %s %s", uts.sysname, uts.release);
        mvwprintw(win, start_y + 1, start_x, "Version: %s", uts.version);
        mvwprintw(win, start_y + 2, start_x, "Machine: %s", uts.machine);
    }
    else
    {
        mvwprintw(win, start_y, start_x, "Failed to get system information");
    }

#else
    mvwprintw(win, start_y, start_x, "System: Unsupported platform");
#endif
}

void print_processes_and_threads(WINDOW *win, int start_y, int start_x)
{
    int processes = 0;
    int threads = 0;

    FILE *fp = popen("ps -A | wc -l", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &processes);
        pclose(fp);
    }

    FILE *fp2 = popen("ps -A -L | wc -l", "r");
    if (fp2 != NULL)
    {
        fscanf(fp2, "%d", &threads);
        pclose(fp2);
    }

    mvwprintw(win, start_y, start_x, "Processes: %d", processes);
    mvwprintw(win, start_y + 1, start_x, "Threads: %d", threads);
}

void print_battery_life(WINDOW *win, int start_y, int start_x)
{
#ifdef __APPLE__
    int battery_life = 0;
    FILE *fp = popen("pmset -g batt | grep -o '[0-9]*%'", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &battery_life);
        pclose(fp);
    }

    mvwprintw(win, start_y, start_x, "Battery Life: %d%%", battery_life);

#elif __linux__
    mvwprintw(win, start_y, start_x, "Battery Life: 0");
#endif
}

double get_cpu_usage()
{
#ifdef __APPLE__
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t status = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count);

    if (status == KERN_SUCCESS)
    {
        long total_ticks = 0;
        for (int i = 0; i < CPU_STATE_MAX; i++)
        {
            total_ticks += cpuinfo.cpu_ticks[i];
        }

        double idle_ticks = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
        return 100.0 * (1.0 - (idle_ticks / total_ticks));
    }
    else
    {
        return 0.0; // Handle error appropriately
    }

#elif __linux__
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
        return 100.0 * (1.0 - ((double)info.freeram / (double)info.totalram));
    }
    else
    {
        return 0.0; // Handle error appropriately
    }

#else
    return 0.0; // Unsupported platform
#endif
}

double get_battery_life()
{
//     int battery_life = 0;
//     FILE *fp = popen("pmset -g batt | grep -o '[0-9]*%'", "r");
//     if (fp != NULL) {
//         fscanf(fp, "%d", &battery_life);
//         pclose(fp);
//     }

//     return battery_life;
#ifdef __APPLE__
    int battery_life = 0;
    FILE *fp = popen("pmset -g batt | grep -o '[0-9]*%'", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &battery_life);
        pclose(fp);
    }

    return battery_life;

#elif __linux__
    return 0;
#endif
}

void draw_battery_life_bar(WINDOW *win, int start_y, int start_x, int width)
{
#ifdef __APPLE__
    double battery_life = get_battery_life();
    int filled_length = (battery_life / 100.0) * width;

    int color_pair;
    if (battery_life > 50.0)
    {
        color_pair = 1; // Green
    }
    else if (battery_life > 25.0)
    {
        color_pair = 2; // Yellow
    }
    else
    {
        color_pair = 3; // Red
    }

    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, start_y, start_x, "[");
    for (int i = 0; i < filled_length; ++i)
    {
        mvwaddch(win, start_y, start_x + 1 + i, ACS_BLOCK);
    }
    mvwprintw(win, start_y, start_x + width + 1, "] %d%%", (int)battery_life);
    wattroff(win, COLOR_PAIR(color_pair));

#elif __linux__
    mvwprintw(win, start_y, start_x, "not supported on Linux");
#endif
}

void draw_cpu_usage_bar(WINDOW *win, int start_y, int start_x, int width)
{
#ifdef __APPLE__
    double cpu_usage = get_cpu_usage();
    int filled_length = (cpu_usage / 100.0) * width;

    int color_pair;
    if (cpu_usage < 50.0)
    {
        color_pair = 1; // Green
    }
    else if (cpu_usage < 75.0)
    {
        color_pair = 2; // Yellow
    }
    else
    {
        color_pair = 3; // Red
    }

    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, start_y, start_x, "[");
    for (int i = 0; i < filled_length; ++i)
    {
        mvwaddch(win, start_y, start_x + 1 + i, ACS_BLOCK);
    }
    mvwprintw(win, start_y, start_x + width + 1, "] %.2f%%", cpu_usage);
    wattroff(win, COLOR_PAIR(color_pair));
#elif __linux__
    double cpu_usage = get_cpu_usage();
    int filled_length = (cpu_usage / 100.0) * width;

    int color_pair;
    if (cpu_usage < 50.0)
    {
        color_pair = 1; // Green
    }
    else if (cpu_usage < 75.0)
    {
        color_pair = 2; // Yellow
    }
    else
    {
        color_pair = 3; // Red
    }

    wattron(win, COLOR_PAIR(color_pair));
    mvwprintw(win, start_y, start_x, "[");
    for (int i = 0; i < filled_length; ++i)
    {
        mvwaddch(win, start_y, start_x + 1 + i, ACS_BLOCK);
    }
    mvwprintw(win, start_y, start_x + width + 1, "] %.2f%%", cpu_usage);
    wattroff(win, COLOR_PAIR(color_pair));
#else
    mvwprintw(win, start_y, start_x, "not supported on Linux");
#endif
}
