use core::sync::atomic::{AtomicBool, AtomicI32, Ordering};
use spin::Mutex;
use x86_64::instructions::port::Port;

#[derive(Debug, Clone, Copy)]
pub struct MouseState {
    pub x: i32,
    pub y: i32,
    pub scroll_delta: i32,
    pub left_button: bool,
    pub right_button: bool,
    pub middle_button: bool,
    pub left_clicked: bool,
    pub right_clicked: bool,
}

pub static MOUSE: Mutex<MouseState> = Mutex::new(MouseState {
    x: 400,
    y: 300,
    scroll_delta: 0,
    left_button: false,
    right_button: false,
    middle_button: false,
    left_clicked: false,
    right_clicked: false,
});

pub static HAS_SCROLL_WHEEL: AtomicBool = AtomicBool::new(false);
static mut PACKET_CYCLE: u8 = 0;
static mut PACKET_BYTES: [u8; 4] = [0; 4];
static mut LAST_LEFT: bool = false;
static mut LAST_RIGHT: bool = false;

pub static SCREEN_WIDTH: AtomicI32 = AtomicI32::new(800);
pub static SCREEN_HEIGHT: AtomicI32 = AtomicI32::new(600);

pub fn set_screen_bounds(width: i32, height: i32) {
    SCREEN_WIDTH.store(width, Ordering::Relaxed);
    SCREEN_HEIGHT.store(height, Ordering::Relaxed);
}

fn mouse_wait_input() {
    let mut status_port = Port::<u8>::new(0x64);
    for _ in 0..200_000 {
        if unsafe { status_port.read() } & 0x02 == 0 {
            return;
        }
        core::hint::spin_loop();
    }
}

fn mouse_wait_output() -> bool {
    let mut status_port = Port::<u8>::new(0x64);
    for _ in 0..200_000 {
        if unsafe { status_port.read() } & 0x01 != 0 {
            return true;
        }
        core::hint::spin_loop();
    }
    false
}

fn mouse_write(byte: u8) {
    let mut cmd_port = Port::<u8>::new(0x64);
    let mut data_port = Port::<u8>::new(0x60);
    mouse_wait_input();
    unsafe { cmd_port.write(0xD4) };
    mouse_wait_input();
    unsafe { data_port.write(byte) };
}

fn mouse_read() -> u8 {
    let mut data_port = Port::<u8>::new(0x60);
    if mouse_wait_output() {
        unsafe { data_port.read() }
    } else {
        0
    }
}

pub fn init(width: i32, height: i32) {
    set_screen_bounds(width, height);

    let mut cmd_port = Port::<u8>::new(0x64);
    let mut data_port = Port::<u8>::new(0x60);

    // Flush any pending data from output buffer
    for _ in 0..100 {
        if (unsafe { cmd_port.read() } & 0x01) != 0 {
            let _ = unsafe { data_port.read() };
        } else {
            break;
        }
    }

    // Enable first PS/2 port (keyboard) and auxiliary device (mouse)
    mouse_wait_input();
    unsafe { cmd_port.write(0xAE) };
    mouse_wait_input();
    unsafe { cmd_port.write(0xA8) };

    // Read controller configuration byte
    mouse_wait_input();
    unsafe { cmd_port.write(0x20) };
    let mut status = mouse_read();

    // Enable interrupts (bit 0 = keyboard, bit 1 = mouse)
    // Clear clock disable (bit 4 = keyboard, bit 5 = mouse)
    status |= 0x03;
    status &= !(0x30);

    // Write updated configuration byte
    mouse_wait_input();
    unsafe { cmd_port.write(0x60) };
    mouse_wait_input();
    unsafe { data_port.write(status) };

    // Reset mouse to default settings
    mouse_write(0xF6);
    let _ = mouse_read(); // ACK

    // Enable data reporting
    mouse_write(0xF4);
    let _ = mouse_read(); // ACK

    let mut state = MOUSE.lock();
    state.x = width / 2;
    state.y = height / 2;
    state.scroll_delta = 0;
    crate::println!("[+] PS/2 Mouse streaming enabled (8042 aux port).");
}

pub fn move_cursor_relative(dx: i32, dy: i32) {
    let max_w = SCREEN_WIDTH.load(Ordering::Relaxed);
    let max_h = SCREEN_HEIGHT.load(Ordering::Relaxed);
    let mut state = MOUSE.lock();
    state.x = (state.x + dx).clamp(0, max_w - 1);
    state.y = (state.y + dy).clamp(0, max_h - 1);
}

pub fn simulate_click(left: bool) {
    let mut state = MOUSE.lock();
    if left {
        state.left_button = true;
        state.left_clicked = true;
    } else {
        state.right_button = true;
        state.right_clicked = true;
    }
}

pub fn simulate_release() {
    let mut state = MOUSE.lock();
    state.left_button = false;
    state.right_button = false;
}

pub fn handle_packet_byte(byte: u8) {
    unsafe {
        match PACKET_CYCLE {
            0 => {
                // Bit 3 must be 1 for a valid PS/2 mouse first packet byte
                if (byte & 0x08) != 0 {
                    PACKET_BYTES[0] = byte;
                    PACKET_CYCLE = 1;
                }
            }
            1 => {
                PACKET_BYTES[1] = byte;
                PACKET_CYCLE = 2;
            }
            2 => {
                PACKET_BYTES[2] = byte;
                PACKET_CYCLE = 0;
                process_packet(PACKET_BYTES[0], PACKET_BYTES[1], PACKET_BYTES[2], 0);
            }
            _ => {
                PACKET_CYCLE = 0;
            }
        }
    }
}

fn process_packet(b0: u8, b1: u8, b2: u8, b3: u8) {
    unsafe {
        // Check for overflow bits
        if (b0 & 0xC0) != 0 {
            return;
        }

        let mut rel_x = b1 as i16;
        let mut rel_y = b2 as i16;

        // Sign extension
        if (b0 & 0x10) != 0 {
            rel_x |= 0xFF00u16 as i16;
        }
        if (b0 & 0x20) != 0 {
            rel_y |= 0xFF00u16 as i16;
        }

        // In PS/2, positive Y is UP, but in screen coordinates positive Y is DOWN
        rel_y = -rel_y;

        let left = (b0 & 0x01) != 0;
        let right = (b0 & 0x02) != 0;
        let middle = (b0 & 0x04) != 0;

        let left_click = left && !LAST_LEFT;
        let right_click = right && !LAST_RIGHT;

        LAST_LEFT = left;
        LAST_RIGHT = right;

        let max_w = SCREEN_WIDTH.load(Ordering::Relaxed);
        let max_h = SCREEN_HEIGHT.load(Ordering::Relaxed);

        // Smooth, Precise & Comfortable Mouse Speed:
        // 1x base speed for pixel-accurate desktop control, with gentle 2x acceleration on fast swipes
        let abs_x = rel_x.abs() as i32;
        let abs_y = rel_y.abs() as i32;
        let mult_x = if abs_x > 10 { 2 } else { 1 };
        let mult_y = if abs_y > 10 { 2 } else { 1 };

        let delta_x = (rel_x as i32) * mult_x;
        let delta_y = (rel_y as i32) * mult_y;

        // Microsoft PS/2 IntelliMouse 4-byte packet scroll wheel decoding:
        // Byte 3 bits 0..3 are 4-bit 2's complement value:
        //   0x01..0x07 = +1..+7 (Scroll UP)
        //   0x08..0x0F = -8..-1 (Scroll DOWN, e.g. 0x0F is -1)
        let low_nibble = b3 & 0x0F;
        let scroll = if low_nibble != 0 {
            if (low_nibble & 0x08) != 0 {
                (low_nibble as i32) - 16 // Sign-extend 4-bit negative: 15 - 16 = -1 (DOWN)
            } else {
                low_nibble as i32 // Positive: +1 (UP)
            }
        } else {
            0
        };

        let mut state = MOUSE.lock();
        state.x = (state.x + delta_x).clamp(0, max_w - 1);
        state.y = (state.y + delta_y).clamp(0, max_h - 1);
        state.scroll_delta += scroll;
        state.left_button = left;
        state.right_button = right;
        state.middle_button = middle;
        if left_click {
            state.left_clicked = true;
        }
        if right_click {
            state.right_clicked = true;
        }
    }
}

pub fn get_mouse_state() -> MouseState {
    x86_64::instructions::interrupts::without_interrupts(|| {
        let mut state = MOUSE.lock();
        let current = *state;
        // Clear edge-triggered click & scroll flags on read
        state.left_clicked = false;
        state.right_clicked = false;
        state.scroll_delta = 0;
        current
    })
}
