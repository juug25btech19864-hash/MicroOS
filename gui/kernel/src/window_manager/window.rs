extern crate alloc;

use alloc::string::String;
use crate::apps::AppType;
use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub const TITLE_BAR_HEIGHT: usize = 24;
pub const BORDER_THICKNESS: usize = 2;

pub struct Window {
    pub id: usize,
    pub title: String,
    pub x: i32,
    pub y: i32,
    pub width: usize,
    pub height: usize,
    pub app_type: AppType,
    pub is_minimized: bool,
    pub is_maximized: bool,
}

impl Window {
    pub fn new(id: usize, title: String, x: i32, y: i32, width: usize, height: usize, app_type: AppType) -> Self {
        Self {
            id,
            title,
            x,
            y,
            width,
            height,
            app_type,
            is_minimized: false,
            is_maximized: false,
        }
    }

    pub fn render_frame(&self, surface: &mut Surface, is_active: bool) {
        if self.is_minimized {
            return;
        }

        // Window drop shadow effect
        surface.draw_rect(self.x + 4, self.y + 4, self.width, self.height, Color::rgba(0, 0, 0, 60));

        // Outer Border
        surface.draw_rect_outline(self.x, self.y, self.width, self.height, Color::WIN_BORDER);

        // Title Bar
        let title_bg = if is_active { Color::WIN_TITLEBAR } else { Color::WIN_TITLEBAR_INACTIVE };
        surface.draw_rect(self.x + BORDER_THICKNESS as i32, self.y + BORDER_THICKNESS as i32, self.width - BORDER_THICKNESS * 2, TITLE_BAR_HEIGHT, title_bg);

        // Title text
        surface.draw_string(self.x + 12, self.y + 6, &self.title, Color::WIN_TITLE_TEXT);

        // Control Buttons: Close (✕) and Minimize (_)
        let btn_close_x = self.x + self.width as i32 - 24;
        let btn_min_x = btn_close_x - 22;
        let btn_y = self.y + 3;

        // Minimize Button
        surface.draw_rect(btn_min_x, btn_y, 18, 18, Color::rgb(71, 85, 105));
        surface.draw_line_h(btn_min_x + 4, btn_y + 12, 10, Color::WHITE);

        // Close Button
        surface.draw_rect(btn_close_x, btn_y, 18, 18, Color::WIN_CONTROL_RED);
        surface.draw_line_h(btn_close_x + 5, btn_y + 5, 8, Color::WHITE);
        surface.draw_line_h(btn_close_x + 5, btn_y + 12, 8, Color::WHITE);
        surface.draw_line_v(btn_close_x + 8, btn_y + 5, 8, Color::WHITE);
    }

    pub fn content_rect(&self) -> (i32, i32, usize, usize) {
        let cx = self.x + BORDER_THICKNESS as i32;
        let cy = self.y + BORDER_THICKNESS as i32 + TITLE_BAR_HEIGHT as i32;
        let cw = self.width.saturating_sub(BORDER_THICKNESS * 2);
        let ch = self.height.saturating_sub(BORDER_THICKNESS * 2 + TITLE_BAR_HEIGHT);
        (cx, cy, cw, ch)
    }

    pub fn is_close_button_clicked(&self, click_x: i32, click_y: i32) -> bool {
        let btn_close_x = self.x + self.width as i32 - 24;
        let btn_y = self.y + 3;
        click_x >= btn_close_x && click_x < btn_close_x + 18 && click_y >= btn_y && click_y < btn_y + 18
    }

    pub fn is_minimize_button_clicked(&self, click_x: i32, click_y: i32) -> bool {
        let btn_close_x = self.x + self.width as i32 - 24;
        let btn_min_x = btn_close_x - 22;
        let btn_y = self.y + 3;
        click_x >= btn_min_x && click_x < btn_min_x + 18 && click_y >= btn_y && click_y < btn_y + 18
    }

    pub fn is_titlebar_clicked(&self, click_x: i32, click_y: i32) -> bool {
        click_x >= self.x && click_x < self.x + self.width as i32 && click_y >= self.y && click_y < self.y + TITLE_BAR_HEIGHT as i32 + 2
    }
}
