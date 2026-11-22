#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub struct Color {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

impl Color {
    pub const fn rgb(r: u8, g: u8, b: u8) -> Self {
        Self { r, g, b, a: 255 }
    }

    pub const fn rgba(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self { r, g, b, a }
    }

    // MicroOS Desktop Theme Palette
    pub const DESKTOP_BG: Color = Color::rgb(52, 69, 88);         // Dark Slate Blue #344558
    pub const TASKBAR_BG: Color = Color::rgb(226, 230, 235);      // Clean Light Grey #E2E6EB
    pub const TASKBAR_BORDER: Color = Color::rgb(180, 188, 200);  // Subtle Border #B4BCC8
    pub const TASKBAR_ACTIVE: Color = Color::rgb(205, 212, 222);  // Active window tab
    pub const TASKBAR_TEXT: Color = Color::rgb(30, 41, 59);       // Slate-900

    // Window Theme
    pub const WIN_TITLEBAR: Color = Color::rgb(44, 59, 77);       // Deep Slate #2C3B4D
    pub const WIN_TITLEBAR_INACTIVE: Color = Color::rgb(90, 104, 120);
    pub const WIN_TITLE_TEXT: Color = Color::rgb(255, 255, 255);
    pub const WIN_BG: Color = Color::rgb(248, 250, 252);          // Light Slate #F8FAFC
    pub const WIN_BORDER: Color = Color::rgb(32, 44, 58);
    pub const WIN_CONTROL_RED: Color = Color::rgb(239, 68, 68);   // Close button
    pub const WIN_CONTROL_YELLOW: Color = Color::rgb(245, 158, 11);
    pub const WIN_CONTROL_GREEN: Color = Color::rgb(34, 197, 94);

    // Standard UI Colors
    pub const WHITE: Color = Color::rgb(255, 255, 255);
    pub const BLACK: Color = Color::rgb(0, 0, 0);
    pub const SLATE_DARK: Color = Color::rgb(15, 23, 42);
    pub const SLATE_CARD: Color = Color::rgb(238, 242, 246);
    pub const SLATE_TEXT: Color = Color::rgb(51, 65, 85);
    pub const MUTED_TEXT: Color = Color::rgb(100, 116, 139);
    pub const ACCENT_BLUE: Color = Color::rgb(37, 99, 235);       // #2563EB
    pub const ACCENT_CYAN: Color = Color::rgb(6, 182, 212);       // #06B6D4
    pub const ACCENT_GREEN: Color = Color::rgb(34, 197, 94);      // #22C55E
    pub const ACCENT_YELLOW: Color = Color::rgb(234, 179, 8);     // #EAB308
    pub const ACCENT_ORANGE: Color = Color::rgb(249, 115, 22);    // #F97316
    pub const ACCENT_RED: Color = Color::rgb(239, 68, 68);        // #EF4444
    pub const TERMINAL_BG: Color = Color::rgb(10, 15, 25);
    pub const TERMINAL_GREEN: Color = Color::rgb(74, 222, 128);

    pub fn to_u32_argb(self) -> u32 {
        ((self.a as u32) << 24)
            | ((self.r as u32) << 16)
            | ((self.g as u32) << 8)
            | (self.b as u32)
    }

    pub fn blend(self, bg: Color) -> Color {
        let alpha = self.a as u32;
        let inv_alpha = 255 - alpha;
        Color {
            r: (((self.r as u32 * alpha) + (bg.r as u32 * inv_alpha)) / 255) as u8,
            g: (((self.g as u32 * alpha) + (bg.g as u32 * inv_alpha)) / 255) as u8,
            b: (((self.b as u32 * alpha) + (bg.b as u32 * inv_alpha)) / 255) as u8,
            a: 255,
        }
    }
}
