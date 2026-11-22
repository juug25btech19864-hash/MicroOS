use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct DashboardApp {
    pub tick_counter: u32,
}

impl DashboardApp {
    pub fn new() -> Self {
        Self { tick_counter: 0 }
    }

    pub fn tick(&mut self) {
        self.tick_counter = self.tick_counter.wrapping_add(1);
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        // App canvas background
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header Title
        surface.draw_string(win_x + 16, win_y + 12, "MicroOS Dashboard", Color::SLATE_DARK);
        surface.draw_string(win_x + 160, win_y + 14, "v1.0 (64-bit x86_64)", Color::MUTED_TEXT);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Card 1: System Telemetry
        let card1_x = win_x + 16;
        let card1_y = win_y + 42;
        let card_w = (win_w - 48) / 2;
        let card1_h = 100;
        surface.draw_rect(card1_x, card1_y, card_w, card1_h, Color::SLATE_CARD);
        surface.draw_rect_outline(card1_x, card1_y, card_w, card1_h, Color::TASKBAR_BORDER);
        surface.draw_string(card1_x + 10, card1_y + 8, "SYSTEM INFORMATION", Color::ACCENT_BLUE);

        surface.draw_string(card1_x + 10, card1_y + 28, "Kernel   : Linux 6.6.63-micro", Color::SLATE_TEXT);
        surface.draw_string(card1_x + 10, card1_y + 44, "Arch     : x86_64 Intel/AMD", Color::SLATE_TEXT);
        surface.draw_string(card1_x + 10, card1_y + 60, "Platform : Oracle VirtualBox", Color::SLATE_TEXT);
        let sec = (self.tick_counter / 30) % 60;
        let min = (self.tick_counter / 1800) % 60;
        let mut time_str = [b'0'; 8];
        time_str[0] = b'0' + (min / 10) as u8;
        time_str[1] = b'0' + (min % 10) as u8;
        time_str[2] = b':';
        time_str[3] = b'0' + (sec / 10) as u8;
        time_str[4] = b'0' + (sec % 10) as u8;
        time_str[5] = b' ';
        time_str[6] = b'u';
        time_str[7] = b'p';
        surface.draw_string(card1_x + 10, card1_y + 76, "Uptime   : ", Color::SLATE_TEXT);
        if let Ok(s) = core::str::from_utf8(&time_str) {
            surface.draw_string(card1_x + 90, card1_y + 76, s, Color::ACCENT_GREEN);
        }

        // Card 2: Resource Utilization
        let card2_x = win_x + 24 + card_w as i32;
        let card2_y = win_y + 42;
        surface.draw_rect(card2_x, card2_y, card_w, card1_h, Color::SLATE_CARD);
        surface.draw_rect_outline(card2_x, card2_y, card_w, card1_h, Color::TASKBAR_BORDER);
        surface.draw_string(card2_x + 10, card2_y + 8, "RESOURCE MONITOR", Color::ACCENT_BLUE);

        // CPU Utilization
        let cpu_pct = 12 + ((self.tick_counter / 15) % 15);
        surface.draw_string(card2_x + 10, card2_y + 28, "CPU Load :", Color::SLATE_TEXT);
        let mut cpu_buf = [b' '; 5];
        cpu_buf[0] = b'0' + (cpu_pct / 10) as u8;
        cpu_buf[1] = b'0' + (cpu_pct % 10) as u8;
        cpu_buf[2] = b'%';
        if let Ok(s) = core::str::from_utf8(&cpu_buf[..3]) {
            surface.draw_string(card2_x + 90, card2_y + 28, s, Color::ACCENT_CYAN);
        }
        // CPU bar
        surface.draw_rect(card2_x + 120, card2_y + 30, 80, 8, Color::WHITE);
        surface.draw_rect(card2_x + 120, card2_y + 30, (cpu_pct as usize * 80) / 100, 8, Color::ACCENT_CYAN);
        surface.draw_rect_outline(card2_x + 120, card2_y + 30, 80, 8, Color::TASKBAR_BORDER);

        // RAM Utilization
        surface.draw_string(card2_x + 10, card2_y + 50, "Memory   : 38MB / 1024MB", Color::SLATE_TEXT);
        surface.draw_rect(card2_x + 10, card2_y + 66, card_w.saturating_sub(20), 8, Color::WHITE);
        surface.draw_rect(card2_x + 10, card2_y + 66, (card_w * 38) / 1024, 8, Color::ACCENT_GREEN);
        surface.draw_rect_outline(card2_x + 10, card2_y + 66, card_w.saturating_sub(20), 8, Color::TASKBAR_BORDER);

        // Storage
        surface.draw_string(card2_x + 10, card2_y + 80, "Storage  : 14.8MB / 512MB (ext4)", Color::SLATE_TEXT);

        // Process Table
        let table_y = win_y + 154;
        let table_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, table_y, table_w, 115, Color::WHITE);
        surface.draw_rect_outline(win_x + 16, table_y, table_w, 115, Color::TASKBAR_BORDER);

        // Table Header
        surface.draw_rect(win_x + 16, table_y, table_w, 22, Color::SLATE_CARD);
        surface.draw_line_h(win_x + 16, table_y + 22, table_w, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, table_y + 4, "PID", Color::MUTED_TEXT);
        surface.draw_string(win_x + 64, table_y + 4, "PROCESS NAME", Color::MUTED_TEXT);
        surface.draw_string(win_x + 220, table_y + 4, "STATUS", Color::MUTED_TEXT);
        surface.draw_string(win_x + 290, table_y + 4, "CPU %", Color::MUTED_TEXT);
        surface.draw_string(win_x + 360, table_y + 4, "RAM", Color::MUTED_TEXT);

        let procs = [
            ("1", "system_init", "Running", " 0.2%", " 4.1 MB"),
            ("2", "compositor_gui", "Active", "12.4%", "16.2 MB"),
            ("3", "iwd_wifi_daemon", "Sleeping", " 0.1%", " 2.8 MB"),
            ("4", "ps2_input_mgr", "Sleeping", " 0.4%", " 1.9 MB"),
            ("5", "apt_daemon", "Idle", " 0.0%", " 3.2 MB"),
        ];

        for (i, p) in procs.iter().enumerate() {
            let row_y = table_y + 26 + (i as i32 * 17);
            surface.draw_string(win_x + 24, row_y, p.0, Color::SLATE_TEXT);
            surface.draw_string(win_x + 64, row_y, p.1, Color::SLATE_DARK);
            surface.draw_string(win_x + 220, row_y, p.2, Color::ACCENT_GREEN);
            surface.draw_string(win_x + 290, row_y, p.3, Color::ACCENT_CYAN);
            surface.draw_string(win_x + 360, row_y, p.4, Color::MUTED_TEXT);
        }
    }
}
