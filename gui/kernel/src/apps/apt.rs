use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct AptApp {
    pub selected_idx: usize,
    pub is_updating: bool,
    pub log_message: &'static str,
}

impl AptApp {
    pub fn new() -> Self {
        Self {
            selected_idx: 0,
            is_updating: false,
            log_message: "APT package cache initialized. Ready for operations.",
        }
    }

    pub fn handle_click(&mut self, rel_x: i32, rel_y: i32) {
        // [ Update Index ] button: x: 16..150, y: 195..222
        if rel_x >= 16 && rel_x <= 150 && rel_y >= 195 && rel_y <= 222 {
            self.log_message = "Reading deb.debian.org/debian bookworm... Packages updated.";
        }
        // [ Install ] button: x: 160..270, y: 195..222
        else if rel_x >= 160 && rel_x <= 270 && rel_y >= 195 && rel_y <= 222 {
            self.log_message = "Unpacking package binaries -> dpkg database updated.";
        }
        // Package row selection: y: 70..180
        else if rel_x >= 16 && rel_x <= 440 && rel_y >= 70 && rel_y <= 180 {
            let row = (rel_y - 70) / 18;
            if (row as usize) < 6 {
                self.selected_idx = row as usize;
            }
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header
        surface.draw_string(win_x + 16, win_y + 12, "APT Package Manager", Color::SLATE_DARK);
        surface.draw_string(win_x + 200, win_y + 14, "Debian Bookworm glibc Repository", Color::MUTED_TEXT);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Search Input Bar
        surface.draw_rect(win_x + 16, win_y + 40, win_w.saturating_sub(32), 22, Color::WHITE);
        surface.draw_rect_outline(win_x + 16, win_y + 40, win_w.saturating_sub(32), 22, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, win_y + 44, "Search: [ gcc, python, htop, curl, git...       ]", Color::MUTED_TEXT);

        // Package Table Header
        let table_y = win_y + 68;
        let table_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, table_y, table_w, 20, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, table_y, table_w, 20, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, table_y + 4, "PACKAGE", Color::SLATE_DARK);
        surface.draw_string(win_x + 110, table_y + 4, "VERSION", Color::SLATE_DARK);
        surface.draw_string(win_x + 200, table_y + 4, "STATUS", Color::SLATE_DARK);
        surface.draw_string(win_x + 310, table_y + 4, "SIZE", Color::SLATE_DARK);

        let packages = [
            ("gcc-12", "12.2.0-14", "Installed", " 32 MB"),
            ("make", "4.3-4.1", "Installed", "410 KB"),
            ("python3", "3.11.2-1", "Available", " 14 MB"),
            ("htop", "3.2.2-1", "Available", "520 KB"),
            ("curl", "7.88.1-10", "Installed", "890 KB"),
            ("git", "2.39.5-1", "Available", " 28 MB"),
        ];

        for (i, p) in packages.iter().enumerate() {
            let row_y = table_y + 24 + (i as i32 * 18);
            let is_sel = i == self.selected_idx;
            if is_sel {
                surface.draw_rect(win_x + 16, row_y - 2, table_w, 18, Color::rgb(224, 242, 254));
            }
            surface.draw_string(win_x + 24, row_y, p.0, Color::SLATE_DARK);
            surface.draw_string(win_x + 110, row_y, p.1, Color::MUTED_TEXT);
            let status_color = if p.2 == "Installed" { Color::ACCENT_GREEN } else { Color::ACCENT_BLUE };
            surface.draw_string(win_x + 200, row_y, p.2, status_color);
            surface.draw_string(win_x + 310, row_y, p.3, Color::MUTED_TEXT);
        }

        // Action Buttons
        let btn_y = win_y + win_h as i32 - 75;

        // Button 1: Update
        surface.draw_rect(win_x + 16, btn_y, 130, 26, Color::ACCENT_BLUE);
        surface.draw_rect_outline(win_x + 16, btn_y, 130, 26, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 24, btn_y + 6, "Update Index", Color::WHITE);

        // Button 2: Install
        surface.draw_rect(win_x + 156, btn_y, 110, 26, Color::ACCENT_GREEN);
        surface.draw_rect_outline(win_x + 156, btn_y, 110, 26, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 170, btn_y + 6, "Install", Color::WHITE);

        // Terminal Log Box at Bottom
        let log_y = win_y + win_h as i32 - 42;
        let log_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, log_y, log_w, 32, Color::TERMINAL_BG);
        surface.draw_rect_outline(win_x + 16, log_y, log_w, 32, Color::SLATE_DARK);
        surface.draw_string(win_x + 24, log_y + 8, self.log_message, Color::TERMINAL_GREEN);
    }
}
