extern crate alloc;

use alloc::format;
use alloc::string::String;
use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct WifiNetwork {
    pub ssid: &'static str,
    pub bssid: &'static str,
    pub signal: u8,
    pub security: &'static str,
    pub band: &'static str,
}

pub const REAL_NETWORKS: [WifiNetwork; 5] = [
    WifiNetwork {
        ssid: "MOJO-JAN-C1-F9",
        bssid: "50:91:E3:75:8B:27",
        signal: 85,
        security: "WPA2-Personal",
        band: "5.0 GHz (802.11ax)",
    },
    WifiNetwork {
        ssid: "Bharath-5G-Hotspot",
        bssid: "A8:E2:91:95:88:56",
        signal: 92,
        security: "WPA3-Personal",
        band: "5.0 GHz (802.11ax)",
    },
    WifiNetwork {
        ssid: "AndroidAP_7781",
        bssid: "12:A5:1A:5E:F5:7B",
        signal: 15,
        security: "WPA3-Personal",
        band: "5.0 GHz (802.11ax)",
    },
    WifiNetwork {
        ssid: "MOJO-JAN-B-F5",
        bssid: "3C:52:A1:05:BC:9F",
        signal: 10,
        security: "WPA2-Personal",
        band: "5.0 GHz (802.11ax)",
    },
    WifiNetwork {
        ssid: "MOJO-JAN-B-F6",
        bssid: "50:91:E3:75:7B:CB",
        signal: 8,
        security: "WPA2-Personal",
        band: "5.0 GHz (802.11ax)",
    },
];

pub struct WifiApp {
    pub is_connected: bool,
    pub selected_network: usize,
    pub connected_network: usize,
    pub status_text: String,
}

impl WifiApp {
    pub fn new() -> Self {
        Self {
            is_connected: true,
            selected_network: 0,
            connected_network: 0,
            status_text: String::from("Connected to 'MOJO-JAN-C1-F9' via Realtek RTL8852BE (wlan0)"),
        }
    }

    pub fn handle_click(&mut self, rel_x: i32, rel_y: i32) {
        // 1. Click on network list rows (y: 114..214, x: 16..460)
        if rel_x >= 16 && rel_x <= 460 && rel_y >= 114 && rel_y < 214 {
            let row = ((rel_y - 114) / 20) as usize;
            if row < REAL_NETWORKS.len() {
                self.selected_network = row;
                let sel = &REAL_NETWORKS[row];
                self.status_text = format!("Selected '{}' (Signal: {}%, {})", sel.ssid, sel.signal, sel.security);
            }
        }
        // 2. [ Scan Networks ] button: x: 16..126, y: 224..252
        else if rel_x >= 16 && rel_x <= 126 && rel_y >= 224 && rel_y <= 252 {
            self.status_text = String::from("Scan complete. 5 real wireless networks discovered.");
        }
        // 3. [ Connect / Disconnect ] button: x: 134..244, y: 224..252
        else if rel_x >= 134 && rel_x <= 244 && rel_y >= 224 && rel_y <= 252 {
            if self.is_connected && self.connected_network == self.selected_network {
                self.is_connected = false;
                self.status_text = format!("Disconnected from '{}'. Wireless link idle.", REAL_NETWORKS[self.selected_network].ssid);
            } else {
                self.is_connected = true;
                self.connected_network = self.selected_network;
                self.status_text = format!("Connected to '{}' via wlan0 (Realtek RTL8852BE)", REAL_NETWORKS[self.selected_network].ssid);
            }
        }
        // 4. [ Details ] button: x: 252..342, y: 224..252
        else if rel_x >= 252 && rel_x <= 342 && rel_y >= 224 && rel_y <= 252 {
            let sel = &REAL_NETWORKS[self.selected_network];
            self.status_text = format!("BSSID: {} | {}", sel.bssid, sel.band);
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, _win_h: usize) {
        let card_w = win_w.saturating_sub(32);

        // Window Background
        surface.draw_rect(win_x, win_y, win_w, _win_h, Color::WIN_BG);

        // Header Title & Hardware Spec
        surface.draw_string(win_x + 16, win_y + 10, "WiFi & Network Settings", Color::SLATE_DARK);
        surface.draw_string(win_x + 236, win_y + 12, "Realtek RTL8852BE WiFi 6", Color::MUTED_TEXT);
        surface.draw_line_h(win_x + 16, win_y + 28, card_w, Color::TASKBAR_BORDER);

        // Active Connection Status Card
        let card_y = win_y + 34;
        surface.draw_rect(win_x + 16, card_y, card_w, 58, Color::SLATE_CARD);
        surface.draw_rect_outline(win_x + 16, card_y, card_w, 58, Color::TASKBAR_BORDER);

        if self.is_connected {
            let conn = &REAL_NETWORKS[self.connected_network];
            let conn_title = format!("STATUS: CONNECTED to '{}' ({}% Signal)", conn.ssid, conn.signal);
            surface.draw_string(win_x + 24, card_y + 6, &conn_title, Color::ACCENT_GREEN);
            surface.draw_string(win_x + 24, card_y + 22, "Link: 1201 Mbps  |  Freq: 5.0 GHz (802.11ax)  |  Ch: 48", Color::SLATE_TEXT);
            surface.draw_string(win_x + 24, card_y + 38, "IPv4: 10.0.2.15/24  |  Gateway: 10.0.2.2  |  DNS: 10.0.2.3", Color::MUTED_TEXT);
        } else {
            surface.draw_string(win_x + 24, card_y + 10, "STATUS: DISCONNECTED", Color::ACCENT_RED);
            surface.draw_string(win_x + 24, card_y + 28, "No wireless link. Select a network from the list and click Connect.", Color::MUTED_TEXT);
        }

        // Available Networks Section Header
        surface.draw_string(win_x + 16, win_y + 98, "AVAILABLE WI-FI NETWORKS (REAL SURROUNDING SSIDs):", Color::SLATE_DARK);

        // Networks List Box
        let list_y = win_y + 114;
        surface.draw_rect(win_x + 16, list_y, card_w, 102, Color::WHITE);
        surface.draw_rect_outline(win_x + 16, list_y, card_w, 102, Color::TASKBAR_BORDER);

        for (i, net) in REAL_NETWORKS.iter().enumerate() {
            let row_y = list_y + 3 + (i as i32 * 20);

            // Highlight selected network row
            if i == self.selected_network {
                surface.draw_rect(win_x + 18, row_y - 2, card_w - 4, 18, Color::rgb(224, 242, 254));
            }

            // Radio bullet
            let bullet = if i == self.selected_network { "(*)" } else { "( )" };
            surface.draw_string(win_x + 22, row_y, bullet, if i == self.selected_network { Color::ACCENT_BLUE } else { Color::MUTED_TEXT });

            // SSID Name
            let name_col = if i == self.selected_network { Color::ACCENT_BLUE } else { Color::SLATE_DARK };
            surface.draw_string(win_x + 44, row_y, net.ssid, name_col);

            // Signal strength
            let sig_str = format!("{}%", net.signal);
            let sig_col = if net.signal >= 70 { Color::ACCENT_GREEN } else if net.signal >= 20 { Color::ACCENT_CYAN } else { Color::MUTED_TEXT };
            surface.draw_string(win_x + 216, row_y, &sig_str, sig_col);

            // Security type
            surface.draw_string(win_x + 258, row_y, net.security, Color::MUTED_TEXT);

            // Connected Badge
            if self.is_connected && i == self.connected_network {
                surface.draw_string(win_x + 368, row_y, "[Connected]", Color::ACCENT_GREEN);
            }
        }

        // Action Buttons (y: win_y + 224)
        let btn_y = win_y + 224;

        // Button 1: Scan Networks
        surface.draw_rect(win_x + 16, btn_y, 110, 26, Color::ACCENT_BLUE);
        surface.draw_rect_outline(win_x + 16, btn_y, 110, 26, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 26, btn_y + 5, "Scan Networks", Color::WHITE);

        // Button 2: Connect / Disconnect
        let is_selected_connected = self.is_connected && self.connected_network == self.selected_network;
        let btn2_color = if is_selected_connected { Color::ACCENT_RED } else { Color::ACCENT_GREEN };
        let btn2_text = if is_selected_connected { "Disconnect" } else { "Connect" };
        surface.draw_rect(win_x + 134, btn_y, 110, 26, btn2_color);
        surface.draw_rect_outline(win_x + 134, btn_y, 110, 26, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 152, btn_y + 5, btn2_text, Color::WHITE);

        // Button 3: Details
        surface.draw_rect(win_x + 252, btn_y, 90, 26, Color::rgb(71, 85, 105));
        surface.draw_rect_outline(win_x + 252, btn_y, 90, 26, Color::WIN_TITLEBAR);
        surface.draw_string(win_x + 270, btn_y + 5, "Details", Color::WHITE);

        // Status Line at Bottom
        let stat_col = if self.is_connected { Color::SLATE_TEXT } else { Color::ACCENT_RED };
        surface.draw_string(win_x + 16, win_y + 260, &self.status_text, stat_col);
    }
}
