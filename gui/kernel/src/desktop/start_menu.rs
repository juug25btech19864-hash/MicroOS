use crate::apps::AppType;
use crate::graphics::color::Color;
use crate::graphics::icons::{draw_icon_24, IconType};
use crate::graphics::surface::Surface;

pub const START_MENU_WIDTH: usize = 250;
pub const START_MENU_HEIGHT: usize = 310;

pub enum StartMenuAction {
    Launch(AppType),
    Shutdown,
    Restart,
    InsideMenu,
}

pub fn render_start_menu(surface: &mut Surface, screen_h: usize) {
    let menu_x = 4;
    let menu_y = (screen_h - 32 - START_MENU_HEIGHT) as i32;

    // Drop shadow
    surface.draw_rect(menu_x + 3, menu_y + 3, START_MENU_WIDTH, START_MENU_HEIGHT, Color::rgba(0, 0, 0, 80));

    // Background & Border
    surface.draw_rect(menu_x, menu_y, START_MENU_WIDTH, START_MENU_HEIGHT, Color::WHITE);
    surface.draw_rect_outline(menu_x, menu_y, START_MENU_WIDTH, START_MENU_HEIGHT, Color::WIN_BORDER);

    // Header Banner
    surface.draw_rect(menu_x, menu_y, START_MENU_WIDTH, 44, Color::WIN_TITLEBAR);
    draw_icon_24(surface, menu_x + 8, menu_y + 10, IconType::StartLogo);
    surface.draw_string(menu_x + 38, menu_y + 10, "MicroOS Desktop", Color::WHITE);
    surface.draw_string(menu_x + 38, menu_y + 26, "User: micro (Administrator)", Color::rgb(148, 163, 184));

    // Application List Items
    let items = [
        ("Micro OS Dashboard", IconType::MicroOSDashboard),
        ("Boot Time Benchmark", IconType::BootTimeBenchmark),
        ("Producer-Consumer", IconType::ProducerConsumer),
        ("Terminal (Bash)", IconType::TerminalBash),
        ("Snake Game", IconType::SnakeGame),
        ("APT Package Manager", IconType::AptPackageManager),
        ("WiFi & Network", IconType::WifiNetwork),
        ("File Manager", IconType::FileManager),
    ];

    let start_items_y = menu_y + 50;
    for (i, (label, icon)) in items.iter().enumerate() {
        let item_y = start_items_y + (i as i32 * 26);
        draw_icon_24(surface, menu_x + 8, item_y + 1, *icon);
        surface.draw_string(menu_x + 38, item_y + 5, label, Color::SLATE_DARK);
    }

    // Divider
    let div_y = start_items_y + (items.len() as i32 * 26) + 4;
    surface.draw_line_h(menu_x + 8, div_y, START_MENU_WIDTH - 16, Color::TASKBAR_BORDER);

    // Power Actions
    let power_y = div_y + 8;
    surface.draw_rect(menu_x + 12, power_y, 105, 24, Color::SLATE_CARD);
    surface.draw_rect_outline(menu_x + 12, power_y, 105, 24, Color::TASKBAR_BORDER);
    surface.draw_string(menu_x + 24, power_y + 5, "Restart", Color::SLATE_DARK);

    surface.draw_rect(menu_x + 128, power_y, 105, 24, Color::rgb(254, 226, 226));
    surface.draw_rect_outline(menu_x + 128, power_y, 105, 24, Color::ACCENT_RED);
    surface.draw_string(menu_x + 140, power_y + 5, "Shut Down", Color::ACCENT_RED);
}

pub fn handle_start_click(click_x: i32, click_y: i32, screen_h: usize) -> Option<StartMenuAction> {
    let menu_x = 4;
    let menu_y = (screen_h - 32 - START_MENU_HEIGHT) as i32;

    if click_x < menu_x || click_x >= menu_x + START_MENU_WIDTH as i32 || click_y < menu_y || click_y >= menu_y + START_MENU_HEIGHT as i32 {
        return None;
    }

    let items_start_y = menu_y + 50;
    let items = [
        AppType::Dashboard,
        AppType::Benchmark,
        AppType::Concurrency,
        AppType::Terminal,
        AppType::Snake,
        AppType::Apt,
        AppType::Wifi,
        AppType::FileManager,
    ];

    for (i, app) in items.iter().enumerate() {
        let item_y = items_start_y + (i as i32 * 26);
        if click_y >= item_y && click_y < item_y + 26 {
            return Some(StartMenuAction::Launch(*app));
        }
    }

    // Power buttons
    let div_y = items_start_y + (items.len() as i32 * 26) + 4;
    let power_y = div_y + 8;

    // Restart button
    if click_x >= menu_x + 12 && click_x < menu_x + 117 && click_y >= power_y && click_y < power_y + 24 {
        return Some(StartMenuAction::Restart);
    }

    // Shutdown button
    if click_x >= menu_x + 128 && click_x < menu_x + 233 && click_y >= power_y && click_y < power_y + 24 {
        return Some(StartMenuAction::Shutdown);
    }

    Some(StartMenuAction::InsideMenu)
}
