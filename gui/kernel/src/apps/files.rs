use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct FilesApp {
    pub is_root_fs: bool,
    pub path: &'static str,
}

impl FilesApp {
    pub fn new(is_root_fs: bool) -> Self {
        Self {
            is_root_fs,
            path: if is_root_fs { "/" } else { "/home/micro" },
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header Title
        let title = if self.is_root_fs { "File System - Root (/)" } else { "File Manager - /home/micro" };
        surface.draw_string(win_x + 16, win_y + 12, title, Color::SLATE_DARK);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Path Bar
        surface.draw_rect(win_x + 16, win_y + 40, win_w.saturating_sub(32), 22, Color::WHITE);
        surface.draw_rect_outline(win_x + 16, win_y + 40, win_w.saturating_sub(32), 22, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, win_y + 44, "Location: ", Color::MUTED_TEXT);
        surface.draw_string(win_x + 104, win_y + 44, self.path, Color::ACCENT_BLUE);

        // File List Header
        let list_y = win_y + 70;
        let list_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, list_y, list_w, 20, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, list_y, list_w, 20, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, list_y + 4, "NAME", Color::SLATE_DARK);
        surface.draw_string(win_x + 180, list_y + 4, "TYPE", Color::SLATE_DARK);
        surface.draw_string(win_x + 280, list_y + 4, "SIZE", Color::SLATE_DARK);
        surface.draw_string(win_x + 360, list_y + 4, "PERMISSIONS", Color::SLATE_DARK);

        if self.is_root_fs {
            let root_items = [
                ("bin", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("boot", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("dev", "DevFS", "0 Bytes", "drwxr-xr-x"),
                ("etc", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("home", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("lib", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("mnt", "MountPoint", "4.0 KB", "drwxr-xr-x"),
                ("proc", "ProcFS", "0 Bytes", "dr-xr-xr-x"),
                ("sys", "SysFS", "0 Bytes", "dr-xr-xr-x"),
                ("usr", "Directory", "4.0 KB", "drwxr-xr-x"),
            ];

            for (i, it) in root_items.iter().enumerate() {
                let ry = list_y + 24 + (i as i32 * 18);
                surface.draw_string(win_x + 24, ry, it.0, Color::ACCENT_BLUE);
                surface.draw_string(win_x + 180, ry, it.1, Color::MUTED_TEXT);
                surface.draw_string(win_x + 280, ry, it.2, Color::SLATE_TEXT);
                surface.draw_string(win_x + 360, ry, it.3, Color::MUTED_TEXT);
            }
        } else {
            let home_items = [
                ("Desktop", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("Documents", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("Downloads", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("Games", "Directory", "4.0 KB", "drwxr-xr-x"),
                ("microos.conf", "Configuration", "1.2 KB", "-rw-r--r--"),
                ("README.txt", "Text Document", "2.8 KB", "-rw-r--r--"),
                ("benchmark.log", "Log File", "8.4 KB", "-rw-r--r--"),
            ];

            for (i, it) in home_items.iter().enumerate() {
                let ry = list_y + 24 + (i as i32 * 18);
                let name_col = if it.1 == "Directory" { Color::ACCENT_BLUE } else { Color::SLATE_DARK };
                surface.draw_string(win_x + 24, ry, it.0, name_col);
                surface.draw_string(win_x + 180, ry, it.1, Color::MUTED_TEXT);
                surface.draw_string(win_x + 280, ry, it.2, Color::SLATE_TEXT);
                surface.draw_string(win_x + 360, ry, it.3, Color::MUTED_TEXT);
            }
        }
    }
}
