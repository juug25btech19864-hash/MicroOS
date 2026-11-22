use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub const BUFFER_CAPACITY: usize = 8;

pub struct ConcurrencyApp {
    pub buffer: [u16; BUFFER_CAPACITY],
    pub count: usize,
    pub head: usize,
    pub tail: usize,
    pub next_item: u16,
    pub mutex_locked: bool,
    pub auto_run: bool,
    pub tick_counter: u32,
    pub last_action: (&'static str, Color),
}

impl ConcurrencyApp {
    pub fn new() -> Self {
        let mut app = Self {
            buffer: [0; BUFFER_CAPACITY],
            count: 3,
            head: 0,
            tail: 3,
            next_item: 4,
            mutex_locked: false,
            auto_run: true,
            tick_counter: 0,
            last_action: ("Initialized bounded buffer with 3 items.", Color::ACCENT_BLUE),
        };
        app.buffer[0] = 1;
        app.buffer[1] = 2;
        app.buffer[2] = 3;
        app
    }

    pub fn produce(&mut self) -> bool {
        if self.count >= BUFFER_CAPACITY {
            self.last_action = ("Buffer FULL! Producer blocked on sem_wait(empty)", Color::ACCENT_RED);
            return false;
        }
        self.mutex_locked = true;
        self.buffer[self.tail] = self.next_item;
        self.next_item = self.next_item.wrapping_add(1);
        self.tail = (self.tail + 1) % BUFFER_CAPACITY;
        self.count += 1;
        self.mutex_locked = false;
        self.last_action = ("Producer: mutex acquired -> inserted item -> sem_post(full)", Color::ACCENT_GREEN);
        true
    }

    pub fn consume(&mut self) -> bool {
        if self.count == 0 {
            self.last_action = ("Buffer EMPTY! Consumer blocked on sem_wait(full)", Color::ACCENT_RED);
            return false;
        }
        self.mutex_locked = true;
        self.buffer[self.head] = 0;
        self.head = (self.head + 1) % BUFFER_CAPACITY;
        self.count -= 1;
        self.mutex_locked = false;
        self.last_action = ("Consumer: mutex acquired -> removed item -> sem_post(empty)", Color::ACCENT_CYAN);
        true
    }

    pub fn tick(&mut self) {
        self.tick_counter = self.tick_counter.wrapping_add(1);
        if self.auto_run && self.tick_counter % 35 == 0 {
            if (self.tick_counter / 35) % 2 == 0 {
                self.produce();
            } else {
                self.consume();
            }
        }
    }

    pub fn handle_click(&mut self, rel_x: i32, rel_y: i32) {
        // [ Produce ] button: x: 16..116, y: 220..248
        if rel_x >= 16 && rel_x <= 116 && rel_y >= 220 && rel_y <= 248 {
            self.produce();
        }
        // [ Consume ] button: x: 126..226, y: 220..248
        else if rel_x >= 126 && rel_x <= 226 && rel_y >= 220 && rel_y <= 248 {
            self.consume();
        }
        // [ Toggle Auto ] button: x: 236..356, y: 220..248
        else if rel_x >= 236 && rel_x <= 356 && rel_y >= 220 && rel_y <= 248 {
            self.auto_run = !self.auto_run;
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::WIN_BG);

        // Header
        surface.draw_string(win_x + 16, win_y + 12, "Producer-Consumer Concurrency", Color::SLATE_DARK);
        surface.draw_string(win_x + 270, win_y + 14, "Bounded Buffer & Semaphores", Color::MUTED_TEXT);
        surface.draw_line_h(win_x + 16, win_y + 32, win_w.saturating_sub(32), Color::TASKBAR_BORDER);

        // Concurrency State Cards
        let card_w = (win_w - 48) / 3;
        let card_y = win_y + 42;

        // Card 1: Mutex Lock
        surface.draw_rect(win_x + 16, card_y, card_w, 48, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, card_y, card_w, 48, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, card_y + 8, "MUTEX LOCK:", Color::MUTED_TEXT);
        if self.mutex_locked {
            surface.draw_string(win_x + 24, card_y + 26, "LOCKED", Color::ACCENT_RED);
        } else {
            surface.draw_string(win_x + 24, card_y + 26, "UNLOCKED", Color::ACCENT_GREEN);
        }

        // Card 2: Empty Slots Semaphore
        let c2_x = win_x + 24 + card_w as i32;
        surface.draw_rect(c2_x, card_y, card_w, 48, Color::SLATE_CARD);
        surface.draw_rect_outline(c2_x, card_y, card_w, 48, Color::TASKBAR_BORDER);
        surface.draw_string(c2_x + 10, card_y + 8, "SEM_EMPTY (Empty):", Color::MUTED_TEXT);
        let empty_count = BUFFER_CAPACITY - self.count;
        let emp_buf = [b'0' + (empty_count as u8), b' ', b's', b'l', b'o', b't', b's'];
        if let Ok(s) = core::str::from_utf8(&emp_buf) {
            surface.draw_string(c2_x + 10, card_y + 26, s, Color::ACCENT_BLUE);
        }

        // Card 3: Full Slots Semaphore
        let c3_x = win_x + 32 + (card_w as i32 * 2);
        surface.draw_rect(c3_x, card_y, card_w, 48, Color::SLATE_CARD);
        surface.draw_rect_outline(c3_x, card_y, card_w, 48, Color::TASKBAR_BORDER);
        surface.draw_string(c3_x + 10, card_y + 8, "SEM_FULL (Items):", Color::MUTED_TEXT);
        let full_buf = [b'0' + (self.count as u8), b' ', b'i', b't', b'e', b'm', b's'];
        if let Ok(s) = core::str::from_utf8(&full_buf) {
            surface.draw_string(c3_x + 10, card_y + 26, s, Color::ACCENT_ORANGE);
        }

        // Bounded Buffer Visual Representation (8 Circular Slots)
        let buf_y = win_y + 104;
        surface.draw_string(win_x + 16, buf_y, "CIRCULAR BUFFER (Capacity = 8):", Color::SLATE_DARK);

        let slot_w = 46;
        let slot_h = 42;
        for i in 0..BUFFER_CAPACITY {
            let sx = win_x + 16 + (i as i32 * (slot_w + 8));
            let sy = buf_y + 20;

            let is_filled = self.buffer[i] != 0;
            let bg_color = if is_filled { Color::rgb(219, 234, 254) } else { Color::WHITE };
            let border_color = if is_filled { Color::ACCENT_BLUE } else { Color::TASKBAR_BORDER };

            surface.draw_rect(sx, sy, slot_w as usize, slot_h as usize, bg_color);
            surface.draw_rect_outline(sx, sy, slot_w as usize, slot_h as usize, border_color);

            // Slot label
            let mut slot_num = [b'['; 3];
            slot_num[1] = b'0' + i as u8;
            slot_num[2] = b']';
            if let Ok(s) = core::str::from_utf8(&slot_num) {
                surface.draw_string(sx + 10, sy + 4, s, Color::MUTED_TEXT);
            }

            if is_filled {
                let item_val = self.buffer[i];
                let it_buf = [b'#', b'0' + ((item_val % 10) as u8)];
                if let Ok(s) = core::str::from_utf8(&it_buf) {
                    surface.draw_string(sx + 14, sy + 22, s, Color::ACCENT_BLUE);
                }
            } else {
                surface.draw_string(sx + 18, sy + 22, ".", Color::TASKBAR_BORDER);
            }
        }

        // Event Log Box
        let log_y = win_y + 178;
        let log_w = win_w.saturating_sub(32);
        surface.draw_rect(win_x + 16, log_y, log_w, 28, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, log_y, log_w, 28, Color::TASKBAR_BORDER);
        surface.draw_string(win_x + 24, log_y + 6, self.last_action.0, self.last_action.1);

        // Control Buttons
        let btn_y = win_y + win_h as i32 - 40;

        // Button 1: Produce Item
        surface.draw_rect(win_x + 16, btn_y, 100, 28, Color::ACCENT_GREEN);
        surface.draw_rect_outline(win_x + 16, btn_y, 100, 28, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 22, btn_y + 6, "+ Produce", Color::WHITE);

        // Button 2: Consume Item
        surface.draw_rect(win_x + 126, btn_y, 100, 28, Color::ACCENT_CYAN);
        surface.draw_rect_outline(win_x + 126, btn_y, 100, 28, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 132, btn_y + 6, "- Consume", Color::WHITE);

        // Button 3: Toggle Auto Simulation
        let auto_bg = if self.auto_run { Color::ACCENT_ORANGE } else { Color::rgb(148, 163, 184) };
        surface.draw_rect(win_x + 236, btn_y, 120, 28, auto_bg);
        surface.draw_rect_outline(win_x + 236, btn_y, 120, 28, Color::WIN_TITLEBAR);
        if self.auto_run {
            surface.draw_string(win_x + 246, btn_y + 6, "Auto: ON", Color::WHITE);
        } else {
            surface.draw_string(win_x + 246, btn_y + 6, "Auto: OFF", Color::WHITE);
        }
    }
}
