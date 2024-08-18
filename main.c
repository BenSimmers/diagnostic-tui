#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <time.h>
#include <sys/utsname.h>

void init_colors()
{
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
}

void print_cpu_usage(WINDOW *win)
{
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

        if (cpu_usage > 75.0)
        {
            wattron(win, COLOR_PAIR(1));
        }
        else
        {
            wattron(win, COLOR_PAIR(2));
        }

        mvwprintw(win, 1, 2, "CPU Usage: %.2f%%", cpu_usage);
        wattroff(win, COLOR_PAIR(1));
        wattroff(win, COLOR_PAIR(2));
    }
    else
    {
        mvwprintw(win, 1, 2, "Failed to get CPU usage");
    }
}

void print_memory_usage(WINDOW *win)
{
    int64_t total_memory;
    size_t length = sizeof(total_memory);
    sysctlbyname("hw.memsize", &total_memory, &length, NULL, 0);

    mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t page_size;
    vm_statistics_data_t vm_stat;

    host_page_size(host_port, &page_size);
    host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size);

    int64_t free_memory = vm_stat.free_count * page_size;
    int64_t used_memory = total_memory - free_memory;

    mvwprintw(win, 3, 2, "Memory Usage: %lld MB used / %lld MB total", used_memory / (1024 * 1024), total_memory / (1024 * 1024));
}

void print_disk_usage(WINDOW *win)
{
    struct statfs stats;
    if (statfs("/", &stats) == 0)
    {
        unsigned long long total = stats.f_blocks * stats.f_bsize;
        unsigned long long free = stats.f_bfree * stats.f_bsize;
        unsigned long long used = total - free;

        mvwprintw(win, 5, 2, "Disk Usage: %llu MB used / %llu MB free", used / (1024 * 1024), free / (1024 * 1024));
    }
    else
    {
        mvwprintw(win, 5, 2, "Failed to get disk usage");
    }
}

void print_network_usage(WINDOW *win)
{
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1)
    {
        mvwprintw(win, 7, 2, "Failed to get network usage");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_LINK)
        {
            struct if_data *if_data = (struct if_data *)ifa->ifa_data;
            mvwprintw(win, 7, 2, "Network Usage: %llu bytes sent / %llu bytes received",
                      if_data->ifi_obytes, if_data->ifi_ibytes);
            break;
        }
    }

    freeifaddrs(ifaddr);
}

void print_system_uptime(WINDOW *win)
{
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

    mvwprintw(win, 9, 2, "System Uptime: %d days, %d hours, %d minutes", days, hours, minutes);
}

void print_uname(WINDOW *win)
{
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        mvwprintw(win, 11, 2, "System Name: %s %s, %s", uts.sysname, uts.nodename, uts.release);
        mvwprintw(win, 12, 2, "Version: %s", uts.version);
        mvwprintw(win, 13, 2, "Machine: %s", uts.machine);
    }
    else
    {
        mvwprintw(win, 11, 2, "Failed to get system information");
    }
}

void printProcessesAndThreads(WINDOW *win)
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

    mvwprintw(win, 17, 2, "Processes: %d", processes);
    mvwprintw(win, 18, 2, "Threads: %d", threads);
}

void print_battery_life(WINDOW *win)
{
    int battery_life = 0;
    FILE *fp = popen("pmset -g batt | grep -o '[0-9]*%'", "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &battery_life);
        pclose(fp);
    }

    mvwprintw(win, 15, 2, "Battery Life: %d%%", battery_life);
}


int main()
{
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);

    init_colors();
    WINDOW *win = newwin(0, 0, 0, 0);
    box(win, 0, 0);

    while (1)
    {
        werase(win);
        box(win, 0, 0);

        print_battery_life(win);
        print_cpu_usage(win);
        print_uname(win);
        print_memory_usage(win);
        print_disk_usage(win);
        print_network_usage(win);
        print_system_uptime(win);
        printProcessesAndThreads(win);

        wrefresh(win);

        if (wgetch(win) == 'q')
        {
            break;
        }

        usleep(500000);
    }

    delwin(win);
    endwin();

    return 0;
}
