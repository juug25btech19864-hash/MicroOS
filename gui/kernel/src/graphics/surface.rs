extern crate alloc;

use alloc::vec;
use alloc::vec::Vec;
use crate::drivers::framebuffer::FramebufferConfig;
use crate::graphics::color::Color;
use crate::graphics::font::{get_glyph, FONT_HEIGHT, FONT_WIDTH};

pub struct Surface {
    pub width: usize,
    pub height: usize,
    pub buffer: Vec<u32>,
}

impl Surface {
    pub fn new(width: usize, height: usize) -> Self {
        Self {
            width,
            height,
            buffer: vec![Color::DESKTOP_BG.to_u32_argb(); width * height],
        }
    }

    pub fn clear(&mut self, color: Color) {
        let val = color.to_u32_argb();
        self.buffer.fill(val);
    }

    #[inline(always)]
    pub fn set_pixel(&mut self, x: i32, y: i32, color: Color) {
        if x < 0 || y < 0 || (x as usize) >= self.width || (y as usize) >= self.height {
            return;
        }
        let idx = (y as usize) * self.width + (x as usize);
        if color.a == 255 {
            self.buffer[idx] = color.to_u32_argb();
        } else if color.a > 0 {
            let bg_val = self.buffer[idx];
            let bg = Color::rgba(
                ((bg_val >> 16) & 0xFF) as u8,
                ((bg_val >> 8) & 0xFF) as u8,
                (bg_val & 0xFF) as u8,
                ((bg_val >> 24) & 0xFF) as u8,
            );
            let blended = color.blend(bg);
            self.buffer[idx] = blended.to_u32_argb();
        }
    }

    pub fn draw_rect(&mut self, x: i32, y: i32, w: usize, h: usize, color: Color) {
        for dy in 0..h {
            for dx in 0..w {
                self.set_pixel(x + dx as i32, y + dy as i32, color);
            }
        }
    }

    pub fn draw_rect_outline(&mut self, x: i32, y: i32, w: usize, h: usize, color: Color) {
        if w == 0 || h == 0 {
            return;
        }
        self.draw_line_h(x, y, w, color);
        self.draw_line_h(x, y + h as i32 - 1, w, color);
        self.draw_line_v(x, y, h, color);
        self.draw_line_v(x + w as i32 - 1, y, h, color);
    }

    pub fn draw_line_h(&mut self, x: i32, y: i32, length: usize, color: Color) {
        for dx in 0..length {
            self.set_pixel(x + dx as i32, y, color);
        }
    }

    pub fn draw_line_v(&mut self, x: i32, y: i32, length: usize, color: Color) {
        for dy in 0..length {
            self.set_pixel(x, y + dy as i32, color);
        }
    }

    pub fn draw_char(&mut self, x: i32, y: i32, c: char, color: Color) {
        let glyph = get_glyph(c);
        for row in 0..FONT_HEIGHT {
            let byte = glyph[row];
            for col in 0..FONT_WIDTH {
                if (byte & (0x80 >> col)) != 0 {
                    self.set_pixel(x + col as i32, y + row as i32, color);
                }
            }
        }
    }

    pub fn draw_string(&mut self, x: i32, y: i32, text: &str, color: Color) {
        let mut cur_x = x;
        for c in text.chars() {
            self.draw_char(cur_x, y, c, color);
            cur_x += FONT_WIDTH as i32;
        }
    }

    /// Modern White Arrow Mouse Cursor with Crisp Dark Outline
    pub fn draw_mouse_cursor(&mut self, x: i32, y: i32) {
        const CURSOR_BITMAP: [&str; 19] = [
            "#               ",
            "##              ",
            "#.#             ",
            "#..#            ",
            "#...#           ",
            "#....#          ",
            "#.....#         ",
            "#......#        ",
            "#.......#       ",
            "#........#      ",
            "#.....#####     ",
            "#..#..#         ",
            "#.# #..#        ",
            "##   #..#       ",
            "#     #..#      ",
            "       #..#     ",
            "        ##      ",
            "                ",
            "                ",
        ];

        for (row, line) in CURSOR_BITMAP.iter().enumerate() {
            for (col, ch) in line.chars().enumerate() {
                match ch {
                    '#' => self.set_pixel(x + col as i32, y + row as i32, Color::BLACK),
                    '.' => self.set_pixel(x + col as i32, y + row as i32, Color::WHITE),
                    _ => {}
                }
            }
        }
    }

    pub fn blit_to_framebuffer(&self, fb: &mut FramebufferConfig) {
        let copy_width = self.width.min(fb.width);
        let copy_height = self.height.min(fb.height);
        let bpp = fb.bytes_per_pixel;
        let is_bgr = fb.pixel_format == bootloader_api::info::PixelFormat::Bgr;

        if is_bgr && bpp == 4 {
            for y in 0..copy_height {
                let src_row = &self.buffer[y * self.width..y * self.width + copy_width];
                let dst_ptr = unsafe { (fb.buffer as *mut u32).add(y * fb.stride) };
                for (i, &val) in src_row.iter().enumerate() {
                    unsafe {
                        *dst_ptr.add(i) = val | 0xFF000000;
                    }
                }
            }
            return;
        }

        for y in 0..copy_height {
            let src_row_start = y * self.width;
            let dst_row_start = y * fb.stride * bpp;

            for x in 0..copy_width {
                let val = self.buffer[src_row_start + x];
                let r = ((val >> 16) & 0xFF) as u8;
                let g = ((val >> 8) & 0xFF) as u8;
                let b = (val & 0xFF) as u8;

                let dst_offset = dst_row_start + x * bpp;
                unsafe {
                    let ptr = fb.buffer.add(dst_offset);
                    if is_bgr {
                        *ptr = b;
                        *ptr.add(1) = g;
                        *ptr.add(2) = r;
                    } else {
                        *ptr = r;
                        *ptr.add(1) = g;
                        *ptr.add(2) = b;
                    }
                    if bpp == 4 {
                        *ptr.add(3) = 0xFF;
                    }
                }
            }
        }
    }
}
