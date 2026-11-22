use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum IconType {
    FileSystem,
    FileManager,
    Home,
    MicroOSDashboard,
    TerminalBash,
    BootTimeBenchmark,
    SnakeGame,
    AptPackageManager,
    ProducerConsumer,
    WifiNetwork,
    StartLogo,
}

pub fn draw_icon_24(surface: &mut Surface, x: i32, y: i32, icon: IconType) {
    match icon {
        IconType::FileSystem => {
            // White/silver disk server chassis with slot LEDs
            surface.draw_rect(x + 2, y + 2, 20, 6, Color::WHITE);
            surface.draw_rect_outline(x + 2, y + 2, 20, 6, Color::BLACK);
            surface.draw_rect(x + 17, y + 4, 3, 2, Color::ACCENT_GREEN);

            surface.draw_rect(x + 2, y + 9, 20, 6, Color::rgb(220, 225, 230));
            surface.draw_rect_outline(x + 2, y + 9, 20, 6, Color::BLACK);
            surface.draw_rect(x + 17, y + 11, 3, 2, Color::ACCENT_BLUE);

            surface.draw_rect(x + 2, y + 16, 20, 6, Color::rgb(200, 205, 215));
            surface.draw_rect_outline(x + 2, y + 16, 20, 6, Color::BLACK);
            surface.draw_rect(x + 17, y + 18, 3, 2, Color::ACCENT_GREEN);
        }
        IconType::FileManager => {
            // Golden yellow folder with tab
            surface.draw_rect(x + 2, y + 4, 8, 4, Color::rgb(217, 119, 6)); // dark gold tab
            surface.draw_rect(x + 2, y + 7, 20, 14, Color::ACCENT_YELLOW);
            surface.draw_rect_outline(x + 2, y + 7, 20, 14, Color::rgb(180, 83, 9));
            surface.draw_line_h(x + 4, y + 11, 16, Color::WHITE);
        }
        IconType::Home => {
            // Blue folder with roof/house accent
            surface.draw_rect(x + 2, y + 4, 8, 4, Color::rgb(29, 78, 216));
            surface.draw_rect(x + 2, y + 7, 20, 14, Color::ACCENT_BLUE);
            surface.draw_rect_outline(x + 2, y + 7, 20, 14, Color::rgb(30, 58, 138));
            // White home icon on folder
            surface.draw_rect(x + 8, y + 13, 8, 6, Color::WHITE);
            surface.draw_line_h(x + 7, y + 12, 10, Color::WHITE);
            surface.draw_line_h(x + 9, y + 10, 6, Color::WHITE);
            surface.draw_line_h(x + 11, y + 9, 2, Color::WHITE);
        }
        IconType::MicroOSDashboard => {
            // Window with gauge / telemetry graph
            surface.draw_rect(x + 2, y + 2, 20, 20, Color::WHITE);
            surface.draw_rect(x + 2, y + 2, 20, 5, Color::WIN_TITLEBAR);
            surface.draw_rect_outline(x + 2, y + 2, 20, 20, Color::BLACK);
            // Sparkline / gauge in dashboard
            surface.draw_rect(x + 5, y + 10, 14, 3, Color::ACCENT_CYAN);
            surface.draw_rect(x + 5, y + 15, 8, 3, Color::ACCENT_GREEN);
        }
        IconType::TerminalBash => {
            // Green CRT monitor with prompt `>_`
            surface.draw_rect(x + 2, y + 2, 20, 17, Color::BLACK);
            surface.draw_rect_outline(x + 2, y + 2, 20, 17, Color::rgb(100, 116, 139));
            surface.draw_rect(x + 8, y + 19, 8, 3, Color::rgb(71, 85, 105)); // stand
            surface.draw_rect(x + 5, y + 22, 14, 2, Color::rgb(51, 65, 85)); // base
            // Prompt `>_`
            surface.set_pixel(x + 5, y + 6, Color::TERMINAL_GREEN);
            surface.set_pixel(x + 6, y + 7, Color::TERMINAL_GREEN);
            surface.set_pixel(x + 7, y + 8, Color::TERMINAL_GREEN);
            surface.set_pixel(x + 6, y + 9, Color::TERMINAL_GREEN);
            surface.set_pixel(x + 5, y + 10, Color::TERMINAL_GREEN);
            surface.draw_line_h(x + 9, y + 11, 4, Color::TERMINAL_GREEN);
        }
        IconType::BootTimeBenchmark => {
            // Crossed wrench & tools / stopwatch
            surface.draw_rect(x + 3, y + 3, 18, 18, Color::WHITE);
            surface.draw_rect_outline(x + 3, y + 3, 18, 18, Color::SLATE_TEXT);
            // Blue tool diagonals
            surface.draw_line_h(x + 7, y + 7, 10, Color::ACCENT_BLUE);
            surface.draw_line_v(x + 12, y + 7, 10, Color::ACCENT_BLUE);
            surface.set_pixel(x + 12, y + 12, Color::ACCENT_RED);
        }
        IconType::SnakeGame => {
            // Playing cards / game icon
            surface.draw_rect(x + 3, y + 2, 14, 18, Color::WHITE);
            surface.draw_rect_outline(x + 3, y + 2, 14, 18, Color::BLACK);
            surface.draw_rect(x + 7, y + 4, 14, 18, Color::rgb(254, 242, 242));
            surface.draw_rect_outline(x + 7, y + 4, 14, 18, Color::ACCENT_RED);
            // Red heart / diamond
            surface.draw_rect(x + 12, y + 11, 4, 4, Color::ACCENT_RED);
            surface.set_pixel(x + 13, y + 10, Color::ACCENT_RED);
            surface.set_pixel(x + 14, y + 10, Color::ACCENT_RED);
            surface.set_pixel(x + 13, y + 15, Color::ACCENT_RED);
            surface.set_pixel(x + 14, y + 15, Color::ACCENT_RED);
        }
        IconType::AptPackageManager => {
            // Cardboard delivery box / crates
            surface.draw_rect(x + 3, y + 6, 18, 15, Color::rgb(217, 119, 6)); // box brown
            surface.draw_rect_outline(x + 3, y + 6, 18, 15, Color::rgb(146, 64, 14));
            // Box tape
            surface.draw_line_v(x + 12, y + 6, 15, Color::rgb(251, 191, 36));
            surface.draw_line_h(x + 3, y + 12, 18, Color::rgb(180, 83, 9));
        }
        IconType::ProducerConsumer => {
            // Cyan heartbeat pulse oscilloscope
            surface.draw_rect(x + 2, y + 2, 20, 20, Color::BLACK);
            surface.draw_rect_outline(x + 2, y + 2, 20, 20, Color::rgb(51, 65, 85));
            // Waveform pulse
            surface.draw_line_h(x + 4, y + 12, 4, Color::ACCENT_CYAN);
            surface.draw_line_v(x + 8, y + 6, 7, Color::ACCENT_CYAN);
            surface.draw_line_v(x + 11, y + 7, 11, Color::ACCENT_CYAN);
            surface.draw_line_v(x + 14, y + 10, 5, Color::ACCENT_CYAN);
            surface.draw_line_h(x + 15, y + 12, 5, Color::ACCENT_CYAN);
        }
        IconType::WifiNetwork => {
            // Red wifi antenna tower with broadcast waves
            // Tower
            surface.draw_line_v(x + 12, y + 10, 11, Color::ACCENT_RED);
            surface.draw_rect(x + 11, y + 8, 3, 3, Color::ACCENT_RED);
            // Stand
            surface.draw_line_h(x + 8, y + 21, 9, Color::ACCENT_RED);
            surface.set_pixel(x + 9, y + 19, Color::ACCENT_RED);
            surface.set_pixel(x + 15, y + 19, Color::ACCENT_RED);
            // Broadcast arcs
            surface.draw_line_h(x + 7, y + 4, 11, Color::ACCENT_RED);
            surface.draw_line_h(x + 9, y + 6, 7, Color::ACCENT_RED);
        }
        IconType::StartLogo => {
            // Blue leaf / feather logo for Start button (LXDE style)
            surface.draw_rect(x + 2, y + 4, 5, 10, Color::ACCENT_BLUE);
            surface.draw_rect(x + 5, y + 2, 6, 12, Color::ACCENT_CYAN);
            surface.draw_rect(x + 9, y + 4, 4, 8, Color::WHITE);
        }
    }
}
