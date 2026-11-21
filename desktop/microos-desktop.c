/*
 * MicroOS Desktop Environment
 * macOS/Windows-inspired TUI Desktop using ncurses
 * Features: Menu bar, Dock, Desktop icons, App windows, WiFi panel, Clock
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
#include <pthread.h>

/* ─── Color pair IDs ─────────────────────────────────────────────── */
#define C_MENUBAR    1   /* white on dark-blue */
#define C_DESKTOP    2   /* white on dark-blue-grey */
#define C_DOCK       3   /* white on semi-transparent grey */
#define C_ICON       4   /* cyan on desktop */
#define C_ICON_SEL   5   /* black on cyan (selected icon) */
#define C_WIN_TITLE  6   /* black on light-grey */
#define C_WIN_BODY   7   /* white on dark-grey */
#define C_WIN_BORDER 8   /* white on dark-grey */
#define C_STATUS_OK  9   /* green on menubar */
#define C_STATUS_ERR 10  /* red on menubar */
#define C_MENU_DROP  11  /* black on white */
#define C_MENU_HIGH  12  /* white on blue */
#define C_WALLPAPER  13  /* dark cyan on navy */
#define C_CLOCK      14  /* yellow on menubar */
#define C_DOCK_ICO   15  /* white on dock */
#define C_ALERT      16  /* white on red */
#define C_DIALOG     17  /* black on light-grey */
#define C_DIALOG_BTN 18  /* black on cyan */
#define C_TERMINAL   19  /* green on black (terminal window) */

/* ─── Desktop layout constants ───────────────────────────────────── */
#define MENUBAR_H    1
#define DOCK_H       3
#define ICON_W       12
#define ICON_H       3
#define ICONS_PER_ROW 6
#define MAX_WINDOWS  8
#define MAX_APPS     12

static int term_rows, term_cols;
static volatile int running = 1;
static volatile int need_redraw = 1;

/* ─── App / Icon definitions ─────────────────────────────────────── */
typedef struct {
    const char *icon;
    const char *label;
    const char *cmd;        /* NULL = built-in handler */
    int builtin;            /* index into builtin table */
} AppDef;

#define BUILTIN_TERMINAL  0
#define BUILTIN_FILES     1
#define BUILTIN_WIFI      2
#define BUILTIN_SETTINGS  3
#define BUILTIN_GAMES     4
#define BUILTIN_SYSINFO   5
#define BUILTIN_ABOUT     6
#define BUILTIN_SHUTDOWN  7

static const AppDef APPS[] = {
    { "[>_]",  "Terminal",  NULL, BUILTIN_TERMINAL  },
    { "[dir]", "Files",     NULL, BUILTIN_FILES     },
    { "[wifi]","WiFi",      NULL, BUILTIN_WIFI      },
    { "[set]", "Settings",  NULL, BUILTIN_SETTINGS  },
    { "[game]","Arcade",    NULL, BUILTIN_GAMES     },
    { "[inf]", "System",    NULL, BUILTIN_SYSINFO   },
};
static const int NUM_APPS = 6;

/* ─── Window state ───────────────────────────────────────────────── */
typedef struct {
    int active;
    int y, x, h, w;
    char title[64];
    int type;       /* BUILTIN_* */
    char content[4096];
    int scroll;
} WinState;

static WinState wins[MAX_WINDOWS];
static int focused_win = -1;
static int selected_icon = 0;

/* ─── Menu state ─────────────────────────────────────────────────── */
static int menu_open = -1;  /* which top-level menu is open (-1 = none) */
static int menu_item = 0;

typedef struct { const char *label; int items; const char **entries; } MenuDef;
static const char *apple_items[] = { "About MicroOS", "---", "Sleep", "Restart", "Shut Down" };
static const char *file_items[]  = { "New Terminal", "New Window", "---", "Close" };
static const char *net_items[]   = { "WiFi Settings", "Network Info", "---", "Disconnect" };
static const char *sys_items[]   = { "System Info", "Task Manager", "---", "Settings" };
static const MenuDef MENUS[] = {
    { "MicroOS", 5, apple_items },
    { "File",    4, file_items  },
    { "Network", 4, net_items   },
    { "System",  4, sys_items   },
};
static const int NUM_MENUS = 4;

/* ─── WiFi state (read from /proc or iwd) ────────────────────────── */
static char wifi_ssid[64]   = "Not Connected";
static char wifi_signal[16] = "--";
static int  wifi_connected   = 0;

/* ─── Forward declarations ───────────────────────────────────────── */
static void draw_all(void);
static void open_window(int type);
static void close_focused_window(void);

/* ══════════════════════════════════════════════════════════════════
   COLOUR INITIALISATION
   ══════════════════════════════════════════════════════════════════ */
static void init_colors(void)
{
    start_color();
    use_default_colors();

    /* macOS-inspired palette */
    init_pair(C_MENUBAR,   COLOR_WHITE,   COLOR_BLUE);
    init_pair(C_DESKTOP,   COLOR_CYAN,    COLOR_BLACK);
    init_pair(C_DOCK,      COLOR_WHITE,   COLOR_WHITE);
    init_pair(C_ICON,      COLOR_WHITE,   COLOR_BLACK);
    init_pair(C_ICON_SEL,  COLOR_BLACK,   COLOR_CYAN);
    init_pair(C_WIN_TITLE, COLOR_BLACK,   COLOR_WHITE);
    init_pair(C_WIN_BODY,  COLOR_WHITE,   COLOR_BLACK);
    init_pair(C_WIN_BORDER,COLOR_WHITE,   COLOR_BLACK);
    init_pair(C_STATUS_OK, COLOR_GREEN,   COLOR_BLUE);
    init_pair(C_STATUS_ERR,COLOR_RED,     COLOR_BLUE);
    init_pair(C_MENU_DROP, COLOR_BLACK,   COLOR_WHITE);
    init_pair(C_MENU_HIGH, COLOR_WHITE,   COLOR_BLUE);
    init_pair(C_WALLPAPER, COLOR_CYAN,    COLOR_BLUE);
    init_pair(C_CLOCK,     COLOR_YELLOW,  COLOR_BLUE);
    init_pair(C_DOCK_ICO,  COLOR_BLACK,   COLOR_WHITE);
    init_pair(C_ALERT,     COLOR_WHITE,   COLOR_RED);
    init_pair(C_DIALOG,    COLOR_BLACK,   COLOR_WHITE);
    init_pair(C_DIALOG_BTN,COLOR_WHITE,   COLOR_BLUE);
    init_pair(C_TERMINAL,  COLOR_GREEN,   COLOR_BLACK);
}

/* ══════════════════════════════════════════════════════════════════
   WALLPAPER / DESKTOP BACKGROUND
   ══════════════════════════════════════════════════════════════════ */
static void draw_wallpaper(void)
{
    int desktop_top  = MENUBAR_H;
    int desktop_bot  = ROWS - DOCK_H;

    attron(COLOR_PAIR(C_WALLPAPER));
    for (int r = desktop_top; r < desktop_bot; r++) {
        move(r, 0);
        for (int c = 0; c < COLS; c++) addch(' ');
    }

    /* Gradient-like pattern using different density chars */
    for (int r = desktop_top; r < desktop_bot; r++) {
        for (int c = 0; c < COLS; c++) {
            if ((r + c) % 20 == 0) {
                mvaddch(r, c, '.');
            }
        }
    }
    attroff(COLOR_PAIR(C_WALLPAPER));

    /* MicroOS watermark in center */
    int mid_r = (desktop_top + desktop_bot) / 2 - 3;
    int mid_c = (COLS - 32) / 2;
    attron(COLOR_PAIR(C_WALLPAPER) | A_BOLD | A_DIM);
    if (mid_r > desktop_top && mid_c > 0) {
        mvprintw(mid_r,     mid_c, "  __  __ _               ___  ___ ");
        mvprintw(mid_r + 1, mid_c, " |  \\/  (_) ___ _ __ ___/ _ \\/ __|");
        mvprintw(mid_r + 2, mid_c, " | |\\/| | |/ __| '__/ _ \\ | | \\__ \\");
        mvprintw(mid_r + 3, mid_c, " |_|  |_|_|\\___/_|  \\___/\\___/|___/");
        mvprintw(mid_r + 5, mid_c + 4, "Minimal Linux. Maximum Control.");
    }
    attroff(COLOR_PAIR(C_WALLPAPER) | A_BOLD | A_DIM);
}

/* ══════════════════════════════════════════════════════════════════
   MENU BAR  (macOS-style top bar)
   ══════════════════════════════════════════════════════════════════ */
static void draw_menubar(void)
{
    /* Background */
    attron(COLOR_PAIR(C_MENUBAR) | A_BOLD);
    move(0, 0);
    for (int c = 0; c < COLS; c++) addch(' ');

    /* Left side: menus */
    int col = 1;
    for (int m = 0; m < NUM_MENUS; m++) {
        if (m == menu_open)
            attron(COLOR_PAIR(C_MENU_HIGH) | A_BOLD);
        else
            attron(COLOR_PAIR(C_MENUBAR) | A_BOLD);
        mvprintw(0, col, " %s ", MENUS[m].label);
        col += (int)strlen(MENUS[m].label) + 3;
    }

    /* Right side: WiFi + Clock */
    char timebuf[32];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(timebuf, sizeof(timebuf), "%a %b %d  %H:%M", tm);

    attron(COLOR_PAIR(C_CLOCK) | A_BOLD);
    mvprintw(0, COLS - (int)strlen(timebuf) - 2, "%s", timebuf);

    /* WiFi indicator */
    int wifi_col = COLS - (int)strlen(timebuf) - 16;
    if (wifi_col > col + 4) {
        if (wifi_connected) {
            attron(COLOR_PAIR(C_STATUS_OK) | A_BOLD);
            mvprintw(0, wifi_col, " WiFi:%s ", wifi_ssid[0] ? wifi_ssid : "On");
        } else {
            attron(COLOR_PAIR(C_STATUS_ERR) | A_BOLD);
            mvprintw(0, wifi_col, " WiFi:Off ");
        }
    }

    attroff(COLOR_PAIR(C_MENUBAR) | A_BOLD);
}

/* ══════════════════════════════════════════════════════════════════
   DESKTOP ICONS
   ══════════════════════════════════════════════════════════════════ */
static void draw_icons(void)
{
    int start_row = MENUBAR_H + 1;
    int start_col = 2;

    for (int i = 0; i < NUM_APPS; i++) {
        int row = start_row + (i / ICONS_PER_ROW) * (ICON_H + 1);
        int col = start_col + (i % ICONS_PER_ROW) * (ICON_W + 2);

        if (row + ICON_H >= ROWS - DOCK_H) break;

        int sel = (i == selected_icon);
        int pair = sel ? C_ICON_SEL : C_WALLPAPER;

        attron(COLOR_PAIR(pair) | (sel ? A_BOLD : 0));

        /* Icon box */
        mvprintw(row,     col, " %-10s", "          ");
        mvprintw(row + 1, col, " %-10s", APPS[i].icon);
        mvprintw(row + 2, col, " %-10s", "          ");

        /* Label below */
        attroff(COLOR_PAIR(pair));
        attron(COLOR_PAIR(sel ? C_ICON_SEL : C_ICON) | A_BOLD);
        int lpad = (ICON_W - (int)strlen(APPS[i].label)) / 2;
        if (lpad < 0) lpad = 0;
        mvprintw(row + 3, col + lpad, "%s", APPS[i].label);
        attroff(COLOR_PAIR(sel ? C_ICON_SEL : C_ICON) | A_BOLD);
    }
}

/* ══════════════════════════════════════════════════════════════════
   DOCK  (macOS-style bottom dock)
   ══════════════════════════════════════════════════════════════════ */
static void draw_dock(void)
{
    int dock_row = ROWS - DOCK_H;

    /* Dock background */
    attron(COLOR_PAIR(C_DOCK));
    for (int r = dock_row; r < ROWS; r++) {
        move(r, 0);
        for (int c = 0; c < COLS; c++) addch(' ');
    }

    /* Separator line */
    attron(COLOR_PAIR(C_WIN_BORDER) | A_BOLD);
    move(dock_row, 0);
    for (int c = 0; c < COLS; c++) addch(ACS_HLINE);

    /* Dock icons */
    int total_w = NUM_APPS * 8;
    int start = (COLS - total_w) / 2;
    attron(COLOR_PAIR(C_DOCK_ICO) | A_BOLD);
    for (int i = 0; i < NUM_APPS; i++) {
        int col = start + i * 8;
        mvprintw(dock_row + 1, col, " %s ", APPS[i].icon);
        /* dot indicator if window is open */
        int has_win = 0;
        for (int w = 0; w < MAX_WINDOWS; w++)
            if (wins[w].active && wins[w].type == APPS[i].builtin) { has_win = 1; break; }
        if (has_win) mvprintw(dock_row + 2, col + 2, ".");
    }

    attroff(COLOR_PAIR(C_DOCK_ICO) | A_BOLD);
}

/* ══════════════════════════════════════════════════════════════════
   WINDOWS
   ══════════════════════════════════════════════════════════════════ */
static void draw_window(int wid)
{
    WinState *w = &wins[wid];
    if (!w->active) return;

    int is_focused = (wid == focused_win);

    /* Shadow */
    attron(COLOR_PAIR(C_WIN_BODY) | A_DIM);
    for (int r = w->y + 1; r <= w->y + w->h && r < ROWS - DOCK_H; r++)
        mvprintw(r, w->x + 1, "%*s", w->w, "");
    attroff(A_DIM);

    /* Title bar */
    attron(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);
    mvprintw(w->y, w->x, " [x] [+] [_]  %-*s", w->w - 16, w->title);
    attroff(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);

    /* Window border */
    attron(COLOR_PAIR(is_focused ? C_WIN_BORDER : C_WIN_BODY) | (is_focused ? A_BOLD : 0));
    for (int r = w->y + 1; r < w->y + w->h && r < ROWS - DOCK_H; r++) {
        mvaddch(r, w->x, ACS_VLINE);
        mvaddch(r, w->x + w->w - 1, ACS_VLINE);
    }
    /* Bottom border */
    if (w->y + w->h < ROWS - DOCK_H) {
        move(w->y + w->h, w->x);
        addch(ACS_LLCORNER);
        for (int c = 1; c < w->w - 1; c++) addch(ACS_HLINE);
        addch(ACS_LRCORNER);
    }
    attroff(COLOR_PAIR(C_WIN_BORDER) | A_BOLD);

    /* Content area */
    int content_pair = (w->type == BUILTIN_TERMINAL) ? C_TERMINAL : C_WIN_BODY;
    attron(COLOR_PAIR(content_pair));
    int content_h = w->h - 2;
    int content_w = w->w - 2;
    /* Fill background */
    for (int r = 0; r < content_h && w->y + 1 + r < ROWS - DOCK_H; r++) {
        mvprintw(w->y + 1 + r, w->x + 1, "%-*s", content_w, "");
    }

    /* Print content lines */
    char *lines[256];
    int   nlines = 0;
    char  buf[4096];
    strncpy(buf, w->content, sizeof(buf) - 1);
    char *p = buf, *nl;
    while ((nl = strchr(p, '\n')) && nlines < 255) {
        *nl = 0;
        lines[nlines++] = p;
        p = nl + 1;
    }
    if (*p) lines[nlines++] = p;

    for (int r = 0; r < content_h && r + w->scroll < nlines && w->y + 1 + r < ROWS - DOCK_H; r++) {
        mvprintw(w->y + 1 + r, w->x + 1, "%-*.*s", content_w, content_w, lines[r + w->scroll]);
    }
    attroff(COLOR_PAIR(content_pair));
}

/* ══════════════════════════════════════════════════════════════════
   DROPDOWN MENU
   ══════════════════════════════════════════════════════════════════ */
static int menu_col_offset(int m)
{
    int col = 1;
    for (int i = 0; i < m; i++)
        col += (int)strlen(MENUS[i].label) + 3;
    return col;
}

static void draw_dropdown(void)
{
    if (menu_open < 0) return;
    const MenuDef *md = &MENUS[menu_open];
    int col = menu_col_offset(menu_open);
    int w = 0;
    for (int i = 0; i < md->items; i++) {
        int l = (int)strlen(md->entries[i]);
        if (l > w) w = l;
    }
    w += 4;

    for (int i = 0; i < md->items; i++) {
        if (strcmp(md->entries[i], "---") == 0) {
            attron(COLOR_PAIR(C_MENU_DROP));
            mvprintw(1 + i, col, " ");
            for (int c = 1; c < w - 1; c++) addch('-');
            addch(' ');
            attroff(COLOR_PAIR(C_MENU_DROP));
        } else {
            int sel = (i == menu_item);
            attron(COLOR_PAIR(sel ? C_MENU_HIGH : C_MENU_DROP) | (sel ? A_BOLD : 0));
            mvprintw(1 + i, col, "  %-*s  ", w - 4, md->entries[i]);
            attroff(COLOR_PAIR(sel ? C_MENU_HIGH : C_MENU_DROP) | A_BOLD);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════
   FULL REDRAW
   ══════════════════════════════════════════════════════════════════ */
static void draw_all(void)
{
    getmaxyx(stdscr, ROWS, COLS);
    erase();
    draw_wallpaper();
    draw_icons();
    draw_dock();
    draw_menubar();      /* draws on top */
    for (int w = 0; w < MAX_WINDOWS; w++) draw_window(w);
    if (menu_open >= 0) draw_dropdown();
    refresh();
}

/* ══════════════════════════════════════════════════════════════════
   WINDOW CONTENT BUILDERS
   ══════════════════════════════════════════════════════════════════ */

static void build_sysinfo(char *buf, int sz)
{
    char tmp[256];
    snprintf(buf, sz,
        "MicroOS System Information\n"
        "==========================\n\n");

    /* CPU */
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
            if (strncmp(tmp, "model name", 10) == 0) {
                char *v = strchr(tmp, ':');
                if (v) { v += 2; v[strcspn(v,"\n")] = 0; }
                snprintf(buf + strlen(buf), sz - strlen(buf), "CPU     : %s\n", v ? v : "?");
                break;
            }
        }
        fclose(f);
    }

    /* Memory */
    f = fopen("/proc/meminfo", "r");
    long total = 0, free_mem = 0;
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
            if (strncmp(tmp, "MemTotal:", 9) == 0) sscanf(tmp + 9, "%ld", &total);
            if (strncmp(tmp, "MemAvailable:", 13) == 0) sscanf(tmp + 13, "%ld", &free_mem);
        }
        fclose(f);
    }
    snprintf(buf + strlen(buf), sz - strlen(buf),
        "RAM     : %ld MB total, %ld MB free\n", total/1024, free_mem/1024);

    /* Uptime */
    f = fopen("/proc/uptime", "r");
    if (f) {
        double up; fscanf(f, "%lf", &up); fclose(f);
        int h = (int)up / 3600, m = ((int)up % 3600) / 60, s = (int)up % 60;
        snprintf(buf + strlen(buf), sz - strlen(buf), "Uptime  : %dh %dm %ds\n", h, m, s);
    }

    /* Kernel */
    f = popen("uname -r 2>/dev/null", "r");
    if (f) { fgets(tmp, sizeof(tmp), f); tmp[strcspn(tmp,"\n")] = 0; pclose(f);
        snprintf(buf + strlen(buf), sz - strlen(buf), "Kernel  : %s\n", tmp); }

    /* Storage */
    snprintf(buf + strlen(buf), sz - strlen(buf), "\nStorage Devices:\n");
    f = popen("lsblk -o NAME,SIZE,TYPE 2>/dev/null | head -10", "r");
    if (f) { while (fgets(tmp, sizeof(tmp), f)) { tmp[strcspn(tmp,"\n")] = 0;
        snprintf(buf + strlen(buf), sz - strlen(buf), "  %s\n", tmp); } pclose(f); }

    snprintf(buf + strlen(buf), sz - strlen(buf),
        "\nMicroOS v1.0  |  Minimal Linux. Maximum Control.\n");
}

static void build_wifi(char *buf, int sz)
{
    char tmp[256];
    snprintf(buf, sz,
        "WiFi & Network Manager\n"
        "======================\n\n"
        "Commands:\n"
        "  wifi scan          - Scan for networks\n"
        "  wifi connect SSID  - Connect to network\n"
        "  wifi disconnect    - Disconnect\n"
        "  wifi status        - Show status\n\n"
        "Current Status:\n");

    FILE *f = popen("/usr/bin/wifi status 2>/dev/null || ip link show 2>/dev/null | head -20", "r");
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
            tmp[strcspn(tmp,"\n")] = 0;
            snprintf(buf + strlen(buf), sz - strlen(buf), "  %s\n", tmp);
        }
        pclose(f);
    }

    /* Network interfaces */
    snprintf(buf + strlen(buf), sz - strlen(buf), "\nNetwork Interfaces:\n");
    f = popen("ip addr show 2>/dev/null | grep -E '(^[0-9]|inet )' | head -20", "r");
    if (f) {
        while (fgets(tmp, sizeof(tmp), f)) {
            tmp[strcspn(tmp,"\n")] = 0;
            snprintf(buf + strlen(buf), sz - strlen(buf), "  %s\n", tmp);
        }
        pclose(f);
    }

    snprintf(buf + strlen(buf), sz - strlen(buf),
        "\nTip: Open a Terminal and use 'wifi' commands\n"
        "     for interactive WiFi management.\n");
}

static void build_files(char *buf, int sz)
{
    snprintf(buf, sz, "File Manager\n============\n\n");
    snprintf(buf + strlen(buf), sz - strlen(buf), "Home Directory (/root):\n\n");

    DIR *d = opendir("/root");
    if (!d) d = opendir("/home");
    if (d) {
        struct dirent *de;
        while ((de = readdir(d)) != NULL) {
            if (de->d_name[0] == '.') continue;
            struct stat st;
            char path[512];
            snprintf(path, sizeof(path), "/root/%s", de->d_name);
            if (stat(path, &st) == 0) {
                snprintf(buf + strlen(buf), sz - strlen(buf),
                    "  %s  %-24s  %ld bytes\n",
                    S_ISDIR(st.st_mode) ? "[DIR]" : "[FIL]",
                    de->d_name, (long)st.st_size);
            }
        }
        closedir(d);
    }

    snprintf(buf + strlen(buf), sz - strlen(buf), "\n/mnt directories:\n");
    DIR *d2 = opendir("/mnt");
    if (d2) {
        struct dirent *de;
        while ((de = readdir(d2)) != NULL) {
            if (de->d_name[0] == '.') continue;
            snprintf(buf + strlen(buf), sz - strlen(buf), "  [DIR]  /mnt/%s\n", de->d_name);
        }
        closedir(d2);
    }

    snprintf(buf + strlen(buf), sz - strlen(buf),
        "\nTip: Use Terminal for full file management.\n");
}

static void build_games(char *buf, int sz)
{
    snprintf(buf, sz,
        "MicroOS Arcade\n==============\n\n"
        "Available Games:\n\n"
        "  1. Snake         - Classic snake game\n"
        "  2. Tetris        - Block stacking puzzle\n"
        "  3. Pong          - Classic paddle game\n"
        "  4. Minesweeper   - Mine detection game\n"
        "  5. 2048          - Number sliding puzzle\n"
        "  6. Number Guess  - Guessing game\n"
        "  7. Tic-Tac-Toe   - Classic X/O game\n"
        "  8. Space Invaders- Shoot em up!\n\n"
        "To play: Open Terminal and type:\n"
        "  arcade           - Game launcher menu\n"
        "  snake            - Play Snake directly\n"
        "  tetris           - Play Tetris directly\n"
        "  pong             - Play Pong directly\n"
        "  mines            - Play Minesweeper\n"
        "  2048             - Play 2048\n"
        "  tictactoe        - Play Tic-Tac-Toe\n"
        "  spaceinvaders    - Play Space Invaders\n\n"
        "All games run in your terminal window.\n");
}

static void build_settings(char *buf, int sz)
{
    snprintf(buf, sz,
        "MicroOS Settings\n================\n\n"
        "System Configuration\n"
        "--------------------\n"
        "  Hostname    : microos\n"
        "  Shell       : /bin/sh (BusyBox ash)\n"
        "  Init        : BusyBox init\n"
        "  Kernel      : Linux 7.0.0-43-generic\n"
        "  Architecture: x86_64\n\n"
        "Network Settings\n"
        "----------------\n"
        "  WiFi Manager : iwd (iwctl)\n"
        "  DHCP Client  : udhcpc\n"
        "  DNS Resolver : /etc/resolv.conf\n\n"
        "Display Settings\n"
        "----------------\n"
        "  Desktop Theme: MicroOS Default\n"
        "  Color Mode   : 256-color terminal\n\n"
        "Package Management\n"
        "------------------\n"
        "  APT/dpkg available on persistent SSD\n"
        "  Mount persistent storage to /mnt/ssd\n"
        "  then run: apt update && apt install\n\n"
        "Keyboard Shortcuts\n"
        "------------------\n"
        "  Enter/Space  : Open selected app\n"
        "  Arrow keys   : Navigate icons\n"
        "  Tab          : Cycle windows\n"
        "  Ctrl+W       : Close window\n"
        "  Ctrl+Q       : Quit desktop (to shell)\n"
        "  F1           : Help\n"
        "  F5           : Refresh desktop\n");
}

static void build_terminal(char *buf, int sz)
{
    snprintf(buf, sz,
        "MicroOS Terminal\n================\n\n"
        "Launching interactive shell...\n"
        "(Desktop suspends while terminal runs)\n\n"
        "Available Commands:\n"
        "  hardware [cpu|memory|network|storage|usb]\n"
        "  wifi [scan|connect|disconnect|status]\n"
        "  disks [list|info]\n"
        "  sysinfo\n"
        "  arcade\n"
        "  snake / tetris / pong / mines\n"
        "  2048 / tictactoe / spaceinvaders\n"
        "  poweroff / reboot\n\n"
        "Press Ctrl+D or type 'exit' to return\n"
        "to the MicroOS Desktop.\n");
}

static void build_about(char *buf, int sz)
{
    snprintf(buf, sz,
        "About MicroOS\n=============\n\n"
        "  __  __ _               ___  ___\n"
        " |  \\/  (_) ___ _ __ ___/ _ \\/ __|\n"
        " | |\\/| | |/ __| '__/ _ \\ | | \\__ \\\n"
        " |_|  |_|_|\\___/_|  \\___/\\___/|___/\n\n"
        "Version      : MicroOS v1.0\n"
        "Architecture : x86_64\n"
        "Kernel       : Linux 7.0.0-43-generic\n"
        "Init         : BusyBox init\n"
        "Shell        : BusyBox ash\n"
        "Desktop      : MicroOS TUI Desktop\n"
        "ISO Size     : 30.4 MB (< 50 MB limit)\n\n"
        "Features:\n"
        "  * macOS/Windows-inspired desktop\n"
        "  * Hardware autodetection\n"
        "  * WiFi / Ethernet networking (iwd)\n"
        "  * 8 built-in arcade games\n"
        "  * Debian APT package manager\n"
        "  * Safe storage management\n"
        "  * BIOS + UEFI boot support\n\n"
        "Minimal Linux. Maximum Control.\n"
        "Built with love for open computing.\n");
}

/* ══════════════════════════════════════════════════════════════════
   WINDOW MANAGEMENT
   ══════════════════════════════════════════════════════════════════ */
static void open_window(int type)
{
    /* Find free slot */
    int slot = -1;
    for (int w = 0; w < MAX_WINDOWS; w++)
        if (!wins[w].active) { slot = w; break; }
    if (slot < 0) return;

    WinState *w = &wins[slot];
    memset(w, 0, sizeof(*w));
    w->active = 1;
    w->type   = type;
    w->h = ROWS - MENUBAR_H - DOCK_H - 2;
    w->w = COLS - 4;
    w->y = MENUBAR_H + 1 + (slot % 3);
    w->x = 2 + (slot % 3);
    w->scroll = 0;

    /* Title and content */
    switch (type) {
        case BUILTIN_TERMINAL:
            strncpy(w->title, "Terminal", sizeof(w->title)-1);
            build_terminal(w->content, sizeof(w->content));
            break;
        case BUILTIN_FILES:
            strncpy(w->title, "File Manager", sizeof(w->title)-1);
            build_files(w->content, sizeof(w->content));
            break;
        case BUILTIN_WIFI:
            strncpy(w->title, "WiFi & Network", sizeof(w->title)-1);
            build_wifi(w->content, sizeof(w->content));
            break;
        case BUILTIN_SETTINGS:
            strncpy(w->title, "Settings", sizeof(w->title)-1);
            build_settings(w->content, sizeof(w->content));
            break;
        case BUILTIN_GAMES:
            strncpy(w->title, "Arcade Games", sizeof(w->title)-1);
            build_games(w->content, sizeof(w->content));
            break;
        case BUILTIN_SYSINFO:
            strncpy(w->title, "System Info", sizeof(w->title)-1);
            build_sysinfo(w->content, sizeof(w->content));
            break;
        case BUILTIN_ABOUT:
            strncpy(w->title, "About MicroOS", sizeof(w->title)-1);
            build_about(w->content, sizeof(w->content));
            break;
        default:
            strncpy(w->title, "Window", sizeof(w->title)-1);
            break;
    }
    focused_win = slot;
}

static void close_focused_window(void)
{
    if (focused_win < 0) return;
    wins[focused_win].active = 0;
    focused_win = -1;
    for (int w = MAX_WINDOWS - 1; w >= 0; w--)
        if (wins[w].active) { focused_win = w; break; }
}

/* ══════════════════════════════════════════════════════════════════
   EXECUTE MENU ACTION
   ══════════════════════════════════════════════════════════════════ */
static void exec_menu(int menu, int item)
{
    menu_open = -1;
    if (menu == 0) { /* MicroOS menu */
        if (item == 0) open_window(BUILTIN_ABOUT);
        else if (item == 2) { /* Sleep */ }
        else if (item == 3) { endwin(); system("reboot"); }
        else if (item == 4) { endwin(); system("poweroff"); }
    } else if (menu == 1) { /* File */
        if (item == 0) open_window(BUILTIN_TERMINAL);
        else if (item == 3) close_focused_window();
    } else if (menu == 2) { /* Network */
        if (item == 0) open_window(BUILTIN_WIFI);
        else if (item == 1) open_window(BUILTIN_SYSINFO);
    } else if (menu == 3) { /* System */
        if (item == 0) open_window(BUILTIN_SYSINFO);
        else if (item == 3) open_window(BUILTIN_SETTINGS);
    }
}

/* ══════════════════════════════════════════════════════════════════
   LAUNCH TERMINAL (suspend desktop)
   ══════════════════════════════════════════════════════════════════ */
static void launch_terminal(void)
{
    endwin();
    printf("\033[H\033[2J");
    printf("\033[1;36m");
    printf("  __  __ _               ___  ___\n");
    printf(" |  \\/  (_) ___ _ __ ___/ _ \\/ __|\n");
    printf(" | |\\/| | |/ __| '__/ _ \\ | | \\__ \\\n");
    printf(" |_|  |_|_|\\___/_|  \\___/\\___/|___/\n");
    printf("\033[0m\n");
    printf("\033[1;33mMicroOS Terminal\033[0m — type \033[1mexit\033[0m to return to Desktop\n\n");
    system("/bin/sh -l");
    /* Reinitialise ncurses */
    initscr();
    raw(); noecho(); keypad(stdscr, TRUE);
    curs_set(0);
    timeout(1000);
    init_colors();
    need_redraw = 1;
}

/* ══════════════════════════════════════════════════════════════════
   INPUT HANDLER
   ══════════════════════════════════════════════════════════════════ */
static void handle_input(int ch)
{
    if (menu_open >= 0) {
        const MenuDef *md = &MENUS[menu_open];
        switch (ch) {
            case KEY_UP:
                do { menu_item = (menu_item - 1 + md->items) % md->items; }
                while (strcmp(md->entries[menu_item], "---") == 0);
                break;
            case KEY_DOWN:
                do { menu_item = (menu_item + 1) % md->items; }
                while (strcmp(md->entries[menu_item], "---") == 0);
                break;
            case KEY_LEFT:
                menu_open = (menu_open - 1 + NUM_MENUS) % NUM_MENUS;
                menu_item = 0;
                break;
            case KEY_RIGHT:
                menu_open = (menu_open + 1) % NUM_MENUS;
                menu_item = 0;
                break;
            case '\n': case '\r': case KEY_ENTER:
                exec_menu(menu_open, menu_item);
                break;
            case 27: case 'q': case 'Q':
                menu_open = -1;
                break;
        }
        need_redraw = 1;
        return;
    }

    switch (ch) {
        /* Quit desktop */
        case 17: /* Ctrl+Q */
            running = 0;
            break;

        /* Open menu bar */
        case KEY_F(2): case '\t':
            if (menu_open < 0) { menu_open = 0; menu_item = 0; }
            else { menu_open = -1; }
            break;

        /* Navigate icons */
        case KEY_LEFT:
            selected_icon = (selected_icon - 1 + NUM_APPS) % NUM_APPS;
            break;
        case KEY_RIGHT:
            selected_icon = (selected_icon + 1) % NUM_APPS;
            break;
        case KEY_UP:
            selected_icon = (selected_icon - ICONS_PER_ROW + NUM_APPS) % NUM_APPS;
            break;
        case KEY_DOWN:
            selected_icon = (selected_icon + ICONS_PER_ROW) % NUM_APPS;
            break;

        /* Open selected app */
        case '\n': case '\r': case KEY_ENTER: case ' ':
            if (APPS[selected_icon].builtin == BUILTIN_TERMINAL)
                launch_terminal();
            else
                open_window(APPS[selected_icon].builtin);
            break;

        /* Close focused window */
        case 23: /* Ctrl+W */
            close_focused_window();
            break;

        /* Scroll active window */
        case KEY_PPAGE:
            if (focused_win >= 0 && wins[focused_win].scroll > 0)
                wins[focused_win].scroll--;
            break;
        case KEY_NPAGE:
            if (focused_win >= 0)
                wins[focused_win].scroll++;
            break;

        /* Cycle windows */
        case KEY_F(6): {
            int next = (focused_win + 1) % MAX_WINDOWS;
            int start = next;
            do {
                if (wins[next].active) { focused_win = next; break; }
                next = (next + 1) % MAX_WINDOWS;
            } while (next != start);
            break;
        }

        /* Help */
        case KEY_F(1):
            open_window(BUILTIN_SETTINGS);
            break;

        /* Refresh */
        case KEY_F(5): case 12: /* Ctrl+L */
            clearok(stdscr, TRUE);
            break;

        /* Open menu by number */
        case 'F': case 'f': menu_open = 1; menu_item = 0; break;
        case 'N': case 'n': menu_open = 2; menu_item = 0; break;
        case 'S': case 's': menu_open = 3; menu_item = 0; break;

        default: break;
    }
    need_redraw = 1;
}

/* ══════════════════════════════════════════════════════════════════
   WIFI PROBE (runs periodically)
   ══════════════════════════════════════════════════════════════════ */
static void probe_wifi(void)
{
    static int tick = 0;
    if (++tick % 5 != 0) return;   /* every 5 seconds */

    FILE *f = popen("ip route show default 2>/dev/null | head -1", "r");
    if (f) {
        char buf[256] = {0};
        if (fgets(buf, sizeof(buf), f) && strlen(buf) > 2) {
            wifi_connected = 1;
        } else {
            wifi_connected = 0;
        }
        pclose(f);
    }

    if (wifi_connected) {
        FILE *g = popen("iw dev 2>/dev/null | grep -A5 Interface | grep ssid | awk '{print $2}'", "r");
        if (g) {
            char ssid[64] = {0};
            if (fgets(ssid, sizeof(ssid), g)) {
                ssid[strcspn(ssid, "\n")] = 0;
                if (strlen(ssid) > 0) strncpy(wifi_ssid, ssid, sizeof(wifi_ssid)-1);
            }
            pclose(g);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════
   SIGNAL HANDLERS
   ══════════════════════════════════════════════════════════════════ */
static void handle_resize(int sig) { (void)sig; need_redraw = 1; endwin(); refresh(); }
static void handle_child(int sig)  { (void)sig; waitpid(-1, NULL, WNOHANG); }

/* ══════════════════════════════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════════════════════════════ */
int main(void)
{
    setlocale(LC_ALL, "");
    signal(SIGWINCH, handle_resize);
    signal(SIGCHLD,  handle_child);

    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(1000);   /* 1-second tick for clock updates */

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "MicroOS Desktop requires a color terminal.\n");
        return 1;
    }
    init_colors();

    /* Welcome: open about window */
    open_window(BUILTIN_ABOUT);

    /* Main event loop */
    while (running) {
        if (need_redraw) {
            draw_all();
            need_redraw = 0;
        }
        int ch = getch();
        if (ch == ERR) {
            /* Timeout — update clock and probe wifi */
            probe_wifi();
            need_redraw = 1;
        } else {
            handle_input(ch);
        }
    }

    endwin();
    printf("\033[H\033[2J");
    printf("\033[1;36m");
    printf("  __  __ _               ___  ___\n");
    printf(" |  \\/  (_) ___ _ __ ___/ _ \\/ __|\n");
    printf(" | |\\/| | |/ __| '__/ _ \\ | | \\__ \\\n");
    printf(" |_|  |_|_|\\___/_|  \\___/\\___/|___/\n");
    printf("\033[0m\n");
    printf("Returned to MicroOS shell. Type 'desktop' to restart.\n\n");
    return 0;
}
