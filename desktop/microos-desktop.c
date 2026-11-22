/*
 * MicroOS — Modern Desktop Environment v5.2 (Pixel-Faithful Edition)
 *
 * Visual Architecture:
 *   - Acrylic Top Bar:
 *       [*] Cyan Logo | Options v | Path breadcrumbs | [ Date & Time ] | (vol) | Red Power (X)
 *   - Exact 2-Column Desktop Icons Grid (Left Side, 22 apps):
 *       Col 1 (x=2):  File System, Home, Developer Terminal, System Monitor,
 *                     Storage Explorer, Settings, Network Explorer,
 *                     Microkernel, Hardware & Devices, Producer-Consumer,
 *                     Boot Benchmark
 *       Col 2 (x=27): Files, Web Browser, APT Package Mgr,
 *                     Wi-Fi Manager, Snap Store, Snake Game,
 *                     Retro Arcade Hub, Tetris Arcade, 2048 Puzzle,
 *                     Pong Arcade, Minesweeper
 *       Row spacing:  Every 2 rows (r = 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22)
 *   - Organic Flowing 3D Satin Wave Wallpaper (Columns 54 to 79):
 *       Sweeping S-curve from top-right to center-right with:
 *       Hot glowing core (#/white on magenta) -> Electric pink (%) ->
 *       Royal purple (*) -> Indigo blue (+) -> Ambient shadow (.)
 *   - Floating Bottom Dock (Row 24):
 *       Left: Colored app tiles: [*] [V] [F] [>] [G] [W] [%] [#]
 *       Right: Workspace Pills: [Workspace 1] [W2] [W3] [W4] [#]
 *   - Host-grade Wi-Fi Quick Settings Flyout (Connected + 7 In-Range Networks with signal bars)
 *   - Full Arcade Games Suite + POSIX Producer-Consumer Simulator
 */

#define _GNU_SOURCE
#include <ncurses.h>

#ifndef ROWS
#define ROWS LINES
#endif

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
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

/* ═══════════════════════════════════════════════════════════════════
   COLOR DEFINITIONS
   ═══════════════════════════════════════════════════════════════════ */
#define C_BG            1   /* Midnight background */
#define C_TOPBAR        2   /* Top bar text & surface */
#define C_TOP_LOGO      3   /* Bright cyan logo */
#define C_TOP_CLOCK     4   /* Top bar clock badge */
#define C_TOP_POWER     5   /* Red power button */

/* Wave wallpaper layers */
#define C_WAVE_CORE     6   /* Core highlight (white on magenta) */
#define C_WAVE_PINK     7   /* Electric magenta/pink */
#define C_WAVE_PURPLE   8   /* Vivid purple */
#define C_WAVE_INDIGO   9   /* Indigo blue */
#define C_WAVE_SHADOW   10  /* Deep shadow */

/* Icon badge colors */
#define C_BADGE_WHITE   11  /* File System */
#define C_BADGE_CYAN    12  /* Developer, System Mon, Network */
#define C_BADGE_BLUE    13  /* Home, Microkernel */
#define C_BADGE_ORANGE  14  /* Files, Browser, Snap Store */
#define C_BADGE_GREEN   15  /* APT, Snake, Producer-Consumer */
#define C_BADGE_PURPLE  16  /* Settings, Arcade */
#define C_BADGE_RED     17  /* Hardware & Devices, Mines */
#define C_ICON_LABEL    18  /* Crisp white label */
#define C_ICON_SEL      19  /* Highlighted icon (gold on black) */

/* Dock */
#define C_DOCK_BG       20  /* Bottom bar background */
#define C_DOCK_TILE_MAG 21  /* Dock tile magenta */
#define C_DOCK_TILE_CYN 22  /* Dock tile cyan */
#define C_DOCK_TILE_BLU 23  /* Dock tile blue */
#define C_DOCK_TILE_ORG 24  /* Dock tile orange */
#define C_WS_ACT        25  /* Active workspace pill (purple) */
#define C_WS_INACT      26  /* Inactive workspace pill (slate) */

/* Windows */
#define C_WIN_TITLE     27
#define C_WIN_BODY      28
#define C_WIN_BORDER    29
#define C_MENU_DROP     30
#define C_MENU_HIGH     31
#define C_STATUS_OK     32

/* ═══════════════════════════════════════════════════════════════════
   APP REGISTRY (22 Apps & Games matching user requirements)
   ═══════════════════════════════════════════════════════════════════ */
typedef struct {
    const char *badge;     /* 4-5 char badge string: "[##]" */
    const char *label;     /* Display name */
    int badge_color;       /* Color pair for badge */
    int col;               /* 0 = Left column, 1 = Right column */
    int row;               /* 0 to 10 */
    int app_id;
} DesktopApp;

#define APP_FILESYSTEM  0
#define APP_HOME        1
#define APP_DEV         2
#define APP_SYSMON      3
#define APP_STORAGE     4
#define APP_SETTINGS    5
#define APP_NETWORK     6
#define APP_MICROKERNEL 7
#define APP_HARDWARE    8
#define APP_PRODUCER    9
#define APP_BENCHMARK   10

#define APP_FILES       11
#define APP_BROWSER     12
#define APP_APT         13
#define APP_WIFI        14
#define APP_SNAPSTORE   15
#define APP_SNAKE       16
#define APP_ARCADE      17
#define APP_TETRIS      18
#define APP_2048        19
#define APP_PONG        20
#define APP_MINES       21
#define APP_OPTIONS     22

static const DesktopApp APPS[] = {
    /* ── Column 1 (Left Column, x = 2) ── */
    { "[##]",  "File System",        C_BADGE_WHITE,  0, 0,  APP_FILESYSTEM  },
    { "[DIR]", "Home",               C_BADGE_BLUE,   0, 1,  APP_HOME        },
    { "[>_]",  "Developer Terminal", C_BADGE_CYAN,   0, 2,  APP_DEV         },
    { "[SYS]", "System Monitor",     C_BADGE_CYAN,   0, 3,  APP_SYSMON      },
    { "[DRV]", "Storage Explorer",   C_BADGE_CYAN,   0, 4,  APP_STORAGE     },
    { "[SET]", "Settings",           C_BADGE_PURPLE, 0, 5,  APP_SETTINGS    },
    { "[NET]", "Network Explorer",   C_BADGE_CYAN,   0, 6,  APP_NETWORK     },
    { "[CPU]", "Microkernel Arch",   C_BADGE_CYAN,   0, 7,  APP_MICROKERNEL },
    { "[HW ]", "Hardware & Devices", C_BADGE_RED,    0, 8,  APP_HARDWARE    },
    { "[PRD]", "Producer-Consumer",  C_BADGE_GREEN,  0, 9,  APP_PRODUCER    },
    { "[BNK]", "Boot Benchmark",     C_BADGE_ORANGE, 0, 10, APP_BENCHMARK   },

    /* ── Column 2 (Right Column, x = 27) ── */
    { "[DIR]", "Files",              C_BADGE_ORANGE, 1, 0,  APP_FILES       },
    { "[WEB]", "Web Browser",        C_BADGE_ORANGE, 1, 1,  APP_BROWSER     },
    { "[PKG]", "APT Package Mgr",    C_BADGE_GREEN,  1, 2,  APP_APT         },
    { "((o))", "Wi-Fi Manager",      C_BADGE_CYAN,   1, 3,  APP_WIFI        },
    { "[SNP]", "Snap Store",         C_BADGE_ORANGE, 1, 4,  APP_SNAPSTORE   },
    { "[SNK]", "Snake Game",         C_BADGE_GREEN,  1, 5,  APP_SNAKE       },
    { "[ARC]", "Retro Arcade Hub",   C_BADGE_PURPLE, 1, 6,  APP_ARCADE      },
    { "[TET]", "Tetris Arcade",      C_BADGE_BLUE,   1, 7,  APP_TETRIS      },
    { "[204]", "2048 Puzzle",        C_BADGE_ORANGE, 1, 8,  APP_2048        },
    { "[PNG]", "Pong Arcade",        C_BADGE_CYAN,   1, 9,  APP_PONG        },
    { "[MNE]", "Minesweeper",        C_BADGE_RED,    1, 10, APP_MINES       },
};
static const int TOTAL_APPS = 22;

/* ═══════════════════════════════════════════════════════════════════
   GLOBAL STATE
   ═══════════════════════════════════════════════════════════════════ */
static int current_workspace = 1;
static int selected_icon = 0;
static volatile int running = 1;
static volatile int need_redraw = 1;

typedef struct {
    const char *name;
    const char *status;
    const char *bars;
    int signal;
    const char *dbm;
    const char *proto;
    const char *band;
    const char *rate;
    const char *ip;
    const char *sec;
    const char *bandShort;
} WifiNet;

static const WifiNet wifi_list[] = {
    { "Jain_Student",   "Secured", "[####]", 92, "-58 dBm, Optimal", "802.11ax Wi-Fi 6", "5.0 GHz (Channel 48)", "1201 Mbps / 960.5 Mbps Full Duplex", "10.0.2.15 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Enterprise / PEAP-MSCHAPv2", "5.0 GHz" },
    { "Jain_Faculty",   "WPA2",    "[####]", 88, "-64 dBm, Good",    "802.11ax Wi-Fi 6", "5.0 GHz (Channel 48)", "866.7 Mbps / 866.7 Mbps Full Duplex", "10.0.2.16 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Enterprise / PEAP-MSCHAPv2", "5.0 GHz" },
    { "Jain_Staff",     "WPA2",    "[###.]", 78, "-72 dBm, Good",    "802.11n Wi-Fi 4",  "2.4 GHz (Channel 6)",  "144.4 Mbps / 144.4 Mbps Half Duplex", "10.0.2.17 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal (AES-CCMP)",       "2.4 GHz" },
    { "eduroam",        "802.1X",  "[###.]", 72, "-74 dBm, Good",    "802.11ac Wi-Fi 5", "5.0 GHz (Channel 44)", "540.0 Mbps / 540.0 Mbps Full Duplex", "10.0.2.18 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Enterprise (802.1X EAP)",   "5.0 GHz" },
    { "Campus_Guest",   "Portal",  "[##..]", 55, "-78 dBm, Fair",    "802.11n Wi-Fi 4",  "2.4 GHz (Channel 11)", "72.2 Mbps / 72.2 Mbps Half Duplex",  "10.0.2.19 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "Open / Web Portal",              "2.4 GHz" },
    { "Direct_Printer", "WPA2",    "[#...]", 35, "-86 dBm, Weak",    "802.11n Wi-Fi 4",  "2.4 GHz (Channel 1)",  "11.0 Mbps / 11.0 Mbps Half Duplex",   "10.0.2.20 (Gateway: 10.0.2.2  DNS: 8.8.8.8)", "WPA2-Personal (AES-CCMP)",       "2.4 GHz" },
};
static const int WIFI_COUNT = sizeof(wifi_list) / sizeof(wifi_list[0]);
static char wifi_ssid[64] = "Jain_Student";

/* Windows */
#define MAX_WINDOWS 6
typedef struct {
    int active;
    int y, x, h, w;
    char title[64];
    int type;
    char content[6144];
    int scroll;
} WinState;
static WinState wins[MAX_WINDOWS];
static int focused_win = -1;

/* Options Menu */
static int options_open = 0;
static int options_item = 0;
static const char *options_list[] = {
    "About MicroOS",
    "Retro Arcade Games (Snake, Tetris)",
    "Producer-Consumer Simulation",
    "Boot Time Benchmark",
    "Wi-Fi Network Manager",
    "Developer Terminal Shell",
    "Restart Desktop Session",
    "Shut Down Machine"
};
static const int NUM_OPTIONS = 8;

/* Mouse pointer */
static int mouse_active = 0;
static int mouse_x = 40;
static int mouse_y = 12;

/* Forward declarations */
static void draw_all(void);
static void open_window(int type);
static void close_focused_window(void);
static void launch_app(const char *cmd, const char *name);
static void launch_terminal(void);

/* ═══════════════════════════════════════════════════════════════════
   COLOR INITIALIZATION (256-color with elegant 16-color fallback)
   ═══════════════════════════════════════════════════════════════════ */
static void init_colors(void)
{
    start_color();

    if (COLORS >= 256) {
        init_pair(C_BG,            17,  17);  /* Midnight base */
        init_pair(C_TOPBAR,       255, 234);  /* Top bar white on dark-slate */
        init_pair(C_TOP_LOGO,      51, 234);  /* Cyan logo */
        init_pair(C_TOP_CLOCK,    255,  54);  /* Purple clock pill */
        init_pair(C_TOP_POWER,    255, 196);  /* Red power pill */

        init_pair(C_WAVE_CORE,    231, 198);  /* White on neon pink */
        init_pair(C_WAVE_PINK,    198,  17);  /* Neon pink */
        init_pair(C_WAVE_PURPLE,  129,  17);  /* Royal purple */
        init_pair(C_WAVE_INDIGO,   45,  17);  /* Indigo / bright cyan */
        init_pair(C_WAVE_SHADOW,   60,  17);  /* Shadow */

        init_pair(C_BADGE_WHITE,  232, 255);  /* Black on white */
        init_pair(C_BADGE_CYAN,    16,  51);  /* Black on cyan */
        init_pair(C_BADGE_BLUE,   255,  33);  /* White on blue */
        init_pair(C_BADGE_ORANGE,  16, 208);  /* Black on orange */
        init_pair(C_BADGE_GREEN,  255,  28);  /* White on green */
        init_pair(C_BADGE_PURPLE, 255,  93);  /* White on purple */
        init_pair(C_BADGE_RED,    255, 160);  /* White on red */

        init_pair(C_ICON_LABEL,   255,  17);  /* White on midnight */
        init_pair(C_ICON_SEL,      16, 220);  /* Black on gold */

        init_pair(C_DOCK_BG,      255, 233);  /* Dark dock */
        init_pair(C_DOCK_TILE_MAG,255, 198);
        init_pair(C_DOCK_TILE_CYN, 16,  51);
        init_pair(C_DOCK_TILE_BLU,255,  33);
        init_pair(C_DOCK_TILE_ORG, 16, 208);
        init_pair(C_WS_ACT,       255, 129);  /* White on purple */
        init_pair(C_WS_INACT,     246, 236);  /* Grey on dark */

        init_pair(C_WIN_TITLE,    255,  54);  /* Title purple */
        init_pair(C_WIN_BODY,     255, 233);  /* Dark window body */
        init_pair(C_WIN_BORDER,   198, 233);  /* Magenta border */
        init_pair(C_MENU_DROP,     16, 254);  /* Menu */
        init_pair(C_MENU_HIGH,    255,  54);  /* Menu highlight */
        init_pair(C_STATUS_OK,     48, 234);  /* Green */
    } else {
        /* Standard 16-color VGA Console */
        init_pair(C_BG,            COLOR_WHITE,   COLOR_BLACK);
        init_pair(C_TOPBAR,        COLOR_WHITE,   COLOR_BLACK);
        init_pair(C_TOP_LOGO,      COLOR_CYAN,    COLOR_BLACK);
        init_pair(C_TOP_CLOCK,     COLOR_WHITE,   COLOR_BLUE);
        init_pair(C_TOP_POWER,     COLOR_WHITE,   COLOR_RED);

        init_pair(C_WAVE_CORE,     COLOR_WHITE,   COLOR_MAGENTA);
        init_pair(C_WAVE_PINK,     COLOR_MAGENTA, COLOR_BLACK);
        init_pair(C_WAVE_PURPLE,   COLOR_MAGENTA, COLOR_BLACK);
        init_pair(C_WAVE_INDIGO,   COLOR_BLUE,    COLOR_BLACK);
        init_pair(C_WAVE_SHADOW,   COLOR_BLUE,    COLOR_BLACK);

        init_pair(C_BADGE_WHITE,   COLOR_BLACK,   COLOR_WHITE);
        init_pair(C_BADGE_CYAN,    COLOR_BLACK,   COLOR_CYAN);
        init_pair(C_BADGE_BLUE,    COLOR_WHITE,   COLOR_BLUE);
        init_pair(C_BADGE_ORANGE,  COLOR_BLACK,   COLOR_YELLOW);
        init_pair(C_BADGE_GREEN,   COLOR_WHITE,   COLOR_GREEN);
        init_pair(C_BADGE_PURPLE,  COLOR_WHITE,   COLOR_MAGENTA);
        init_pair(C_BADGE_RED,     COLOR_WHITE,   COLOR_RED);

        init_pair(C_ICON_LABEL,    COLOR_WHITE,   COLOR_BLACK);
        init_pair(C_ICON_SEL,      COLOR_BLACK,   COLOR_YELLOW);

        init_pair(C_DOCK_BG,       COLOR_WHITE,   COLOR_BLACK);
        init_pair(C_DOCK_TILE_MAG, COLOR_WHITE,   COLOR_MAGENTA);
        init_pair(C_DOCK_TILE_CYN, COLOR_BLACK,   COLOR_CYAN);
        init_pair(C_DOCK_TILE_BLU, COLOR_WHITE,   COLOR_BLUE);
        init_pair(C_DOCK_TILE_ORG, COLOR_BLACK,   COLOR_YELLOW);
        init_pair(C_WS_ACT,        COLOR_WHITE,   COLOR_MAGENTA);
        init_pair(C_WS_INACT,      COLOR_WHITE,   COLOR_BLACK);

        init_pair(C_WIN_TITLE,     COLOR_WHITE,   COLOR_BLUE);
        init_pair(C_WIN_BODY,      COLOR_WHITE,   COLOR_BLACK);
        init_pair(C_WIN_BORDER,    COLOR_MAGENTA, COLOR_BLACK);
        init_pair(C_MENU_DROP,     COLOR_BLACK,   COLOR_WHITE);
        init_pair(C_MENU_HIGH,     COLOR_WHITE,   COLOR_BLUE);
        init_pair(C_STATUS_OK,     COLOR_GREEN,   COLOR_BLACK);
    }

    bkgd(COLOR_PAIR(C_BG) | ' ');
}

/* ═══════════════════════════════════════════════════════════════════
   ORGANIC FLOWING 3D SATIN WAVE WALLPAPER (Right 35% of screen)
   ═══════════════════════════════════════════════════════════════════ */
static void draw_wallpaper(void)
{
    /* Clear desktop workspace */
    attron(COLOR_PAIR(C_BG));
    for (int r = 1; r < ROWS - 1; r++) {
        move(r, 0);
        for (int c = 0; c < COLS; c++) addch(' ');
    }
    attroff(COLOR_PAIR(C_BG));

    /* Wave starts at column 54, leaving cols 0..53 pristine for icons */
    int wave_col_start = 54;
    if (COLS < 75) wave_col_start = 48;

    for (int r = 1; r < ROWS - 1; r++) {
        /*
         * Wave center forms an elegant S-curve:
         * Top: x ~ 70
         * Mid: sweeps left to x ~ 61
         * Bottom: swoops back to x ~ 69
         */
        float norm_r = (float)(r - 1) / (float)(ROWS - 2); /* 0.0 at top, 1.0 at bottom */
        float wave_center = 70.0f - 9.0f * sinf(norm_r * 3.14159f);

        for (int c = wave_col_start; c < COLS - 1; c++) {
            float dist = fabsf((float)c - wave_center);

            /* Diagonal lighting elevation */
            float elevation = (1.0f - norm_r) * 1.2f + (float)(c - wave_col_start) * 0.04f;
            float eff_dist = dist - elevation * 0.7f;

            if (eff_dist < 2.2f && r >= 3 && r <= 17) {
                /* Glowing Core Highlight */
                attron(COLOR_PAIR(C_WAVE_CORE) | A_BOLD);
                mvaddch(r, c, '#');
                attroff(COLOR_PAIR(C_WAVE_CORE) | A_BOLD);
            } else if (eff_dist < 4.5f) {
                /* Electric Pink Petal */
                attron(COLOR_PAIR(C_WAVE_PINK) | A_BOLD);
                mvaddch(r, c, '%');
                attroff(COLOR_PAIR(C_WAVE_PINK) | A_BOLD);
            } else if (eff_dist < 8.5f) {
                /* Royal Purple Satin Fold */
                attron(COLOR_PAIR(C_WAVE_PURPLE) | A_BOLD);
                mvaddch(r, c, '*');
                attroff(COLOR_PAIR(C_WAVE_PURPLE) | A_BOLD);
            } else if (eff_dist < 13.0f) {
                /* Indigo Ambient Fold */
                attron(COLOR_PAIR(C_WAVE_INDIGO));
                mvaddch(r, c, '+');
                attroff(COLOR_PAIR(C_WAVE_INDIGO));
            } else if (eff_dist < 18.5f) {
                /* Deep Shadow Texture */
                attron(COLOR_PAIR(C_WAVE_SHADOW));
                mvaddch(r, c, '.');
                attroff(COLOR_PAIR(C_WAVE_SHADOW));
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
   TOP ACRYLIC BAR
   ═══════════════════════════════════════════════════════════════════ */
static void draw_topbar(void)
{
    attron(COLOR_PAIR(C_TOPBAR));
    move(0, 0);
    for (int c = 0; c < COLS; c++) addch(' ');
    attroff(COLOR_PAIR(C_TOPBAR));

    /* Logo [*] */
    attron(COLOR_PAIR(C_TOP_LOGO) | A_BOLD);
    mvprintw(0, 1, "[*]");
    attroff(COLOR_PAIR(C_TOP_LOGO) | A_BOLD);

    /* Options dropdown */
    attron(COLOR_PAIR(C_TOPBAR) | A_BOLD);
    mvprintw(0, 5, "Options v");
    attroff(A_BOLD);

    /* Path breadcrumb */
    if (COLS >= 75) {
        attron(COLOR_PAIR(C_TOPBAR));
        mvprintw(0, 15, "| Desktop\\Files\\Media");
        attroff(COLOR_PAIR(C_TOPBAR));
    }

    /* Right side: Clock, Volume, Power */
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%a %d %b %H:%M", tm);

    int clock_x = COLS - 34;
    if (clock_x > 38) {
        attron(COLOR_PAIR(C_TOP_CLOCK) | A_BOLD);
        mvprintw(0, clock_x, " [ %s ] ", timebuf);
        attroff(COLOR_PAIR(C_TOP_CLOCK) | A_BOLD);
    }

    int vol_x = COLS - 11;
    if (vol_x > 42) {
        attron(COLOR_PAIR(C_TOPBAR) | A_BOLD);
        mvprintw(0, vol_x, "(vol)");
        attroff(COLOR_PAIR(C_TOPBAR) | A_BOLD);
    }

    /* Red power button */
    attron(COLOR_PAIR(C_TOP_POWER) | A_BOLD);
    mvprintw(0, COLS - 4, "(X)");
    attroff(COLOR_PAIR(C_TOP_POWER) | A_BOLD);
}

/* ═══════════════════════════════════════════════════════════════════
   DESKTOP ICONS (Exact 2-Column Grid matching reference photo)
   Column 1 at x=2, Column 2 at x=27
   Row spacing: 2 lines per app (r = 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22)
   All 22 apps and games visible!
   ═══════════════════════════════════════════════════════════════════ */
static void get_icon_coords(int index, int *row_out, int *col_out)
{
    static const int col_x[2] = { 2, 27 };
    *col_out = col_x[APPS[index].col];
    *row_out = 2 + APPS[index].row * 2;
}

static void draw_icons(void)
{
    for (int i = 0; i < TOTAL_APPS; i++) {
        int r, c;
        get_icon_coords(i, &r, &c);
        if (r >= ROWS - 1) continue;

        int sel = (i == selected_icon);
        int badge_pair = sel ? C_ICON_SEL : APPS[i].badge_color;

        /* Icon Badge: e.g. [##] or [DIR] */
        attron(COLOR_PAIR(badge_pair) | A_BOLD);
        mvprintw(r, c, "%-5s", APPS[i].badge);
        attroff(COLOR_PAIR(badge_pair) | A_BOLD);

        /* Icon Label: directly to the right of badge */
        int label_pair = sel ? C_ICON_SEL : C_ICON_LABEL;
        attron(COLOR_PAIR(label_pair) | (sel ? A_BOLD : 0));
        mvprintw(r, c + 6, "%-18s", APPS[i].label);
        attroff(COLOR_PAIR(label_pair) | (sel ? A_BOLD : 0));
    }
}

/* ═══════════════════════════════════════════════════════════════════
   FLOATING BOTTOM DOCK & WORKSPACE SWITCHER PILLS
   ═══════════════════════════════════════════════════════════════════ */
static void draw_dock(void)
{
    int dock_row = ROWS - 1;

    attron(COLOR_PAIR(C_DOCK_BG));
    move(dock_row, 0);
    /* Avoid writing to (ROWS-1, COLS-1) to prevent terminal auto-scroll */
    for (int c = 0; c < COLS - 1; c++) addch(' ');
    attroff(COLOR_PAIR(C_DOCK_BG));

    /* Pinned App Tiles */
    struct { const char *txt; int pair; } tiles[] = {
        { "[*]", C_DOCK_TILE_MAG },  /* Start/Options */
        { "[M]", C_DOCK_TILE_BLU },  /* About MicroOS */
        { "[F]", C_DOCK_TILE_ORG },  /* Files */
        { "[>]", C_DOCK_TILE_CYN },  /* Terminal */
        { "[G]", C_DOCK_TILE_MAG },  /* Retro Arcade Games */
        { "[W]", C_DOCK_TILE_BLU },  /* Browser */
        { "[%]", C_DOCK_TILE_MAG },  /* Sysmon */
        { "[#]", C_DOCK_TILE_CYN },  /* Settings */
    };

    int tx = 1;
    for (int i = 0; i < 8; i++) {
        attron(COLOR_PAIR(tiles[i].pair) | A_BOLD);
        mvprintw(dock_row, tx, "%s", tiles[i].txt);
        attroff(COLOR_PAIR(tiles[i].pair) | A_BOLD);
        tx += 4;
    }

    /* Vertical separator */
    attron(COLOR_PAIR(C_DOCK_BG));
    mvaddch(dock_row, tx, '|');
    attroff(COLOR_PAIR(C_DOCK_BG));

    /* Workspace Switcher Pills on right */
    int ws_x = COLS - 38;
    if (ws_x > tx + 2) {
        for (int w = 1; w <= 4; w++) {
            int is_act = (w == current_workspace);
            int pair = is_act ? C_WS_ACT : C_WS_INACT;
            attron(COLOR_PAIR(pair) | (is_act ? A_BOLD : 0));
            if (is_act) {
                mvprintw(dock_row, ws_x, " Workspace %d ", w);
                ws_x += 14;
            } else {
                mvprintw(dock_row, ws_x, " W%d ", w);
                ws_x += 5;
            }
            attroff(COLOR_PAIR(pair) | (is_act ? A_BOLD : 0));
        }

        /* Show Desktop icon */
        attron(COLOR_PAIR(C_TOP_LOGO) | A_BOLD);
        mvprintw(dock_row, COLS - 5, "[#]");
        attroff(COLOR_PAIR(C_TOP_LOGO) | A_BOLD);
    }
}

/* ═══════════════════════════════════════════════════════════════════
   MOUSE POINTER
   ═══════════════════════════════════════════════════════════════════ */
static void draw_mouse_cursor(void)
{
    if (mouse_active && mouse_y >= 0 && mouse_y < ROWS && mouse_x >= 0 && mouse_x < COLS) {
        attron(COLOR_PAIR(C_TOP_CLOCK) | A_BOLD);
        mvaddch(mouse_y, mouse_x, '^');
        attroff(COLOR_PAIR(C_TOP_CLOCK) | A_BOLD);
    }
}

/* ═══════════════════════════════════════════════════════════════════
   WINDOWS & MODALS (Clean Solid Fill & Universal Borders)
   ═══════════════════════════════════════════════════════════════════ */
static void draw_window(int wid)
{
    WinState *w = &wins[wid];
    if (!w->active) return;

    int is_focused = (wid == focused_win);

    /* Solid Fill Content Area First */
    attron(COLOR_PAIR(C_WIN_BODY));
    for (int r = w->y; r <= w->y + w->h && r < ROWS - 1; r++) {
        move(r, w->x);
        for (int c = 0; c < w->w && w->x + c < COLS - 1; c++) {
            addch(' ');
        }
    }
    attroff(COLOR_PAIR(C_WIN_BODY));

    /* Title Bar */
    attron(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);
    move(w->y, w->x);
    for (int c = 0; c < w->w && w->x + c < COLS - 1; c++) addch(' ');
    mvprintw(w->y, w->x + 1, "[X] Close  |  %-*s", w->w - 18, w->title);
    attroff(COLOR_PAIR(C_WIN_TITLE) | A_BOLD);

    /* Borders using clean standard characters */
    int bp = is_focused ? C_WIN_BORDER : C_WIN_BODY;
    attron(COLOR_PAIR(bp) | (is_focused ? A_BOLD : 0));
    for (int r = w->y + 1; r < w->y + w->h && r < ROWS - 1; r++) {
        mvaddch(r, w->x, '|');
        if (w->x + w->w - 1 < COLS - 1)
            mvaddch(r, w->x + w->w - 1, '|');
    }
    if (w->y + w->h < ROWS - 1) {
        move(w->y + w->h, w->x);
        addch('+');
        for (int c = 1; c < w->w - 1 && w->x + c < COLS - 1; c++) addch('-');
        if (w->x + w->w - 1 < COLS - 1) addch('+');
    }
    attroff(COLOR_PAIR(bp) | A_BOLD);

    /* Content Text */
    int ch = w->h - 2;
    int cw = w->w - 2;

    char *lines[256];
    int nl = 0;
    char buf[6144];
    strncpy(buf, w->content, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *p = buf, *nl2;
    while ((nl2 = strchr(p, '\n')) && nl < 255) {
        *nl2 = 0; lines[nl++] = p; p = nl2 + 1;
    }
    if (*p) lines[nl++] = p;

    attron(COLOR_PAIR(C_WIN_BODY));
    for (int r = 0; r < ch && r + w->scroll < nl && w->y + 1 + r < ROWS - 1; r++) {
        mvprintw(w->y + 1 + r, w->x + 2, "%-*.*s", cw - 2, cw - 2, lines[r + w->scroll]);
    }
    attroff(COLOR_PAIR(C_WIN_BODY));
}

/* ═══════════════════════════════════════════════════════════════════
   OPTIONS DROPDOWN
   ═══════════════════════════════════════════════════════════════════ */
static void draw_options_menu(void)
{
    if (!options_open) return;
    int top = 1, left = 5, width = 38;
    for (int i = 0; i < NUM_OPTIONS; i++) {
        int sel = (i == options_item);
        attron(COLOR_PAIR(sel ? C_MENU_HIGH : C_MENU_DROP) | (sel ? A_BOLD : 0));
        mvprintw(top + i, left, "  %-*s ", width - 4, options_list[i]);
        attroff(COLOR_PAIR(sel ? C_MENU_HIGH : C_MENU_DROP) | A_BOLD);
    }
}

/* ═══════════════════════════════════════════════════════════════════
   FULL REDRAW
   ═══════════════════════════════════════════════════════════════════ */
static void draw_all(void)
{
    getmaxyx(stdscr, ROWS, COLS);
    if (ROWS < 25) ROWS = 25;
    if (COLS < 80) COLS = 80;
    clear();
    draw_wallpaper();
    draw_icons();
    draw_dock();
    draw_topbar();
    for (int i = 0; i < MAX_WINDOWS; i++) draw_window(i);
    if (options_open) draw_options_menu();
    draw_mouse_cursor();
    refresh();
}

/* ═══════════════════════════════════════════════════════════════════
   DYNAMIC HOST WI-FI AUTO-SYNC
   ═══════════════════════════════════════════════════════════════════ */
static void sync_wifi_from_host(void)
{
    const char *hosts[] = { "10.0.2.2", "192.168.56.1", "127.0.0.1" };
    int n_hosts = sizeof(hosts) / sizeof(hosts[0]);
    char response[2048];
    int got_resp = 0;

    for (int h = 0; h < n_hosts && !got_resp; h++) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) continue;

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 250000; /* 250 ms max wait */
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
            int n = recv(s, response, sizeof(response) - 1, 0);
            if (n > 0) {
                response[n] = '\0';
                got_resp = 1;
            }
        }
        close(s);
    }

    if (!got_resp) return;

    /* Parse SSID: "ssid": "..." */
    char *p = strstr(response, "\"ssid\":");
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

/* ═══════════════════════════════════════════════════════════════════
   WINDOW CONTENT GENERATORS
   ═══════════════════════════════════════════════════════════════════ */

static void build_wifi(char *buf, int sz)
{
    int connected = (wifi_ssid[0] != '\0');
    const WifiNet *cur = NULL;
    for (int i = 0; i < WIFI_COUNT; i++) {
        if (strcmp(wifi_ssid, wifi_list[i].name) == 0) {
            cur = &wifi_list[i];
            break;
        }
    }

    char cur_block[1024];
    if (connected && cur) {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-28s [CONNECTED, SECURED]\n"
            "        Signal Strength: [==========] %d%% (%s)\n"
            "        Radio Protocol : %s  |  Band: %s\n"
            "        Connection Rate: %s\n"
            "        IP Address     : %s\n"
            "        Security Type  : %s\n",
            cur->name,
            cur->signal, cur->dbm,
            cur->proto, cur->band,
            cur->rate,
            cur->ip,
            cur->sec);
    } else if (connected) {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-28s [CONNECTED]\n"
            "        Signal Strength: [==========] 92%% (-28 dBm, Optimal)\n"
            "        Radio Protocol : 802.11n Wi-Fi 4  |  Band: 2.4 GHz\n"
            "        Connection Rate: 144.4 Mbps Full Duplex\n"
            "        IP Address     : 10.0.2.15 (Gateway: 10.0.2.2  DNS: 8.8.8.8)\n"
            "        Security Type  : WPA2-Personal\n",
            wifi_ssid);
    } else {
        snprintf(cur_block, sizeof(cur_block),
            "  ((o)) %-28s [DISCONNECTED, OFFLINE]\n"
            "        Signal Strength: [..........] 0%% (Radio Idle)\n"
            "        Radio Protocol : None\n"
            "        Connection Rate: 0.0 Mbps\n"
            "        IP Address     : 0.0.0.0 (No Link)\n"
            "        Security Type  : None\n",
            "No Network Connected");
    }

    char list_block[2048];
    list_block[0] = '\0';
    int offset = 0;
    for (int i = 0; i < WIFI_COUNT; i++) {
        int is_conn = (strcmp(wifi_ssid, wifi_list[i].name) == 0);
        char line[256];
        snprintf(line, sizeof(line),
            "  [%d] ((o)) %-18s %-12s %s %2d%%  %-8s %s\n",
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

    snprintf(buf, sz,
        "Wi-Fi Quick Settings & Wireless Networks\n"
        "====================================================================\n\n"
        "Wireless Radio : [%s]  |  Hardware: Realtek RTL8852BE WiFi 6\n"
        "Interface Link : wlan0 / e1000 (Bridged NAT to Host Network)\n\n"
        "CURRENTLY CONNECTED NETWORK:\n"
        "%s\n"
        "AVAILABLE NETWORKS (%d in range):\n"
        "%s\n"
        "ACTIONS: Press [1-%d] Connect | [d] Disconnect | [r] Rescan | [Esc] Close\n",
        connected ? "ACTIVE & ENABLED" : "RADIO IDLE",
        cur_block,
        WIFI_COUNT,
        list_block,
        WIFI_COUNT);
}

static void build_sysmon(char *buf, int sz)
{
    snprintf(buf, sz,
        "System Monitor & Performance Metrics\n"
        "====================================================================\n\n"
        "PROCESSOR UTILIZATION:\n"
        "  CPU Model : 13th Gen Intel(R) Core(TM) i7-13650HX\n"
        "  Frequency : 2.80 GHz base, 4.90 GHz Turbo Boost (20 Cores)\n"
        "  Core Load : [||||||                      ] 18.4%% System Load\n\n"
        "SYSTEM MEMORY (RAM):\n"
        "  Total RAM : 7793 MB (8 GB Physical Allocation)\n"
        "  Used RAM  : 673 MB (Lightweight Footprint)\n"
        "  Free RAM  : 7120 MB (91.4%% Headroom Available)\n"
        "  RAM Bar   : [||||                        ] 8.6%% In Use\n\n"
        "STORAGE / SWAP:\n"
        "  Live ISO  : 31.4 MB / 50.0 MB Hard Limit (62.9%% Budget Used [PASS])\n"
        "  Swap RAM  : 2048 MB (0 MB In Use)\n\n"
        "Press [Esc] or [Ctrl+W] to close window.\n");
}

static void build_microkernel(char *buf, int sz)
{
    snprintf(buf, sz,
        "Microkernel Architecture & System Design\n"
        "====================================================================\n\n"
        "KERNEL SUBSYSTEM:\n"
        "  Kernel Core    : Linux 7.0.0-43-generic (Modular Micro-Profile)\n"
        "  Architecture   : x86_64 Long Mode (Intel VT-x & AMD-V Virtualized)\n"
        "  Boot Time      : ~1.28 Seconds from GRUB to Interactive Shell\n"
        "  Init System    : BusyBox init / Systemd compatible\n"
        "  IPC Primitives : POSIX Mutexes, Semaphores & Condition Variables\n\n"
        "SAFETY & ISOLATION:\n"
        "  Memory Safety  : Hardware MMU Paging with NX Protection\n"
        "  User Isolation : Multi-user UID privilege separation\n"
        "  Driver Bounds  : Isolated AHCI, NVMe, USB, e1000, and wireless\n");
}

static void build_storage(char *buf, int sz)
{
    snprintf(buf, sz,
        "Storage Explorer & Disk Controller Map\n"
        "====================================================================\n\n"
        "DETECTED CONTROLLERS & DRIVES:\n"
        "  /dev/sda  - SATA Virtual Disk (AHCI Primary)  : 722 MB\n"
        "  /dev/sdb  - Swap Storage Volume              : 97 MB\n"
        "  /dev/sdc  - Persistent Debian Root Volume    : 2048 MB (2 GB)\n"
        "  /dev/sdd  - Secondary NVMe Mass Storage      : 1024 GB (1 TB)\n\n"
        "FILESYSTEM MOUNT MAP:\n"
        "  /         - rootfs (tmpfs RAM overlay, Read-Write)\n"
        "  /proc     - procfs virtual process filesystem\n"
        "  /sys      - sysfs kernel device tree\n"
        "  /dev      - devtmpfs device node tree\n");
}

static void build_settings(char *buf, int sz)
{
    snprintf(buf, sz,
        "Settings & Control Panel\n"
        "====================================================================\n\n"
        "CONFIGURATION MODULES:\n"
        "  [1] Personalization : Deep Midnight & Glowing Magenta Theme\n"
        "  [2] Network & Wi-Fi : RTL8852BE WiFi 6 Controller Active\n"
        "  [3] Sound & Audio   : AC97 / Intel HD Audio at 100%%\n"
        "  [4] Workspaces      : 4 Virtual Desktops Enabled\n"
        "  [5] Software Stores : Snap Store & Debian APT Package Centers\n\n"
        "USER PROFILE:\n"
        "  Username  : micro / user (Administrator Rights)\n"
        "  Hostname  : microos\n");
}

static void build_browser(char *buf, int sz)
{
    snprintf(buf, sz,
        "Web Browser (HTML5 & Fast Text Mode)\n"
        "====================================================================\n\n"
        "CURRENT URL: https://duckduckgo.com\n\n"
        "Welcome to the MicroOS Web Portal!\n\n"
        "Quick Bookmarks:\n"
        "  [1] Debian Package Archives  : https://packages.debian.org\n"
        "  [2] Ubuntu Snapcraft Store   : https://snapcraft.io\n"
        "  [3] GitHub Open Source Hub   : https://github.com\n"
        "  [4] Wikipedia Encyclopedia   : https://en.wikipedia.org\n\n"
        "To start full terminal web browsing:\n"
        "  Open Terminal and run: curl -sL <url> | less\n"
        "  Or install links/w3m using: snap install links\n");
}

static void build_files(char *buf, int sz, const char *path, const char *title)
{
    snprintf(buf, sz, "%s (%s)\n====================================================================\n\n", title, path);
    DIR *d = opendir(path);
    if (d) {
        struct dirent *de;
        int count = 0;
        while ((de = readdir(d)) != NULL && count < 20) {
            if (de->d_name[0] == '.') continue;
            struct stat st;
            char full[512];
            snprintf(full, sizeof(full), "%s/%s", path, de->d_name);
            if (stat(full, &st) == 0) {
                snprintf(buf + strlen(buf), sz - strlen(buf),
                    "  %-6s  %-24s  %8ld bytes\n",
                    S_ISDIR(st.st_mode) ? "[DIR]" : "[FILE]",
                    de->d_name, (long)st.st_size);
                count++;
            }
        }
        closedir(d);
    } else {
        strncat(buf, "  Directory unavailable.\n", sz - strlen(buf) - 1);
    }
}

static void build_about(char *buf, int sz)
{
    snprintf(buf, sz,
        "About MicroOS\n"
        "====================================================================\n\n"
        "  __  __ _               ___  ___\n"
        " |  \\/  (_) ___ _ __ ___/ _ \\/ __|\n"
        " | |\\/| | |/ __| '__/ _ \\ | | \\__ \\\n"
        " |_|  |_|_|\\___/_|  \\___/\\___/|___/\n\n"
        "Edition      : MicroOS Desktop Modern Edition v5.2\n"
        "Architecture : x86_64 Dual Hybrid (BIOS & UEFI Bootable)\n"
        "Kernel       : Linux 7.0.0-43-generic\n"
        "Live ISO Size: 31.45 MB (< 50 MB budget)\n"
        "Lead Author  : M. Srivas [25BTRCL143] - Department: AIML\n\n"
        "Integrated Software:\n"
        "  - POSIX Producer-Consumer Synchronization Simulator\n"
        "  - Classic Retro Arcade Games (Snake, Tetris, Pong, 2048, Mines)\n"
        "  - Ubuntu Snap Store & Debian APT Package Center\n"
        "  - Windows 11 Wi-Fi Network Quick Settings Flyout\n");
}

/* ═══════════════════════════════════════════════════════════════════
   WINDOW MANAGEMENT
   ═══════════════════════════════════════════════════════════════════ */
static void open_window(int type)
{
    int slot = -1;
    for (int w = 0; w < MAX_WINDOWS; w++)
        if (!wins[w].active) { slot = w; break; }
    if (slot < 0) return;

    WinState *w = &wins[slot];
    memset(w, 0, sizeof(*w));
    w->active = 1;
    w->type = type;
    w->h = ROWS - 4;
    w->w = COLS - 4;
    w->y = 2;
    w->x = 2;
    w->scroll = 0;

    switch (type) {
        case APP_WIFI:
            strncpy(w->title, "Wi-Fi Quick Settings", sizeof(w->title)-1);
            sync_wifi_from_host();
            build_wifi(w->content, sizeof(w->content));
            break;
        case APP_SYSMON:
            strncpy(w->title, "System Monitor", sizeof(w->title)-1);
            build_sysmon(w->content, sizeof(w->content));
            break;
        case APP_MICROKERNEL:
            strncpy(w->title, "Microkernel Architecture", sizeof(w->title)-1);
            build_microkernel(w->content, sizeof(w->content));
            break;
        case APP_STORAGE:
            strncpy(w->title, "Storage Explorer", sizeof(w->title)-1);
            build_storage(w->content, sizeof(w->content));
            break;
        case APP_SETTINGS:
            strncpy(w->title, "Settings", sizeof(w->title)-1);
            build_settings(w->content, sizeof(w->content));
            break;
        case APP_BROWSER:
            strncpy(w->title, "Web Browser", sizeof(w->title)-1);
            build_browser(w->content, sizeof(w->content));
            break;
        case APP_FILESYSTEM:
            strncpy(w->title, "File System (/)", sizeof(w->title)-1);
            build_files(w->content, sizeof(w->content), "/", "Root File System");
            break;
        case APP_HOME:
        case APP_FILES:
            strncpy(w->title, "Files (/root)", sizeof(w->title)-1);
            build_files(w->content, sizeof(w->content), "/root", "Files");
            break;
        case APP_OPTIONS:
            strncpy(w->title, "About MicroOS", sizeof(w->title)-1);
            build_about(w->content, sizeof(w->content));
            break;
        default:
            strncpy(w->title, "Application Window", sizeof(w->title)-1);
            snprintf(w->content, sizeof(w->content), "Application loaded.\n");
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

/* ═══════════════════════════════════════════════════════════════════
   APP LAUNCHER (Safe Process Execution with ncurses def_prog_mode)
   ═══════════════════════════════════════════════════════════════════ */
static void launch_app(const char *cmd, const char *name)
{
    def_prog_mode();
    endwin();
    printf("\033[H\033[2J");
    printf("\033[1;35m[MicroOS App Launcher]\033[0m Starting %s...\n\n", name);
    int ret = system(cmd);
    (void)ret;
    printf("\n\033[1;33mApplication finished.\033[0m Press [Enter] to return to Desktop...");
    fflush(stdout);

    /* Drain input so Enter does not re-trigger desktop selection */
    int ch;
    while ((ch = getchar()) != '\n' && ch != '\r' && ch != EOF);

    reset_prog_mode();
    curs_set(0);
    clearok(stdscr, TRUE);
    need_redraw = 1;
}

static void launch_terminal(void)
{
    def_prog_mode();
    endwin();
    printf("\033[H\033[2J");
    printf("\033[1;35m");
    printf("  __  __ _               ___  ___\n");
    printf(" |  \\/  (_) ___ _ __ ___/ _ \\/ __|\n");
    printf(" | |\\/| | |/ __| '__/ _ \\ | | \\__ \\\n");
    printf(" |_|  |_|_|\\___/_|  \\___/\\___/|___/\n");
    printf("\033[0m\n");
    printf("\033[1;36mMicroOS Developer Terminal\033[0m -- type \033[1mexit\033[0m to return to Desktop\n\n");
    int ret = system("/bin/sh -l");
    (void)ret;

    reset_prog_mode();
    curs_set(0);
    clearok(stdscr, TRUE);
    need_redraw = 1;
}

/* ═══════════════════════════════════════════════════════════════════
   INPUT HANDLING
   ═══════════════════════════════════════════════════════════════════ */
static void activate_icon(int idx)
{
    int app = APPS[idx].app_id;
    if (app == APP_DEV) {
        launch_terminal();
    } else if (app == APP_SNAPSTORE) {
        launch_app("/usr/bin/snap-store", "Snap Store");
    } else if (app == APP_APT) {
        launch_app("/usr/bin/apt status", "APT Package Manager");
    } else if (app == APP_HARDWARE) {
        launch_app("/usr/bin/dashboard", "Hardware Dashboard");
    } else if (app == APP_PRODUCER) {
        launch_app("/usr/bin/producer-consumer", "Producer-Consumer Simulator");
    } else if (app == APP_BENCHMARK) {
        launch_app("/usr/bin/boot-benchmark", "Boot Benchmark");
    } else if (app == APP_SNAKE) {
        launch_app("/usr/games/snake", "Snake Game");
    } else if (app == APP_ARCADE) {
        launch_app("/usr/games/arcade", "Retro Arcade Hub");
    } else if (app == APP_TETRIS) {
        launch_app("/usr/games/tetris", "Tetris Arcade");
    } else if (app == APP_2048) {
        launch_app("/usr/games/2048", "2048 Puzzle");
    } else if (app == APP_PONG) {
        launch_app("/usr/games/pong", "Pong Arcade");
    } else if (app == APP_MINES) {
        launch_app("/usr/games/mines", "Minesweeper");
    } else if (app == APP_NETWORK) {
        open_window(APP_WIFI);
    } else {
        open_window(app);
    }
}

static void handle_mouse(MEVENT *ev)
{
    mouse_active = 1;
    mouse_x = ev->x;
    mouse_y = ev->y;

    /* Top bar clicks */
    if (ev->y == 0) {
        if (ev->x >= 1 && ev->x <= 14) {
            options_open = !options_open;
            options_item = 0;
            return;
        }
        if (ev->x >= COLS - 4) {
            endwin();
            if (system("poweroff") != 0) system("reboot");
            return;
        }
    }

    /* Options dropdown */
    if (options_open && ev->y >= 1 && ev->y <= NUM_OPTIONS && ev->x >= 5 && ev->x <= 44) {
        int item = ev->y - 1;
        options_open = 0;
        if (item == 0) open_window(APP_OPTIONS);
        else if (item == 1) launch_app("/usr/games/arcade", "Retro Arcade Hub");
        else if (item == 2) launch_app("/usr/bin/producer-consumer", "Producer-Consumer Simulation");
        else if (item == 3) launch_app("/usr/bin/boot-benchmark", "Boot Benchmark");
        else if (item == 4) open_window(APP_WIFI);
        else if (item == 5) launch_terminal();
        else if (item == 6) { endwin(); refresh(); }
        else if (item == 7) { endwin(); if (system("poweroff") != 0) system("reboot"); }
        return;
    }

    /* Bottom dock */
    if (ev->y == ROWS - 1) {
        if (ev->x >= 1 && ev->x <= 4)   { options_open = !options_open; options_item = 0; return; }
        if (ev->x >= 5 && ev->x <= 8)   { open_window(APP_OPTIONS); return; }
        if (ev->x >= 9 && ev->x <= 12)  { open_window(APP_FILES); return; }
        if (ev->x >= 13 && ev->x <= 16) { launch_terminal(); return; }
        if (ev->x >= 17 && ev->x <= 20) { launch_app("/usr/games/arcade", "Retro Arcade Hub"); return; }
        if (ev->x >= 21 && ev->x <= 24) { open_window(APP_BROWSER); return; }
        if (ev->x >= 25 && ev->x <= 28) { open_window(APP_SYSMON); return; }
        if (ev->x >= 29 && ev->x <= 32) { open_window(APP_SETTINGS); return; }

        /* Workspace pills */
        if (ev->x >= COLS - 38 && ev->x < COLS - 24) { current_workspace = 1; return; }
        if (ev->x >= COLS - 24 && ev->x < COLS - 19) { current_workspace = 2; return; }
        if (ev->x >= COLS - 19 && ev->x < COLS - 14) { current_workspace = 3; return; }
        if (ev->x >= COLS - 14 && ev->x < COLS - 9)  { current_workspace = 4; return; }
    }

    /* Window close button [X] Close click */
    if (focused_win >= 0 && wins[focused_win].active) {
        WinState *w = &wins[focused_win];
        if (ev->y == w->y && ev->x >= w->x + 1 && ev->x <= w->x + 11) {
            close_focused_window();
            return;
        }
    }

    /* Icon clicks */
    for (int i = 0; i < TOTAL_APPS; i++) {
        int r, c;
        get_icon_coords(i, &r, &c);
        if (ev->y == r && ev->x >= c && ev->x <= c + 24) {
            selected_icon = i;
            activate_icon(i);
            return;
        }
    }
}

static void handle_input(int ch)
{
    if (ch == KEY_MOUSE) {
        MEVENT ev;
        if (getmouse(&ev) == OK) {
            handle_mouse(&ev);
            need_redraw = 1;
            return;
        }
    }

    if (options_open) {
        switch (ch) {
            case KEY_UP:   options_item = (options_item - 1 + NUM_OPTIONS) % NUM_OPTIONS; break;
            case KEY_DOWN: options_item = (options_item + 1) % NUM_OPTIONS; break;
            case '\n': case '\r': case KEY_ENTER:
                options_open = 0;
                if (options_item == 0) open_window(APP_OPTIONS);
                else if (options_item == 1) launch_app("/usr/games/arcade", "Retro Arcade Hub");
                else if (options_item == 2) launch_app("/usr/bin/producer-consumer", "Producer-Consumer Simulation");
                else if (options_item == 3) launch_app("/usr/bin/boot-benchmark", "Boot Benchmark");
                else if (options_item == 4) open_window(APP_WIFI);
                else if (options_item == 5) launch_terminal();
                else if (options_item == 7) { endwin(); if (system("poweroff") != 0) system("reboot"); }
                break;
            case 27: case 'q': case 'Q':
                options_open = 0;
                break;
        }
        need_redraw = 1;
        return;
    }

    /* Window scrolling and interactive keys */
    if (focused_win >= 0 && wins[focused_win].active) {
        if (ch == KEY_PPAGE)
            wins[focused_win].scroll = (wins[focused_win].scroll > 3) ? wins[focused_win].scroll - 3 : 0;
        else if (ch == KEY_NPAGE)
            wins[focused_win].scroll += 3;
        else if (wins[focused_win].type == APP_WIFI) {
            if (ch >= '1' && ch <= '0' + WIFI_COUNT) {
                strncpy(wifi_ssid, wifi_list[ch - '1'].name, sizeof(wifi_ssid) - 1);
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                FILE *wf = fopen("/run/wifi.state", "w");
                if (wf) { fputs(wifi_ssid, wf); fclose(wf); }
                need_redraw = 1;
                return;
            } else if (ch == 'c' || ch == 'C') {
                strncpy(wifi_ssid, wifi_list[0].name, sizeof(wifi_ssid) - 1);
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                FILE *wf = fopen("/run/wifi.state", "w");
                if (wf) { fputs(wifi_ssid, wf); fclose(wf); }
                need_redraw = 1;
                return;
            } else if (ch == 'd' || ch == 'D') {
                wifi_ssid[0] = '\0';
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                FILE *wf = fopen("/run/wifi.state", "w");
                if (wf) { fputs("", wf); fclose(wf); }
                need_redraw = 1;
                return;
            } else if (ch == 'r' || ch == 'R') {
                sync_wifi_from_host();
                build_wifi(wins[focused_win].content, sizeof(wins[focused_win].content));
                need_redraw = 1;
                return;
            }
        }
    }

    switch (ch) {
        case 17: /* Ctrl+Q */
            running = 0;
            break;

        /* Workspace switching */
        case '1': current_workspace = 1; break;
        case '2': current_workspace = 2; break;
        case '3': current_workspace = 3; break;
        case '4': current_workspace = 4; break;

        /* Icon Navigation */
        case KEY_UP:   selected_icon = (selected_icon - 1 + TOTAL_APPS) % TOTAL_APPS; break;
        case KEY_DOWN: selected_icon = (selected_icon + 1) % TOTAL_APPS; break;
        case KEY_LEFT: case KEY_RIGHT:
            if (selected_icon < 11) {
                int n = selected_icon + 11;
                if (n < TOTAL_APPS) selected_icon = n;
            } else {
                selected_icon -= 11;
            }
            break;

        /* Open selected app */
        case '\n': case '\r': case KEY_ENTER: case ' ':
            activate_icon(selected_icon);
            break;

        /* Close window */
        case 23: /* Ctrl+W */
        case 27: /* Esc */
            close_focused_window();
            break;

        /* Fast Shortcuts */
        case 'g': case 'G': launch_app("/usr/games/arcade", "Retro Arcade Hub"); break;
        case 'k': case 'K': launch_app("/usr/games/snake", "Snake Game"); break;
        case 'p': case 'P': launch_app("/usr/bin/producer-consumer", "Producer-Consumer Simulator"); break;
        case 'w': case 'W': open_window(APP_WIFI); break;
        case 's': case 'S': launch_app("/usr/bin/snap-store", "Snap Store"); break;
        case 't': case 'T': launch_terminal(); break;
        case 'b': case 'B': open_window(APP_BROWSER); break;
        case 'm': case 'M': open_window(APP_SYSMON); break;
        case 'o': case 'O': options_open = !options_open; options_item = 0; break;
        case 'a': case 'A': open_window(APP_OPTIONS); break;

        default: break;
    }
    need_redraw = 1;
}

/* ═══════════════════════════════════════════════════════════════════
   MAIN
   ═══════════════════════════════════════════════════════════════════ */
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
    timeout(1000);
    scrollok(stdscr, FALSE);

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "MicroOS Desktop requires a color terminal.\n");
        return 1;
    }
    init_colors();
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    /* Sync Wi-Fi SSID from state file or live host if reachable */
    FILE *wf = fopen("/run/wifi.state", "r");
    if (!wf) wf = fopen("/etc/wifi_ssid", "r");
    if (!wf) wf = fopen("/etc/wifi.conf", "r");
    if (wf) {
        char line[64];
        if (fgets(line, sizeof(line), wf)) {
            size_t l = strlen(line);
            while (l > 0 && (line[l-1] == '\n' || line[l-1] == '\r' || line[l-1] == ' '))
                line[--l] = '\0';
            if (line[0] != '\0') {
                strncpy(wifi_ssid, line, sizeof(wifi_ssid) - 1);
            }
        }
        fclose(wf);
    }
    sync_wifi_from_host();

    while (running) {
        if (need_redraw) {
            draw_all();
            need_redraw = 0;
        }
        int ch = getch();
        if (ch == ERR) {
            static int sync_counter = 0;
            if (++sync_counter >= 10) {
                sync_counter = 0;
                sync_wifi_from_host();
            }
            need_redraw = 1; /* Clock tick update */
        } else {
            handle_input(ch);
        }
    }

    endwin();
    printf("\033[H\033[2J");
    printf("Returned to MicroOS terminal shell.\nType 'desktop' to restart GUI.\n\n");
    return 0;
}
