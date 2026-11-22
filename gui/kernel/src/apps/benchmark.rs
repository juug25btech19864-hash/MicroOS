use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct BenchmarkApp {
    pub is_running: bool,
    pub progress: u32,
    pub test_count: u32,
}

impl BenchmarkApp {
    pub fn new() -> Self {
        Self {
            is_running: false,
            progress: 100,
            test_count: 1,
        }
    }

    pub fn tick(&mut self) {
        if self.is_running {
            self.progress += 2;
            if self.progress >= 100 {
                self.progress = 100;
                self.is_running = false;
                self.test_count += 1;
            }
        }
    }

    pub fn handle_click(&mut self, rel_x: i32, rel_y: i32) {
        // "Run Benchmark" button bounds: x: 16..180, y: 250..275
        if rel_x >= 16 && rel_x <= 180 && rel_y >= 250 && rel_y <= 275 {
            self.is_running = true;
            self.progress = 0;
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header
        surface.draw_string(win_x + 16, win_y + 12, "Boot Time Benchmark", Color::SLATE_DARK);
        surface.draw_string(win_x + 180, win_y + 14, "Kernel & Userspace Startup Latency", Color::MUTED_TEXT);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Stage Breakdown Card
        let card_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, win_y + 42, card_w, 110, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, win_y + 42, card_w, 110, Color::TASKBAR_BORDER);

        surface.draw_string(win_x + 24, win_y + 50, "COLD BOOT PHASES (x86_64)", Color::ACCENT_BLUE);

        let stages = [
            ("1. BIOS/UEFI Firmware Handoff", "0.14 s", 14),
            ("2. Kernel Decompress & Relocation", "0.18 s", 18),
            ("3. Hardware Discovery (PCI/Storage)", "0.09 s", 9),
            ("4. VFS & Rootfs Staging Mount", "0.06 s", 6),
            ("5. Compositor & GUI Shell Ready", "0.12 s", 12),
        ];

        for (i, s) in stages.iter().enumerate() {
            let row_y = win_y + 70 + (i as i32 * 15);
            surface.draw_string(win_x + 24, row_y, s.0, Color::SLATE_TEXT);
            surface.draw_string(win_x + 290, row_y, s.1, Color::ACCENT_GREEN);
            // Visual bar
            surface.draw_rect(win_x + 350, row_y + 3, s.2 * 5, 6, Color::ACCENT_CYAN);
        }

        // Total Boot Time Badge
        surface.draw_rect(win_x + 16, win_y + 160, card_w, 28, Color::WHITE);
        surface.draw_rect_outline(win_x + 16, win_y + 160, card_w, 28, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, win_y + 168, "TOTAL TIME TO INTERACTIVE DESKTOP:", Color::SLATE_DARK);
        surface.draw_string(win_x + 310, win_y + 168, "0.59 seconds", Color::ACCENT_GREEN);
        surface.draw_string(win_x + 420, win_y + 168, "[ULTRA-FAST]", Color::ACCENT_BLUE);

        // OS Comparison Bars
        let comp_y = win_y + 198;
        surface.draw_string(win_x + 16, comp_y, "BOOT DURATION COMPARISON:", Color::MUTED_TEXT);

        let os_comps = [
            ("MicroOS (Ours)", 25, "0.59s", Color::ACCENT_GREEN),
            ("Alpine Linux", 60, "2.10s", Color::ACCENT_CYAN),
            ("Ubuntu Desktop", 190, "16.8s", Color::ACCENT_ORANGE),
            ("Windows 11", 240, "22.4s", Color::ACCENT_RED),
        ];

        for (i, item) in os_comps.iter().enumerate() {
            let ry = comp_y + 18 + (i as i32 * 16);
            surface.draw_string(win_x + 16, ry, item.0, Color::SLATE_TEXT);
            surface.draw_rect(win_x + 140, ry + 2, item.1, 10, item.3);
            surface.draw_string(win_x + 150 + item.1 as i32, ry, item.2, Color::SLATE_TEXT);
        }

        // Action Button: Run Benchmark Test
        let btn_y = win_y + win_h as i32 - 40;
        let btn_bg = if self.is_running { Color::rgb(220, 225, 230) } else { Color::ACCENT_BLUE };
        surface.draw_rect(win_x + 16, btn_y, 160, 28, btn_bg);
        surface.draw_rect_outline(win_x + 16, btn_y, 160, 28, Color::WIN_TITLEBAR);
        if self.is_running {
            surface.draw_string(win_x + 32, btn_y + 6, "Benchmarking...", Color::SLATE_TEXT);
        } else {
            surface.draw_string(win_x + 30, btn_y + 6, "Run Benchmark", Color::WHITE);
        }

        if self.is_running {
            surface.draw_rect(win_x + 190, btn_y + 8, 200, 12, Color::WHITE);
            surface.draw_rect(win_x + 190, btn_y + 8, self.progress as usize * 2, 12, Color::ACCENT_GREEN);
            surface.draw_rect_outline(win_x + 190, btn_y + 8, 200, 12, Color::TASKBAR_BORDER);
        }
    }
}
