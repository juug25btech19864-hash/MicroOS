/*
 * MicroOS Desktop Environment
 * Classic Windows 11 / Slate TUI Desktop using ncurses
 * Pixel-faithful match to classic MicroOS Dashboard layout
 */

#define _GNU_SOURCE
#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#ifndef ROWS
#define ROWS LINES
#endif

/* ─── Color Pairs ─────────────────────────────────────────────────── */
#define C_DESKTOP     1   /* Desktop background: white on dark navy/slate */
#define C_TASKBAR     2   /* Taskbar: black on light gray/white */
#define C_START_BTN   3   /* Start button: white on dark blue */
#define C_TASK_TAB    4   /* Active window tab: white on dark slate */
#define C_TRAY        5   /* Tray text: black on light gray */
#define C_ICON_TITLE  6   /* Icon title: bold white on desktop */
#define C_ICON_SEL    7   /* Selected icon: black on cyan/gold */
#define C_BADGE_GOLD  8   /* File system gold */
#define C_BADGE_BLUE  9   /* Home/Folder blue */
#define C_BADGE_GRAY  10  /* Terminal gray */
#define C_BADGE_GREEN 11  /* Snake green */
#define C_BADGE_PURP  12  /* Producer-consumer purple */
#define C_BADGE_RED   13  /* Wi-Fi red */
#define C_BADGE_CYAN  14  /* Dashboard cyan */
#define C_BADGE_LBLU  15  /* Tool light blue */
#define C_WIN_TITLE   16  /* Window title: white on dark slate */
#define C_WIN_CLOSE   17  /* Window close: white on red */
#define C_WIN_BODY    18  /* Window body: black on white */
#define C_WIN_HEADER  19  /* Window card header: dark blue on white */
#define C_WIN_BORDER  20  /* Window border: dark slate on white */
#define C_WIN_BAR     21  /* Resource bar: green on white */
#define C_MENU_BG     22  /* Start menu: black on white */
#define C_MENU_SEL    23  /* Start menu selected: white on blue */

/* ─── Applications / Desktop Icons ────────────────────────────────── */
#define TOTAL_ICONS 10

typedef struct {
    const char *badge;
    const char *label;
    int badge_pair;
    int col;        /* 0 = left col (x=2), 1 = right col (x=17) */
    int row;        /* 0..5 */
    int app_type;
    const char *cmd;
} DesktopIcon;

enum {
    APP_FILESYSTEM = 0,
    APP_FILE_MANAGER,
    APP_HOME,
    APP_DASHBOARD,
    APP_TERMINAL,
    APP_BENCHMARK,
    APP_SNAKE,
    APP_APT,
    APP_PRODUCER,
    APP_WIFI,
    APP_COUNT
};

static const DesktopIcon ICONS[TOTAL_ICONS] = {
    /* Column 1 */
    { "[##]",   "File System",        C_BADGE_GOLD,  0, 0, APP_FILESYSTEM,   NULL },
    { "[DIR]",  "Home",               C_BADGE_BLUE,  0, 1, APP_HOME,         NULL },
    { "[>_]",   "Terminal (Bash)",    C_BADGE_GRAY,  0, 2, APP_TERMINAL,     "/usr/bin/terminal" },
    { "[SNK]",  "Snake Game",         C_BADGE_GREEN, 0, 3, APP_SNAKE,        "/usr/games/snake" },
    { "[PRD]",  "Producer-Consumer...",C_BADGE_PURP, 0, 4, APP_PRODUCER,     "/usr/bin/producer-consumer" },
    { "((o))",  "WiFi & Network Se...",C_BADGE_RED,  0, 5, APP_WIFI,         NULL },

    /* Column 2 */
    { "[DIR]",  "File Manager",       C_BADGE_GOLD,  1, 0, APP_FILE_MANAGER, NULL },
    { "[DASH]", "Micro OS Dashboard", C_BADGE_CYAN,  1, 1, APP_DASHBOARD,    NULL },
    { "[TOOL]", "Boot Time Benchmark",C_BADGE_LBLU,  1, 2, APP_BENCHMARK,    "/usr/bin/boot-benchmark" },
    { "[PKG]",  "APT Package Manager",C_BADGE_GOLD,  1, 3, APP_APT,          "/usr/bin/snap-store" },
};

/* ─── Window Management ───────────────────────────────────────────── */
#define MAX_WINDOWS 4

typedef struct {
    int active;
    int y, x, h, w;
    char title[64];
    int type;
    char content[4096];
    int scroll;
} WinState;

static WinState wins[MAX_WINDOWS];
static int focused_win = -1;
static int selected_icon = 0;
static volatile int running = 1;
static volatile int need_redraw = 1;
static int start_menu_open = 0;
static int start_menu_item = 0;

/* ─── Wi-Fi Data Model ────────────────────────────────────────────── */
typedef struct {
    char name[48];
    char status[24];
    char bars[12];
    int signal;
    char dbm[32];
    char proto[32];
    char band[32];
    char rate[48];
    char ip[48];
    char sec[48];
    char bandShort[16];
} WifiNet;

static WifiNet wifi_list[16] = {
    { "MOJO-JAN-C1-F9", "Secured", "[####]", 85, "-56 dBm, Good", "802.11ax Wi-Fi 6", "5.0 GHz (Channel 48)", "1201 Mbps / 1201 Mbps Full Duplex", "10.0.2.15 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
    { "MOJO-JAN-B-F7",  "Secured", "[###.]", 60, "-70 dBm, Good", "802.11ax Wi-Fi 6", "5.0 GHz (Auto)",       "866.7 Mbps Full Duplex",            "10.0.2.16 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
    { "MOJO-F2",        "Secured", "[###.]", 60, "-70 dBm, Good", "802.11ax Wi-Fi 6", "5.0 GHz (Auto)",       "866.7 Mbps Full Duplex",            "10.0.2.17 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
    { "MOJO-JAN-C1-F8", "Secured", "[###.]", 60, "-70 dBm, Good", "802.11ax Wi-Fi 6", "5.0 GHz (Auto)",       "866.7 Mbps Full Duplex",            "10.0.2.18 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
    { "Mojo-JAN-B-F9",  "Secured", "[###.]", 60, "-70 dBm, Good", "802.11ax Wi-Fi 6", "5.0 GHz (Auto)",       "866.7 Mbps Full Duplex",            "10.0.2.19 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
    { "MOJO-JAN-B-F5",  "Secured", "[##..]", 37, "-81 dBm, Fair", "802.11ax Wi-Fi 6", "5.0 GHz (Auto)",       "866.7 Mbps Full Duplex",            "10.0.2.20 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal", "5.0 GHz" },
};
static int wifi_count = 6;
static char wifi_ssid[64] = "MOJO-JAN-C1-F9";

/* ─── Forward Declarations ────────────────────────────────────────── */
static void draw_all(void);
static void open_window(int type);
static void close_focused_window(void);
static void sync_wifi_from_host(void);
static void build_dashboard(char *buf, int sz);
static void build_wifi(char *buf, int sz);
static void build_filesystem(char *buf, int sz, const char *path, const char *name);

/* ─── Color Initializer ───────────────────────────────────────────── */
static void init_colors(void)
{
    start_color();
    if (COLORS >= 256) {
        init_pair(C_DESKTOP,    255,  17);  /* Dark midnight slate wallpaper */
        init_pair(C_TASKBAR,     16, 254);  /* Light gray taskbar */
        init_pair(C_START_BTN,  255,  24);  /* Dark blue start button */
        init_pair(C_TASK_TAB,   255, 237);  /* Active tab */
        init_pair(C_TRAY,        16, 254);  /* Clock / tray */
        init_pair(C_ICON_TITLE, 255,  17);  /* Icon title */
        init_pair(C_ICON_SEL,    16, 220);  /* Gold selection */

        init_pair(C_BADGE_GOLD, 214,  17);  /* Gold */
        init_pair(C_BADGE_BLUE,  39,  17);  /* Blue */
        init_pair(C_BADGE_GRAY, 250, 236);  /* Gray */
        init_pair(C_BADGE_GREEN, 46,  17);  /* Green */
        init_pair(C_BADGE_PURP, 141,  17);  /* Purple */
        init_pair(C_BADGE_RED,  196,  17);  /* Red */
        init_pair(C_BADGE_CYAN,  51,  17);  /* Cyan */
        init_pair(C_BADGE_LBLU,  45,  17);  /* Light blue */

        init_pair(C_WIN_TITLE,  255, 236);  /* Dark window title */
        init_pair(C_WIN_CLOSE,  255, 196);  /* Red close button */
        init_pair(C_WIN_BODY,    16, 255);  /* Clean white window body */
        init_pair(C_WIN_HEADER,  21, 255);  /* Bold blue header on white */
        init_pair(C_WIN_BORDER, 240, 255);  /* Border */
        init_pair(C_WIN_BAR,     28, 255);  /* Progress green */
        init_pair(C_MENU_BG,     16, 254);  /* Start menu */
        init_pair(C_MENU_SEL,   255,  24);  /* Start menu selected */
    } else {
        init_pair(C_DESKTOP,    COLOR_WHITE, COLOR_BLUE);
        init_pair(C_TASKBAR,    COLOR_BLACK, COLOR_WHITE);
        init_pair(C_START_BTN,  COLOR_WHITE, COLOR_BLACK);
        init_pair(C_TASK_TAB,   COLOR_WHITE, COLOR_BLACK);
        init_pair(C_TRAY,       COLOR_BLACK, COLOR_WHITE);
        init_pair(C_ICON_TITLE, COLOR_WHITE, COLOR_BLUE);
        init_pair(C_ICON_SEL,   COLOR_BLACK, COLOR_CYAN);
        init_pair(C_BADGE_GOLD, COLOR_YELLOW, COLOR_BLUE);
        init_pair(C_BADGE_BLUE, COLOR_CYAN,   COLOR_BLUE);
        init_pair(C_BADGE_GRAY, COLOR_WHITE,  COLOR_BLACK);
        init_pair(C_BADGE_GREEN,COLOR_GREEN, COLOR_BLUE);
        init_pair(C_BADGE_PURP, COLOR_MAGENTA,COLOR_BLUE);
        init_pair(C_BADGE_RED,  COLOR_RED,   COLOR_BLUE);
        init_pair(C_BADGE_CYAN, COLOR_CYAN,  COLOR_BLUE);
        init_pair(C_BADGE_LBLU, COLOR_CYAN,  COLOR_BLUE);
        init_pair(C_WIN_TITLE,  COLOR_WHITE, COLOR_BLACK);
        init_pair(C_WIN_CLOSE,  COLOR_WHITE, COLOR_RED);
        init_pair(C_WIN_BODY,   COLOR_BLACK, COLOR_WHITE);
        init_pair(C_WIN_HEADER, COLOR_BLUE,  COLOR_WHITE);
        init_pair(C_WIN_BORDER, COLOR_BLACK, COLOR_WHITE);
        init_pair(C_WIN_BAR,    COLOR_GREEN, COLOR_WHITE);
        init_pair(C_MENU_BG,    COLOR_BLACK, COLOR_WHITE);
        init_pair(C_MENU_SEL,   COLOR_WHITE, COLOR_BLUE);
    }
}

/* ─── Real-Time Host Wi-Fi Auto-Sync ──────────────────────────────── */
static void sync_wifi_from_host(void)
{
    const char *hosts[] = { "10.0.2.2", "192.168.56.1", "127.0.0.1" };
    int n_hosts = sizeof(hosts) / sizeof(hosts[0]);
    static char response[8192];
    int got_resp = 0;

    for (int h = 0; h < n_hosts && !got_resp; h++) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) continue;

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 250000;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        struct sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons(8080);
        if (inet_pton(AF_INET, hosts[h], &sin.sin_addr) <= 0) {
            close(s);
            continue;
        }

        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
            const char *req = "GET /api/wifi HTTP/1.0\r\nHost: localhost\r\nConnection: close\r\n\r\n";
            send(s, req, strlen(req), 0);
            int total = 0;
            while (total < (int)sizeof(response) - 1) {
                int n = recv(s, response + total, sizeof(response) - 1 - total, 0);
                if (n <= 0) break;
                total += n;
            }
            if (total > 0) {
                response[total] = '\0';
                got_resp = 1;
            }
        }
        close(s);
    }

    if (!got_resp) return;

    /* Parse connected SSID */
    char *conn_ptr = strstr(response, "\"connected\":");
    if (conn_ptr) {
        char *p = strstr(conn_ptr, "\"ssid\":");
        if (p) {
            char *q1 = strchr(p + 7, '\"');
            if (q1) {
                char *q2 = strchr(q1 + 1, '\"');
                if (q2 && (q2 - q1 < (int)sizeof(wifi_ssid))) {
                    int len = q2 - q1 - 1;
                    strncpy(wifi_ssid, q1 + 1, len);
                    wifi_ssid[len] = '\0';
                    FILE *wf = fopen("/run/wifi.state", "w");
                    if (wf) { fputs(wifi_ssid, wf); fclose(wf); }
                }
            }
        }
    }

    /* Parse available array */
    char *avail_ptr = strstr(response, "\"available\":");
    if (avail_ptr) {
        char *bracket = strchr(avail_ptr, '[');
        if (bracket) {
            int count = 0;
            char *obj_start = strchr(bracket, '{');
            while (obj_start && count < 16) {
                char *obj_end = strchr(obj_start, '}');
                if (!obj_end) break;

                WifiNet *net = &wifi_list[count];
                memset(net, 0, sizeof(WifiNet));

                char *p_ssid = strstr(obj_start, "\"ssid\":");
                if (p_ssid && p_ssid < obj_end) {
                    char *s1 = strchr(p_ssid + 7, '\"');
                    if (s1 && s1 < obj_end) {
                        char *s2 = strchr(s1 + 1, '\"');
                        if (s2 && s2 <= obj_end) {
                            int l = s2 - s1 - 1;
                            if (l >= (int)sizeof(net->name)) l = sizeof(net->name) - 1;
                            strncpy(net->name, s1 + 1, l);
                            net->name[l] = '\0';
                        }
                    }
                }

                if (net->name[0] != '\0') {
                    net->signal = 75;
                    char *p_sig = strstr(obj_start, "\"signal\":");
                    if (p_sig && p_sig < obj_end) net->signal = atoi(p_sig + 9);

                    if (net->signal >= 80) strcpy(net->bars, "[####]");
                    else if (net->signal >= 60) strcpy(net->bars, "[###.]");
                    else if (net->signal >= 40) strcpy(net->bars, "[##..]");
                    else strcpy(net->bars, "[#...]");

                    strcpy(net->status, "Secured");
                    strcpy(net->sec, "WPA2-Personal");
                    char *p_sec = strstr(obj_start, "\"sec\":");
                    if (p_sec && p_sec < obj_end) {
                        char *sc1 = strchr(p_sec + 6, '\"');
                        if (sc1 && sc1 < obj_end) {
                            char *sc2 = strchr(sc1 + 1, '\"');
                            if (sc2 && sc2 <= obj_end) {
                                int l = sc2 - sc1 - 1;
                                if (l >= (int)sizeof(net->sec)) l = sizeof(net->sec) - 1;
                                strncpy(net->sec, sc1 + 1, l);
                                net->sec[l] = '\0';
                            }
                        }
                    }

                    strcpy(net->bandShort, "5.0 GHz");
                    strcpy(net->band, "5.0 GHz (Channel 48)");
                    char *p_band = strstr(obj_start, "\"band\":");
                    if (p_band && p_band < obj_end) {
                        char *b1 = strchr(p_band + 7, '\"');
                        if (b1 && b1 < obj_end) {
                            char *b2 = strchr(b1 + 1, '\"');
                            if (b2 && b2 <= obj_end) {
                                int l = b2 - b1 - 1;
                                if (l >= (int)sizeof(net->bandShort)) l = sizeof(net->bandShort) - 1;
                                strncpy(net->bandShort, b1 + 1, l);
                                net->bandShort[l] = '\0';
                                snprintf(net->band, sizeof(net->band), "%s (Auto)", net->bandShort);
                            }
                        }
                    }

                    strcpy(net->rate, "866.7 Mbps Full Duplex");
                    char *p_rate = strstr(obj_start, "\"rate\":");
                    if (p_rate && p_rate < obj_end) {
                        char *r1 = strchr(p_rate + 7, '\"');
                        if (r1 && r1 < obj_end) {
                            char *r2 = strchr(r1 + 1, '\"');
                            if (r2 && r2 <= obj_end) {
                                int l = r2 - r1 - 1;
                                if (l >= (int)sizeof(net->rate)) l = sizeof(net->rate) - 1;
                                strncpy(net->rate, r1 + 1, l);
                                net->rate[l] = '\0';
                            }
                        }
                    }

                    strcpy(net->dbm, "-56 dBm, Good");
                    char *p_dbm = strstr(obj_start, "\"dbm\":");
                    if (p_dbm && p_dbm < obj_end) {
                        char *d1 = strchr(p_dbm + 6, '\"');
                        if (d1 && d1 < obj_end) {
                            char *d2 = strchr(d1 + 1, '\"');
                            if (d2 && d2 <= obj_end) {
                                int l = d2 - d1 - 1;
                                if (l >= (int)sizeof(net->dbm)) l = sizeof(net->dbm) - 1;
                                strncpy(net->dbm, d1 + 1, l);
                                net->dbm[l] = '\0';
                            }
                        }
                    }

                    if (strstr(net->bandShort, "5") != NULL) strcpy(net->proto, "802.11ax Wi-Fi 6");
                    else strcpy(net->proto, "802.11n Wi-Fi 4");
                    snprintf(net->ip, sizeof(net->ip), "10.0.2.%d (Gateway: 10.0.2.2)", 15 + count);
                    count++;
                }
                obj_start = strchr(obj_end + 1, '{');
            }
            if (count > 0) wifi_count = count;
        }
    }
}

/* ─── Content Builders ────────────────────────────────────────────── */

static void build_dashboard(char *buf, int sz)
{
    snprintf(buf, sz,
        "MicroOS Dashboard v1.0 (64-bit x86_64)\n"
        "----------------------------------------------------------------\n"
        "SYSTEM INFORMATION               RESOURCE MONITOR\n"
        "Kernel   : Linux 7.0.0-generic   CPU Load : 12%% [===..........]\n"
        "Arch     : x86_64 Intel/AMD      Memory   : 38MB / 1024MB\n"
        "Platform : Oracle VirtualBox     Storage  : 14.8MB / 512MB [==....]\n"
        "Uptime   : 00:00 up\n"
        "----------------------------------------------------------------\n"
        "PID   PROCESS NAME      STATUS    CPU %%   RAM\n"
        " 1    system_init       Running   0.2%%    4.1 MB\n"
        " 2    compositor_gui    Active   12.4%%   16.2 MB\n"
        " 3    iwd_wifi_daemon   Sleeping  0.1%%    2.8 MB\n"
        " 4    ps2_input_mgr     Sleeping  0.4%%    1.9 MB\n"
        " 5    apt_daemon        Idle      0.0%%    3.2 MB\n"
        "----------------------------------------------------------------\n"
        "ACTIONS: Press [Esc] Close Window | [Tab] Switch App\n");
}

static void build_wifi(char *buf, int sz)
{
    int connected = (wifi_ssid[0] != '\0');
    const WifiNet *cur = NULL;
    for (int i = 0; i < wifi_count; i++) {
        if (strcmp(wifi_ssid, wifi_list[i].name) == 0) {
            cur = &wifi_list[i];
            break;
        }
    }

    char cur_block[512];
    if (connected && cur) {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-24s [CONNECTED, SECURED]\n"
            "        Signal: %d%% (%s)  |  Band: %s\n"
            "        Speed : %s  |  Proto: %s\n"
            "        IP    : %s\n",
            cur->name,
            cur->signal, cur->dbm, cur->band,
            cur->rate, cur->proto,
            cur->ip);
    } else if (connected) {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-24s [CONNECTED]\n"
            "        Signal: 85%% (-56 dBm, Good)  |  Band: 5.0 GHz (Channel 48)\n"
            "        Speed : 1201 Mbps Full Duplex  |  Proto: 802.11ax Wi-Fi 6\n"
            "        IP    : 10.0.2.15 (Gateway: 10.0.2.2  DNS: 8.8.8.8)\n",
            wifi_ssid);
    } else {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-24s [DISCONNECTED, OFFLINE]\n"
            "        Signal: 0%% (Radio Idle)  |  Band: None  |  Speed: 0 Mbps\n",
            "No Network Connected");
    }

    char list_block[1536];
    list_block[0] = '\0';
    int offset = 0;
    for (int i = 0; i < wifi_count && i < 6; i++) {
        int is_conn = (strcmp(wifi_ssid, wifi_list[i].name) == 0);
        char line[256];
        snprintf(line, sizeof(line),
            "  [%d] ((o)) %-16s %-11s %s %2d%% %-7s %s\n",
            i + 1,
            wifi_list[i].name,
            is_conn ? "[Connected]" : "[In Range]",
            wifi_list[i].bars,
            wifi_list[i].signal,
            wifi_list[i].bandShort,
            wifi_list[i].status);
        int len = strlen(line);
        if (offset + len < (int)sizeof(list_block) - 1) {
            strcat(list_block, line);
            offset += len;
        }
    }

    int max_key = (wifi_count > 6) ? 6 : wifi_count;
    snprintf(buf, sz,
        "Quick Settings -- Wi-Fi & Available Networks\n"
        "================================================================\n\n"
        "Wi-Fi Radio: [%s]  |  Hardware Adapter: RTL8852BE WiFi 6\n"
        "Interface  : wlan0 / e1000 (Bridged NAT to Host Network)\n\n"
        "CURRENTLY CONNECTED NETWORK:\n"
        "%s\n"
        "AVAILABLE NETWORKS (%d in range):\n"
        "%s\n"
        "ACTIONS: Press [1-%d] Connect | [d] Disconnect | [r] Rescan | [Esc] Close\n",
        connected ? "ENABLED" : "DISABLED",
        cur_block,
        wifi_count,
        list_block,
        max_key);
}

static void build_filesystem(char *buf, int sz, const char *path, const char *name)
{
    char list[1024] = "";
    DIR *d = opendir(path);
    if (d) {
        struct dirent *dir;
        int count = 0;
        while ((dir = readdir(d)) != NULL && count < 10) {
            if (dir->d_name[0] == '.') continue;
            char item[128];
            snprintf(item, sizeof(item), "  [ITEM] %-18s (Ready)\n", dir->d_name);
            strcat(list, item);
            count++;
        }
        closedir(d);
    } else {
        strcpy(list, "  [INFO] Standard system root volume mounted (Read-Write)\n");
    }

    snprintf(buf, sz,
        "%s Explorer & Directory Contents\n"
        "================================================================\n\n"
        "Directory Path : %s\n"
        "Mount Type     : ext4 / tmpfs RAM overlay\n\n"
        "DIRECTORY ITEMS:\n"
        "%s\n"
        "Press [Esc] to return to Desktop.\n",
        name, path, list);
}

/* ─── Window Actions ──────────────────────────────────────────────── */
static void open_window(int type)
{
    /* If already open, focus it */
    for (int w = 0; w < MAX_WINDOWS; w++) {
        if (wins[w].active && wins[w].type == type) {
            focused_win = w;
            need_redraw = 1;
            return;
        }
    }

    int slot = -1;
    for (int w = 0; w < MAX_WINDOWS; w++) {
        if (!wins[w].active) { slot = w; break; }
    }
    if (slot < 0) slot = 0;

    WinState *w = &wins[slot];
    memset(w, 0, sizeof(*w));
    w->active = 1;
    w->type = type;
    w->scroll = 0;

    /* Window geometry positioned neatly on the right, icons stay visible on left */
    w->x = 30;
    w->y = 2;
    w->w = 48;
    w->h = 20;

    switch (type) {
        case APP_DASHBOARD:
            strncpy(w->title, "MicroOS Dashboard", sizeof(w->title)-1);
            build_dashboard(w->content, sizeof(w->content));
            break;
        case APP_WIFI:
            strncpy(w->title, "WiFi & Network Settings", sizeof(w->title)-1);
            sync_wifi_from_host();
            build_wifi(w->content, sizeof(w->content));
            break;
        case APP_FILESYSTEM:
            strncpy(w->title, "File System (/)", sizeof(w->title)-1);
            build_filesystem(w->content, sizeof(w->content), "/", "File System");
            break;
        case APP_FILE_MANAGER:
            strncpy(w->title, "File Manager", sizeof(w->title)-1);
            build_filesystem(w->content, sizeof(w->content), "/root", "User Files");
            break;
        case APP_HOME:
            strncpy(w->title, "Home Directory", sizeof(w->title)-1);
            build_filesystem(w->content, sizeof(w->content), "/root", "Home Directory");
            break;
        default:
            strncpy(w->title, "MicroOS Window", sizeof(w->title)-1);
            snprintf(w->content, sizeof(w->content), "Application active.\nPress [Esc] to close.\n");
            break;
    }
    focused_win = slot;
}

static void close_focused_window(void)
{
    if (focused_win >= 0) {
        wins[focused_win].active = 0;
        focused_win = -1;
        for (int w = MAX_WINDOWS - 1; w >= 0; w--) {
            if (wins[w].active) { focused_win = w; break; }
        }
    }
}

static void launch_app(const char *cmd)
{
    if (!cmd) return;
    endwin();
    system(cmd);
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    init_colors();
    need_redraw = 1;
}

/* ─── Rendering Engine ────────────────────────────────────────────── */

static void draw_desktop_icons(void)
{
    /* Coordinates for 2 columns */
    const int col_x[2] = { 2, 16 };
    const int row_y[6] = { 1, 4, 7, 10, 13, 16 };

    for (int i = 0; i < TOTAL_ICONS; i++) {
        const DesktopIcon *ic = &ICONS[i];
        int x = col_x[ic->col];
        int y = row_y[ic->row];
        int is_sel = (i == selected_icon);

        /* Icon Badge */
        attron(COLOR_PAIR(ic->badge_pair) | A_BOLD);
        mvprintw(y, x, "%s", ic->badge);
        attroff(COLOR_PAIR(ic->badge_pair) | A_BOLD);

        /* Label */
        if (is_sel) attron(COLOR_PAIR(C_ICON_SEL) | A_BOLD);
        else        attron(COLOR_PAIR(C_ICON_TITLE));
        mvprintw(y + 1, x, "%-14.14s", ic->label);
        if (is_sel) attroff(COLOR_PAIR(C_ICON_SEL) | A_BOLD);
        else        attroff(COLOR_PAIR(C_ICON_TITLE));
    }
}

static void draw_window(int slot)
{
    WinState *w = &wins[slot];
    if (!w->active) return;

    /* Window Body Fill (solid white) */
    attron(COLOR_PAIR(C_WIN_BODY));
    for (int r = w->y; r < w->y + w->h && r < ROWS - 1; r++) {
        move(r, w->x);
        for (int c = 0; c < w->w && w->x + c < COLS; c++) addch(' ');
    }
    attroff(COLOR_PAIR(C_WIN_BODY));

    /* Window Title Bar */
    attron(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);
    move(w->y, w->x);
    for (int c = 0; c < w->w && w->x + c < COLS; c++) addch(' ');
    mvprintw(w->y, w->x + 1, "%-*.*s", w->w - 12, w->w - 12, w->title);
    mvprintw(w->y, w->x + w->w - 9, "[-] [ ]");
    attroff(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);

    /* Red [X] Close button */
    attron(COLOR_PAIR(C_WIN_CLOSE) | A_BOLD);
    mvprintw(w->y, w->x + w->w - 3, "[X]");
    attroff(COLOR_PAIR(C_WIN_CLOSE) | A_BOLD);

    /* Content Lines */
    char buf[4096];
    strncpy(buf, w->content, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *lines[64];
    int nl = 0;
    char *p = buf, *nl2;
    while ((nl2 = strchr(p, '\n')) && nl < 60) {
        *nl2 = '\0';
        lines[nl++] = p;
        p = nl2 + 1;
    }
    if (*p) lines[nl++] = p;

    attron(COLOR_PAIR(C_WIN_BODY));
    int max_rows = w->h - 2;
    int max_cols = w->w - 2;
    for (int r = 0; r < max_rows && r + w->scroll < nl && w->y + 1 + r < ROWS - 1; r++) {
        const char *line = lines[r + w->scroll];
        if (strstr(line, "INFORMATION") || strstr(line, "PROCESS NAME") || strstr(line, "AVAILABLE")) {
            attron(COLOR_PAIR(C_WIN_HEADER) | A_BOLD);
            mvprintw(w->y + 1 + r, w->x + 1, "%-*.*s", max_cols, max_cols, line);
            attroff(COLOR_PAIR(C_WIN_HEADER) | A_BOLD);
            attron(COLOR_PAIR(C_WIN_BODY));
        } else {
            mvprintw(w->y + 1 + r, w->x + 1, "%-*.*s", max_cols, max_cols, line);
        }
    }
    attroff(COLOR_PAIR(C_WIN_BODY));
}

static void draw_taskbar(void)
{
    int dock_row = ROWS - 1;

    /* Fill taskbar (light gray) */
    attron(COLOR_PAIR(C_TASKBAR));
    move(dock_row, 0);
    for (int c = 0; c < COLS; c++) addch(' ');

    /* Start Button */
    attron(COLOR_PAIR(C_START_BTN) | A_BOLD);
    mvprintw(dock_row, 0, " [ Start ] ");
    attroff(COLOR_PAIR(C_START_BTN) | A_BOLD);

    /* Active Window Tab */
    if (focused_win >= 0 && wins[focused_win].active) {
        attron(COLOR_PAIR(C_TASK_TAB) | A_BOLD);
        char tab[24];
        snprintf(tab, sizeof(tab), " [ %-14.14s ] ", wins[focused_win].title);
        mvprintw(dock_row, 12, "%s", tab);
        attroff(COLOR_PAIR(C_TASK_TAB) | A_BOLD);
    }

    /* System Tray (Wi-Fi + Clock in 12-hour format) */
    char tray[32];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char timebuf[16];
    strftime(timebuf, sizeof(timebuf), "%I:%M %p", tm);
    snprintf(tray, sizeof(tray), "|ll  %s ", timebuf);

    attron(COLOR_PAIR(C_TRAY) | A_BOLD);
    mvprintw(dock_row, COLS - (int)strlen(tray), "%s", tray);
    attroff(COLOR_PAIR(C_TRAY) | A_BOLD);
}

static void draw_start_menu(void)
{
    if (!start_menu_open) return;

    static const char *items[] = {
        "MicroOS Dashboard",
        "WiFi & Network Settings",
        "File Manager",
        "Terminal (Bash)",
        "Snake Arcade Game",
        "Restart System",
        "Shut Down Computer"
    };
    int count = 7;
    int w = 26;
    int h = count + 2;
    int x = 0;
    int y = ROWS - 1 - h;

    attron(COLOR_PAIR(C_MENU_BG));
    for (int r = y; r < y + h; r++) {
        move(r, x);
        for (int c = 0; c < w; c++) addch(' ');
    }
    mvprintw(y, x + 1, "=== MicroOS Menu ===");
    for (int i = 0; i < count; i++) {
        if (i == start_menu_item) {
            attron(COLOR_PAIR(C_MENU_SEL) | A_BOLD);
            mvprintw(y + 1 + i, x, "  %-*.*s", w - 2, w - 2, items[i]);
            attroff(COLOR_PAIR(C_MENU_SEL) | A_BOLD);
            attron(COLOR_PAIR(C_MENU_BG));
        } else {
            mvprintw(y + 1 + i, x, "  %-*.*s", w - 2, w - 2, items[i]);
        }
    }
    attroff(COLOR_PAIR(C_MENU_BG));
}

static void draw_all(void)
{
    /* Clear desktop with dark slate color */
    attron(COLOR_PAIR(C_DESKTOP));
    for (int r = 0; r < ROWS - 1; r++) {
        move(r, 0);
        for (int c = 0; c < COLS; c++) addch(' ');
    }
    attroff(COLOR_PAIR(C_DESKTOP));

    draw_desktop_icons();

    /* Draw any active windows */
    for (int w = 0; w < MAX_WINDOWS; w++) {
        if (wins[w].active) draw_window(w);
    }

    draw_taskbar();
    if (start_menu_open) draw_start_menu();

    refresh();
}

/* ─── Input Handling ──────────────────────────────────────────────── */

static void handle_input(int ch)
{
    /* Start menu active */
    if (start_menu_open) {
        switch (ch) {
            case KEY_UP:
                start_menu_item = (start_menu_item - 1 + 7) % 7;
                break;
            case KEY_DOWN:
                start_menu_item = (start_menu_item + 1) % 7;
                break;
            case '\n': case '\r': case KEY_ENTER:
                start_menu_open = 0;
                switch (start_menu_item) {
                    case 0: open_window(APP_DASHBOARD); break;
                    case 1: open_window(APP_WIFI); break;
                    case 2: open_window(APP_FILE_MANAGER); break;
                    case 3: launch_app("/usr/bin/terminal"); break;
                    case 4: launch_app("/usr/games/snake"); break;
                    case 5: endwin(); system("reboot"); break;
                    case 6: endwin(); system("poweroff"); break;
                }
                break;
            case 27: case 'q': case 'Q':
                start_menu_open = 0;
                break;
        }
        need_redraw = 1;
        return;
    }

    /* Window active */
    if (focused_win >= 0 && wins[focused_win].active) {
        if (ch == 27) { /* Esc: close window */
            close_focused_window();
            need_redraw = 1;
            return;
        }
        if (ch == KEY_UP || ch == KEY_PPAGE) {
            wins[focused_win].scroll = (wins[focused_win].scroll > 1) ? wins[focused_win].scroll - 1 : 0;
            need_redraw = 1;
            return;
        }
        if (ch == KEY_DOWN || ch == KEY_NPAGE) {
            wins[focused_win].scroll += 1;
            need_redraw = 1;
            return;
        }

        /* Wi-Fi specific keys */
        if (wins[focused_win].type == APP_WIFI) {
            if (ch >= '1' && ch <= '6' && ch <= '0' + wifi_count) {
                strncpy(wifi_ssid, wifi_list[ch - '1'].name, sizeof(wifi_ssid) - 1);
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                FILE *wf = fopen("/run/wifi.state", "w");
                if (wf) { fputs(wifi_ssid, wf); fclose(wf); }
                need_redraw = 1;
                return;
            }
            if (ch == 'd' || ch == 'D') {
                wifi_ssid[0] = '\0';
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                FILE *wf = fopen("/run/wifi.state", "w");
                if (wf) { fputs("", wf); fclose(wf); }
                need_redraw = 1;
                return;
            }
            if (ch == 'r' || ch == 'R') {
                sync_wifi_from_host();
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                need_redraw = 1;
                return;
            }
        }
    }

    /* Desktop Icon Navigation & Global Keys */
    switch (ch) {
        case KEY_UP:
            if (selected_icon > 0) selected_icon--;
            break;
        case KEY_DOWN:
            if (selected_icon < TOTAL_ICONS - 1) selected_icon++;
            break;
        case KEY_LEFT:
            if (selected_icon >= 6) selected_icon -= 6;
            break;
        case KEY_RIGHT:
            if (selected_icon < 4) selected_icon += 6;
            break;
        case '\n': case '\r': case KEY_ENTER:
            if (ICONS[selected_icon].cmd) {
                launch_app(ICONS[selected_icon].cmd);
            } else {
                open_window(ICONS[selected_icon].app_type);
            }
            break;
        case 's': case 'S':
            start_menu_open = !start_menu_open;
            start_menu_item = 0;
            break;
        case 'w': case 'W':
            open_window(APP_WIFI);
            break;
        case 'd': case 'D':
            open_window(APP_DASHBOARD);
            break;
        case 't': case 'T':
            launch_app("/usr/bin/terminal");
            break;
        case 'k': case 'K':
            launch_app("/usr/games/snake");
            break;
        case 'p': case 'P':
            launch_app("/usr/bin/producer-consumer");
            break;
        case 'b': case 'B':
            launch_app("/usr/bin/boot-benchmark");
            break;
        case 'q': case 'Q':
            if (focused_win >= 0) close_focused_window();
            break;
    }
    need_redraw = 1;
}

static void handle_mouse(MEVENT *ev)
{
    /* Start Button click */
    if (ev->y == ROWS - 1 && ev->x >= 0 && ev->x <= 11) {
        start_menu_open = !start_menu_open;
        start_menu_item = 0;
        need_redraw = 1;
        return;
    }

    /* Window close [X] button click */
    if (focused_win >= 0 && wins[focused_win].active) {
        WinState *w = &wins[focused_win];
        if (ev->y == w->y && ev->x >= w->x + w->w - 4 && ev->x <= w->x + w->w) {
            close_focused_window();
            need_redraw = 1;
            return;
        }
    }

    /* Desktop icon clicks */
    const int col_x[2] = { 2, 16 };
    const int row_y[6] = { 1, 4, 7, 10, 13, 16 };
    for (int i = 0; i < TOTAL_ICONS; i++) {
        int x = col_x[ICONS[i].col];
        int y = row_y[ICONS[i].row];
        if (ev->x >= x && ev->x <= x + 14 && ev->y >= y && ev->y <= y + 1) {
            selected_icon = i;
            if (ICONS[i].cmd) launch_app(ICONS[i].cmd);
            else open_window(ICONS[i].app_type);
            need_redraw = 1;
            return;
        }
    }
}

/* ─── Entry Point ─────────────────────────────────────────────────── */
int main(void)
{
    setlocale(LC_ALL, "");

    if (!getenv("TERMINFO")) setenv("TERMINFO", "/etc/terminfo", 1);
    setenv("TERM", "linux", 1);
    setenv("LINES", "25", 1);
    setenv("COLUMNS", "80", 1);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(500); /* 500 ms non-blocking wait for clock ticks */
    mousemask(BUTTON1_CLICKED | BUTTON1_PRESSED, NULL);

    init_colors();

    /* Load initial Wi-Fi SSID if present */
    FILE *wf = fopen("/run/wifi.state", "r");
    if (!wf) wf = fopen("/etc/wifi_ssid", "r");
    if (wf) {
        char line[64];
        if (fgets(line, sizeof(line), wf)) {
            int l = strlen(line);
            while (l > 0 && (line[l-1] == '\n' || line[l-1] == '\r' || line[l-1] == ' '))
                line[--l] = '\0';
            if (line[0] != '\0') strncpy(wifi_ssid, line, sizeof(wifi_ssid) - 1);
        }
        fclose(wf);
    }

    /* Sync initial host Wi-Fi */
    sync_wifi_from_host();

    /* Open MicroOS Dashboard window on boot, matching the target screenshot */
    open_window(APP_DASHBOARD);

    int tick = 0;
    while (running) {
        if (need_redraw) {
            draw_all();
            need_redraw = 0;
        }

        int ch = getch();
        if (ch == KEY_MOUSE) {
            MEVENT ev;
            if (getmouse(&ev) == OK) handle_mouse(&ev);
        } else if (ch != ERR) {
            handle_input(ch);
        } else {
            /* Clock & Wi-Fi background tick every 5 seconds */
            if (++tick >= 10) {
                tick = 0;
                sync_wifi_from_host();
                if (focused_win >= 0 && wins[focused_win].active && wins[focused_win].type == APP_WIFI) {
                    build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                }
                need_redraw = 1;
            }
        }
    }

    endwin();
    return 0;
}
