use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub const GRID_W: usize = 20;
pub const GRID_H: usize = 16;
pub const CELL_SIZE: usize = 14;

#[derive(Copy, Clone, PartialEq, Eq)]
pub enum Direction {
    Up,
    Down,
    Left,
    Right,
}

pub struct SnakeApp {
    pub body: [(i32, i32); 100],
    pub len: usize,
    pub dir: Direction,
    pub food: (i32, i32),
    pub score: u32,
    pub high_score: u32,
    pub game_over: bool,
    pub tick_counter: u32,
}

impl SnakeApp {
    pub fn new() -> Self {
        let mut app = Self {
            body: [(0, 0); 100],
            len: 4,
            dir: Direction::Right,
            food: (12, 8),
            score: 0,
            high_score: 40,
            game_over: false,
            tick_counter: 0,
        };
        app.reset();
        app
    }

    pub fn reset(&mut self) {
        self.len = 4;
        self.body[0] = (8, 8);
        self.body[1] = (7, 8);
        self.body[2] = (6, 8);
        self.body[3] = (5, 8);
        self.dir = Direction::Right;
        self.food = (14, 8);
        self.score = 0;
        self.game_over = false;
    }

    pub fn handle_key(&mut self, key: char) {
        match key {
            'w' | 'W' => if self.dir != Direction::Down { self.dir = Direction::Up; },
            's' | 'S' => if self.dir != Direction::Up { self.dir = Direction::Down; },
            'a' | 'A' => if self.dir != Direction::Right { self.dir = Direction::Left; },
            'd' | 'D' => if self.dir != Direction::Left { self.dir = Direction::Right; },
            'r' | 'R' => self.reset(),
            _ => {}
        }
    }

    pub fn handle_click(&mut self, rel_x: i32, rel_y: i32) {
        // [ New Game ] button: x: 300..390, y: 50..78
        if rel_x >= 300 && rel_x <= 390 && rel_y >= 50 && rel_y <= 78 {
            self.reset();
        }
        // D-Pad buttons for mouse play:
        // Up: x: 335..365, y: 130..155
        else if rel_x >= 335 && rel_x <= 365 && rel_y >= 130 && rel_y <= 155 {
            if self.dir != Direction::Down { self.dir = Direction::Up; }
        }
        // Down: x: 335..365, y: 190..215
        else if rel_x >= 335 && rel_x <= 365 && rel_y >= 190 && rel_y <= 215 {
            if self.dir != Direction::Up { self.dir = Direction::Down; }
        }
        // Left: x: 305..335, y: 160..185
        else if rel_x >= 305 && rel_x <= 335 && rel_y >= 160 && rel_y <= 185 {
            if self.dir != Direction::Right { self.dir = Direction::Left; }
        }
        // Right: x: 365..395, y: 160..185
        else if rel_x >= 365 && rel_x <= 395 && rel_y >= 160 && rel_y <= 185 {
            if self.dir != Direction::Left { self.dir = Direction::Right; }
        }
    }

    pub fn tick(&mut self) {
        if self.game_over {
            return;
        }

        self.tick_counter += 1;
        if self.tick_counter % 8 != 0 {
            return;
        }

        // Calculate new head
        let (head_x, head_y) = self.body[0];
        let (new_x, new_y) = match self.dir {
            Direction::Up => (head_x, head_y - 1),
            Direction::Down => (head_x, head_y + 1),
            Direction::Left => (head_x - 1, head_y),
            Direction::Right => (head_x + 1, head_y),
        };

        // Wall collision
        if new_x < 0 || new_x >= GRID_W as i32 || new_y < 0 || new_y >= GRID_H as i32 {
            self.game_over = true;
            return;
        }

        // Self collision
        for i in 0..self.len {
            if self.body[i] == (new_x, new_y) {
                self.game_over = true;
                return;
            }
        }

        // Shift body
        for i in (1..self.len).rev() {
            self.body[i] = self.body[i - 1];
        }
        self.body[0] = (new_x, new_y);

        // Check food
        if (new_x, new_y) == self.food {
            if self.len < 99 {
                self.len += 1;
            }
            self.score += 10;
            if self.score > self.high_score {
                self.high_score = self.score;
            }
            // Spawn next food
            self.food = (
                ((new_x * 7 + 3) % (GRID_W as i32 - 2)) + 1,
                ((new_y * 5 + 4) % (GRID_H as i32 - 2)) + 1,
            );
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header
        surface.draw_string(win_x + 16, win_y + 12, "Micro Arcade: Snake Game", Color::SLATE_DARK);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Grid Board
        let grid_x = win_x + 16;
        let grid_y = win_y + 42;
        let board_w = GRID_W * CELL_SIZE;
        let board_h = GRID_H * CELL_SIZE;

        surface.draw_rect(grid_x, grid_y, board_w, board_h, Color::SLATE_DARK);
        surface.draw_rect_outline(grid_x, grid_y, board_w, board_h, Color::BLACK);

        // Render Food (Apple)
        let (fx, fy) = self.food;
        let food_px = grid_x + fx * CELL_SIZE as i32;
        let food_py = grid_y + fy * CELL_SIZE as i32;
        surface.draw_rect(food_px + 2, food_py + 2, CELL_SIZE - 4, CELL_SIZE - 4, Color::ACCENT_RED);

        // Render Snake Body
        for i in 0..self.len {
            let (bx, by) = self.body[i];
            let px = grid_x + bx * CELL_SIZE as i32;
            let py = grid_y + by * CELL_SIZE as i32;
            let body_color = if i == 0 { Color::ACCENT_GREEN } else { Color::rgb(74, 222, 128) };
            surface.draw_rect(px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2, body_color);
        }

        // Side Control Panel
        let side_x = win_x + board_w as i32 + 30;
        let side_y = win_y + 42;

        surface.draw_string(side_x, side_y, "SCORE", Color::MUTED_TEXT);
        let mut score_buf = [b'0'; 5];
        let s = self.score;
        score_buf[0] = b'0' + ((s / 1000) % 10) as u8;
        score_buf[1] = b'0' + ((s / 100) % 10) as u8;
        score_buf[2] = b'0' + ((s / 10) % 10) as u8;
        score_buf[3] = b'0' + (s % 10) as u8;
        if let Ok(st) = core::str::from_utf8(&score_buf[..4]) {
            surface.draw_string(side_x, side_y + 16, st, Color::ACCENT_GREEN);
        }

        surface.draw_string(side_x, side_y + 40, "BEST", Color::MUTED_TEXT);
        let mut best_buf = [b'0'; 5];
        let bs = self.high_score;
        best_buf[0] = b'0' + ((bs / 1000) % 10) as u8;
        best_buf[1] = b'0' + ((bs / 100) % 10) as u8;
        best_buf[2] = b'0' + ((bs / 10) % 10) as u8;
        best_buf[3] = b'0' + (bs % 10) as u8;
        if let Ok(st) = core::str::from_utf8(&best_buf[..4]) {
            surface.draw_string(side_x, side_y + 56, st, Color::ACCENT_CYAN);
        }

        // [ New Game ] Button
        surface.draw_rect(side_x, side_y + 80, 90, 26, Color::ACCENT_BLUE);
        surface.draw_rect_outline(side_x, side_y + 80, 90, 26, Color::WIN_TITLEBAR);
        surface.draw_string(side_x + 12, side_y + 86, "New Game", Color::WHITE);

        // On-screen D-Pad for mouse play
        surface.draw_string(side_x + 10, side_y + 120, "CONTROLS", Color::MUTED_TEXT);

        // Up button
        surface.draw_rect(side_x + 35, side_y + 140, 28, 22, Color::SLATE_CARD);
        surface.draw_rect_outline(side_x + 35, side_y + 140, 28, 22, Color::TASKBAR_BORDER);
        surface.draw_string(side_x + 45, side_y + 144, "^", Color::SLATE_DARK);

        // Left button
        surface.draw_rect(side_x + 5, side_y + 168, 28, 22, Color::SLATE_CARD);
        surface.draw_rect_outline(side_x + 5, side_y + 168, 28, 22, Color::TASKBAR_BORDER);
        surface.draw_string(side_x + 15, side_y + 172, "<", Color::SLATE_DARK);

        // Down button
        surface.draw_rect(side_x + 35, side_y + 168, 28, 22, Color::SLATE_CARD);
        surface.draw_rect_outline(side_x + 35, side_y + 168, 28, 22, Color::TASKBAR_BORDER);
        surface.draw_string(side_x + 45, side_y + 172, "v", Color::SLATE_DARK);

        // Right button
        surface.draw_rect(side_x + 65, side_y + 168, 28, 22, Color::SLATE_CARD);
        surface.draw_rect_outline(side_x + 65, side_y + 168, 28, 22, Color::TASKBAR_BORDER);
        surface.draw_string(side_x + 75, side_y + 172, ">", Color::SLATE_DARK);

        surface.draw_string(side_x, side_y + 200, "W/A/S/D to move", Color::MUTED_TEXT);

        if self.game_over {
            surface.draw_rect(grid_x + 40, grid_y + 80, 200, 50, Color::SLATE_DARK);
            surface.draw_rect_outline(grid_x + 40, grid_y + 80, 200, 50, Color::ACCENT_RED);
            surface.draw_string(grid_x + 85, grid_y + 92, "GAME OVER", Color::ACCENT_RED);
            surface.draw_string(grid_x + 55, grid_y + 110, "Click 'New Game' or 'R'", Color::WHITE);
        }
    }
}
