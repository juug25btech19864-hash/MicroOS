use crate::graphics::color::Color;
use crate::graphics::icons::{draw_icon_24, IconType};
use crate::graphics::surface::Surface;
use crate::window_manager::compositor::Compositor;

pub const TASKBAR_HEIGHT: usize = 32;

pub fn render_taskbar(
    surface: &mut Surface,
    compositor: &Compositor,
    start_menu_open: bool,
    screen_w: usize,
    screen_h: usize,
    tick_counter: u32,
) {
    let taskbar_y = (screen_h - TASKBAR_HEIGHT) as i32;

    // 1. Taskbar Bar Background
    surface.draw_rect(0, taskbar_y, screen_w, TASKBAR_HEIGHT, Color::TASKBAR_BG);
    surface.draw_line_h(0, taskbar_y, screen_w, Color::TASKBAR_BORDER);

    // 2. Start Button
    let start_w = 75;
    let start_h = 24;
    let start_x = 4;
    let start_y = taskbar_y + 4;

    let start_bg = if start_menu_open { Color::rgb(203, 213, 225) } else { Color::WHITE };
    surface.draw_rect(start_x, start_y, start_w, start_h, start_bg);
    surface.draw_rect_outline(start_x, start_y, start_w, start_h, Color::TASKBAR_BORDER);

    // Blue feather / leaf logo
    draw_icon_24(surface, start_x + 4, start_y + 1, IconType::StartLogo);
    surface.draw_string(start_x + 24, start_y + 5, "Start", Color::TASKBAR_TEXT);

    // 3. Open Window Tabs in Taskbar
    let mut tab_x = start_x + start_w as i32 + 8;
    let tab_w = 140;
    let tab_h = 24;

    for win in compositor.windows.iter() {
        let is_active = compositor.active_id == Some(win.id) && !win.is_minimized;
        let tab_bg = if is_active { Color::TASKBAR_ACTIVE } else { Color::WHITE };
        let tab_border = if is_active { Color::WIN_TITLEBAR } else { Color::TASKBAR_BORDER };

        surface.draw_rect(tab_x, start_y, tab_w, tab_h, tab_bg);
        surface.draw_rect_outline(tab_x, start_y, tab_w, tab_h, tab_border);

        // Truncate title if needed
        let max_chars = 14;
        let title_bytes = win.title.as_bytes();
        let display_len = title_bytes.len().min(max_chars);
        if let Ok(display_str) = core::str::from_utf8(&title_bytes[..display_len]) {
            surface.draw_string(tab_x + 8, start_y + 5, display_str, Color::TASKBAR_TEXT);
        }

        tab_x += tab_w as i32 + 4;
    }

    // 4. System Tray on Right Side
    let tray_w = 110;
    let tray_x = (screen_w - tray_w - 6) as i32;

    // Small tray box
    surface.draw_rect(tray_x, start_y, tray_w, start_h, Color::SLATE_CARD);
    surface.draw_rect_outline(tray_x, start_y, tray_w, start_h, Color::TASKBAR_BORDER);

    // Mini wifi indicator
    surface.draw_rect(tray_x + 8, start_y + 14, 2, 4, Color::ACCENT_GREEN);
    surface.draw_rect(tray_x + 12, start_y + 11, 2, 7, Color::ACCENT_GREEN);
    surface.draw_rect(tray_x + 16, start_y + 8, 2, 10, Color::ACCENT_GREEN);

    // Live Clock display
    let total_secs = (tick_counter / 30) % (24 * 3600);
    let hours = (10 + total_secs / 3600) % 12;
    let display_hour = if hours == 0 { 12 } else { hours };
    let mins = (30 + (total_secs / 60)) % 60;

    let mut clock_buf = [b'0'; 8];
    clock_buf[0] = b'0' + (display_hour / 10) as u8;
    clock_buf[1] = b'0' + (display_hour % 10) as u8;
    clock_buf[2] = b':';
    clock_buf[3] = b'0' + (mins / 10) as u8;
    clock_buf[4] = b'0' + (mins % 10) as u8;
    clock_buf[5] = b' ';
    clock_buf[6] = b'A';
    clock_buf[7] = b'M';

    if let Ok(clock_str) = core::str::from_utf8(&clock_buf) {
        surface.draw_string(tray_x + 28, start_y + 5, clock_str, Color::TASKBAR_TEXT);
    }
}
