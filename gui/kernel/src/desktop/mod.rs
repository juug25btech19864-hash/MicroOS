pub mod taskbar;
pub mod start_menu;

use crate::apps::AppType;
use crate::graphics::color::Color;
use crate::graphics::icons::{draw_icon_24, IconType};
use crate::graphics::surface::Surface;
use crate::window_manager::compositor::Compositor;
use self::start_menu::{handle_start_click, render_start_menu, StartMenuAction};
use self::taskbar::{render_taskbar, TASKBAR_HEIGHT};

pub struct Desktop {
    pub start_menu_open: bool,
    pub tick_counter: u32,
}

impl Desktop {
    pub fn new() -> Self {
        Self {
            start_menu_open: false,
            tick_counter: 0,
        }
    }

    pub fn tick(&mut self) {
        self.tick_counter = self.tick_counter.wrapping_add(1);
    }

    pub fn render(&self, surface: &mut Surface, compositor: &Compositor, screen_w: usize, screen_h: usize) {
        // 1. Sleek Slate-Blue Desktop Background
        surface.clear(Color::DESKTOP_BG);

        // 2. Desktop Application Icons in Two Columns matching user reference
        let col1_x = 24;
        let col2_x = 156;

        let desktop_icons = [
            // Column 1
            ("File System", IconType::FileSystem, col1_x, 20),
            ("Home", IconType::Home, col1_x, 96),
            ("Terminal (Bash)", IconType::TerminalBash, col1_x, 172),
            ("Snake Game", IconType::SnakeGame, col1_x, 248),
            ("Producer-Consumer...", IconType::ProducerConsumer, col1_x, 324),
            ("WiFi & Network Se...", IconType::WifiNetwork, col1_x, 400),

            // Column 2
            ("File Manager", IconType::FileManager, col2_x, 20),
            ("Micro OS Dashboard", IconType::MicroOSDashboard, col2_x, 96),
            ("Boot Time Benchmark", IconType::BootTimeBenchmark, col2_x, 172),
            ("APT Package Manager", IconType::AptPackageManager, col2_x, 248),
        ];

        for (name, icon, ix, iy) in desktop_icons.iter() {
            // Draw Icon (centered horizontally over text)
            let icon_offset = if *ix == col1_x { 24 } else { 24 };
            draw_icon_24(surface, *ix + icon_offset, *iy, *icon);
            // Draw Label
            surface.draw_string(*ix, *iy + 30, name, Color::WHITE);
        }

        // 3. Render Open Windows
        compositor.render_windows(surface);

        // 4. Render Bottom Taskbar
        render_taskbar(surface, compositor, self.start_menu_open, screen_w, screen_h, self.tick_counter);

        // 5. Render Start Menu (if open)
        if self.start_menu_open {
            render_start_menu(surface, screen_h);
        }
    }

    pub fn handle_click(&mut self, mouse_x: i32, mouse_y: i32, compositor: &mut Compositor, _screen_w: usize, screen_h: usize) -> bool {
        let taskbar_y = (screen_h - TASKBAR_HEIGHT) as i32;

        // 1. If Start Menu is open
        if self.start_menu_open {
            if let Some(action) = handle_start_click(mouse_x, mouse_y, screen_h) {
                match action {
                    StartMenuAction::Launch(app) => {
                        self.start_menu_open = false;
                        self.launch_app(app, compositor);
                        return true;
                    }
                    StartMenuAction::Shutdown => {
                        self.start_menu_open = false;
                        unsafe {
                            // ACPI / VirtualBox poweroff
                            x86_64::instructions::port::Port::<u16>::new(0x604).write(0x2000);
                            x86_64::instructions::port::Port::<u16>::new(0x4004).write(0x3400);
                        }
                        return true;
                    }
                    StartMenuAction::Restart => {
                        self.start_menu_open = false;
                        unsafe {
                            // PS/2 8042 CPU Reset
                            x86_64::instructions::port::Port::<u8>::new(0x64).write(0xFE);
                        }
                        return true;
                    }
                    StartMenuAction::InsideMenu => return true,
                }
            } else {
                self.start_menu_open = false;
            }
        }

        // 2. Taskbar Click
        if mouse_y >= taskbar_y {
            // Start button click (x: 4..80)
            if mouse_x >= 4 && mouse_x <= 80 {
                self.start_menu_open = !self.start_menu_open;
                return true;
            }

            // Window Tabs in Taskbar
            let mut tab_x = 88;
            for win in compositor.windows.iter_mut() {
                let tab_w = 140;
                if mouse_x >= tab_x && mouse_x < tab_x + tab_w {
                    win.is_minimized = !win.is_minimized;
                    if !win.is_minimized {
                        let id = win.id;
                        compositor.bring_to_front(id);
                    }
                    return true;
                }
                tab_x += tab_w + 4;
            }

            return true;
        }

        // 3. Window Click
        if compositor.handle_click(mouse_x, mouse_y) {
            return true;
        }

        // 4. Desktop Icon Click
        let col1_x = 24;
        let col2_x = 156;

        let icon_targets = [
            (AppType::FileSystem, col1_x, 20),
            (AppType::Home, col1_x, 96),
            (AppType::Terminal, col1_x, 172),
            (AppType::Snake, col1_x, 248),
            (AppType::Concurrency, col1_x, 324),
            (AppType::Wifi, col1_x, 400),

            (AppType::FileManager, col2_x, 20),
            (AppType::Dashboard, col2_x, 96),
            (AppType::Benchmark, col2_x, 172),
            (AppType::Apt, col2_x, 248),
        ];

        for (app, ix, iy) in icon_targets.iter() {
            if mouse_x >= *ix && mouse_x < *ix + 120 && mouse_y >= *iy && mouse_y < *iy + 65 {
                self.launch_app(*app, compositor);
                return true;
            }
        }

        false
    }

    pub fn launch_app(&self, app: AppType, compositor: &mut Compositor) {
        match app {
            AppType::Dashboard => {
                compositor.open_window("MicroOS Dashboard", 320, 30, 460, 290, app);
            }
            AppType::Benchmark => {
                compositor.open_window("Boot Time Benchmark", 330, 50, 450, 300, app);
            }
            AppType::Concurrency => {
                compositor.open_window("Producer-Consumer Concurrency", 310, 60, 470, 280, app);
            }
            AppType::Snake => {
                compositor.open_window("Micro Arcade: Snake Game", 340, 40, 420, 290, app);
            }
            AppType::Apt => {
                compositor.open_window("APT Package Manager", 330, 50, 450, 300, app);
            }
            AppType::Wifi => {
                compositor.open_window("WiFi & Network Settings", 260, 50, 500, 320, app);
            }
            AppType::Terminal => {
                compositor.open_window("Terminal (Bash)", 250, 70, 520, 350, app);
            }
            AppType::FileManager => {
                compositor.open_window("File Manager", 330, 50, 450, 280, app);
            }
            AppType::FileSystem => {
                compositor.open_window("File System (/)", 320, 40, 450, 280, app);
            }
            AppType::Home => {
                compositor.open_window("Home (/home/micro)", 330, 50, 450, 280, app);
            }
        }
    }
}
