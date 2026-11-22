extern crate alloc;

use alloc::string::String;
use alloc::vec::Vec;
use crate::apps::apt::AptApp;
use crate::apps::benchmark::BenchmarkApp;
use crate::apps::concurrency::ConcurrencyApp;
use crate::apps::dashboard::DashboardApp;
use crate::apps::files::FilesApp;
use crate::apps::snake::SnakeApp;
use crate::apps::terminal::TerminalApp;
use crate::apps::wifi::WifiApp;
use crate::apps::AppType;
use crate::graphics::surface::Surface;
use crate::window_manager::window::Window;

pub struct Compositor {
    pub windows: Vec<Window>,
    pub next_id: usize,
    pub active_id: Option<usize>,
    pub dragging_id: Option<usize>,
    pub drag_offset: (i32, i32),

    // Application state instances
    pub dashboard_app: DashboardApp,
    pub benchmark_app: BenchmarkApp,
    pub concurrency_app: ConcurrencyApp,
    pub snake_app: SnakeApp,
    pub apt_app: AptApp,
    pub wifi_app: WifiApp,
    pub terminal_app: TerminalApp,
    pub files_user_app: FilesApp,
    pub files_root_app: FilesApp,
}

impl Compositor {
    pub fn new() -> Self {
        Self {
            windows: Vec::new(),
            next_id: 1,
            active_id: None,
            dragging_id: None,
            drag_offset: (0, 0),

            dashboard_app: DashboardApp::new(),
            benchmark_app: BenchmarkApp::new(),
            concurrency_app: ConcurrencyApp::new(),
            snake_app: SnakeApp::new(),
            apt_app: AptApp::new(),
            wifi_app: WifiApp::new(),
            terminal_app: TerminalApp::new(),
            files_user_app: FilesApp::new(false),
            files_root_app: FilesApp::new(true),
        }
    }

    pub fn open_window(&mut self, title: &str, x: i32, y: i32, width: usize, height: usize, app_type: AppType) {
        // If window already exists, bring it to front
        for win in self.windows.iter_mut() {
            if win.app_type == app_type {
                win.is_minimized = false;
                let id = win.id;
                self.bring_to_front(id);
                return;
            }
        }

        let id = self.next_id;
        self.next_id += 1;
        let win = Window::new(id, String::from(title), x, y, width, height, app_type);
        self.windows.push(win);
        self.active_id = Some(id);
    }

    pub fn bring_to_front(&mut self, id: usize) {
        if let Some(pos) = self.windows.iter().position(|w| w.id == id) {
            let win = self.windows.remove(pos);
            self.windows.push(win);
            self.active_id = Some(id);
        }
    }

    pub fn close_window(&mut self, id: usize) {
        if let Some(pos) = self.windows.iter().position(|w| w.id == id) {
            self.windows.remove(pos);
            self.active_id = self.windows.last().map(|w| w.id);
        }
    }

    pub fn tick(&mut self) {
        self.dashboard_app.tick();
        self.benchmark_app.tick();
        self.concurrency_app.tick();
        self.snake_app.tick();

        if let Some(app) = self.terminal_app.take_pending_launch() {
            match app {
                AppType::Wifi => self.open_window("WiFi & Network Settings", 260, 50, 500, 320, app),
                _ => {}
            }
        }
    }

    pub fn handle_key(&mut self, key: char) {
        if let Some(active_id) = self.active_id {
            if let Some(win) = self.windows.iter().find(|w| w.id == active_id) {
                match win.app_type {
                    AppType::Terminal => self.terminal_app.handle_key(key),
                    AppType::Snake => self.snake_app.handle_key(key),
                    _ => {}
                }
            }
        }
    }

    pub fn handle_click(&mut self, mouse_x: i32, mouse_y: i32) -> bool {
        let mut hit = None;

        for i in (0..self.windows.len()).rev() {
            let win = &self.windows[i];
            if win.is_minimized {
                continue;
            }

            if mouse_x >= win.x && mouse_x < win.x + win.width as i32 && mouse_y >= win.y && mouse_y < win.y + win.height as i32 {
                let id = win.id;
                let is_close = win.is_close_button_clicked(mouse_x, mouse_y);
                let is_min = win.is_minimize_button_clicked(mouse_x, mouse_y);
                let is_titlebar = win.is_titlebar_clicked(mouse_x, mouse_y);
                let (cx, cy, _, _) = win.content_rect();
                let rel_x = mouse_x - cx;
                let rel_y = mouse_y - cy;
                let drag_x = mouse_x - win.x;
                let drag_y = mouse_y - win.y;

                hit = Some((id, is_close, is_min, is_titlebar, rel_x, rel_y, drag_x, drag_y, win.app_type));
                break;
            }
        }

        if let Some((id, is_close, is_min, is_titlebar, rel_x, rel_y, drag_x, drag_y, app_type)) = hit {
            if is_close {
                self.close_window(id);
                return true;
            }

            if is_min {
                if let Some(w) = self.windows.iter_mut().find(|w| w.id == id) {
                    w.is_minimized = true;
                }
                return true;
            }

            self.bring_to_front(id);

            if is_titlebar {
                self.dragging_id = Some(id);
                self.drag_offset = (drag_x, drag_y);
                return true;
            }

            match app_type {
                AppType::Benchmark => self.benchmark_app.handle_click(rel_x, rel_y),
                AppType::Concurrency => self.concurrency_app.handle_click(rel_x, rel_y),
                AppType::Snake => self.snake_app.handle_click(rel_x, rel_y),
                AppType::Apt => self.apt_app.handle_click(rel_x, rel_y),
                AppType::Wifi => self.wifi_app.handle_click(rel_x, rel_y),
                _ => {}
            }

            return true;
        }

        self.dragging_id = None;
        false
    }

    pub fn handle_drag(&mut self, mouse_x: i32, mouse_y: i32) {
        if let Some(id) = self.dragging_id {
            if let Some(win) = self.windows.iter_mut().find(|w| w.id == id) {
                win.x = (mouse_x - self.drag_offset.0).max(0);
                win.y = (mouse_y - self.drag_offset.1).max(0);
            }
        }
    }

    pub fn stop_drag(&mut self) {
        self.dragging_id = None;
    }

    pub fn render_windows(&self, surface: &mut Surface) {
        for win in self.windows.iter() {
            if win.is_minimized {
                continue;
            }

            let is_active = self.active_id == Some(win.id);
            win.render_frame(surface, is_active);

            let (cx, cy, cw, ch) = win.content_rect();
            match win.app_type {
                AppType::Dashboard => self.dashboard_app.render(surface, cx, cy, cw, ch),
                AppType::Benchmark => self.benchmark_app.render(surface, cx, cy, cw, ch),
                AppType::Concurrency => self.concurrency_app.render(surface, cx, cy, cw, ch),
                AppType::Snake => self.snake_app.render(surface, cx, cy, cw, ch),
                AppType::Apt => self.apt_app.render(surface, cx, cy, cw, ch),
                AppType::Wifi => self.wifi_app.render(surface, cx, cy, cw, ch),
                AppType::Terminal => self.terminal_app.render(surface, cx, cy, cw, ch),
                AppType::FileManager | AppType::Home => self.files_user_app.render(surface, cx, cy, cw, ch),
                AppType::FileSystem => self.files_root_app.render(surface, cx, cy, cw, ch),
            }
        }
    }
}
