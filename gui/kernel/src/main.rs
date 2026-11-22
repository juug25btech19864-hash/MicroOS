#![no_std]
#![no_main]
#![feature(abi_x86_interrupt)]

extern crate alloc;

pub mod arch;
pub mod memory;
pub mod drivers;
pub mod graphics;
pub mod window_manager;
pub mod desktop;
pub mod apps;

use core::panic::PanicInfo;
use bootloader_api::{entry_point, BootInfo, BootloaderConfig};
use pc_keyboard::DecodedKey;

use crate::apps::AppType;
use crate::desktop::Desktop;
use crate::drivers::framebuffer::{self, FRAMEBUFFER};
use crate::drivers::ps2_mouse;
use crate::graphics::surface::Surface;
use crate::window_manager::compositor::Compositor;

pub static BOOTLOADER_CONFIG: BootloaderConfig = {
    let mut config = BootloaderConfig::new_default();
    config.mappings.physical_memory = Some(bootloader_api::config::Mapping::Dynamic);
    config.kernel_stack_size = 1024 * 1024; // 1 MiB stack
    config
};

entry_point!(kernel_main, config = &BOOTLOADER_CONFIG);

fn kernel_main(boot_info: &'static mut BootInfo) -> ! {
    // 1. Initialize Serial Port Logger (COM1 0x3F8)
    drivers::serial::init();
    println!("==================================================");
    println!("     MicroOS GUI Edition v1.0 (x86_64 Bare-Metal) ");
    println!("   Lightweight X11/LXDE Style Graphical Desktop   ");
    println!("==================================================");

    // 2. Initialize GDT, IDT, PIC, and CPU Interrupts
    arch::init();
    println!("[+] Hardware Interrupts & GDT/IDT active.");

    // 3. Initialize Memory Management & 16 MiB Heap Allocator
    let phys_mem_offset = boot_info
        .physical_memory_offset
        .into_option()
        .expect("Bootloader did not map physical memory");
    memory::init(phys_mem_offset, &boot_info.memory_regions);
    println!("[+] 4-Level Paging & 16 MiB Heap Allocator initialized.");

    // 4. Initialize Keyboard Subsystem
    drivers::ps2_keyboard::init();
    println!("[+] PS/2 Keyboard driver active.");

    // 5. Initialize Framebuffer
    let fb = boot_info.framebuffer.as_mut().expect("No framebuffer available");
    framebuffer::init(fb);
    let (screen_w, screen_h) = {
        let guard = FRAMEBUFFER.lock();
        let cfg = guard.as_ref().unwrap();
        (cfg.width, cfg.height)
    };
    println!("[+] UEFI GOP Framebuffer: {}x{}", screen_w, screen_h);

    // 6. Initialize Mouse Driver
    drivers::ps2_mouse::init(screen_w as i32, screen_h as i32);
    println!("[+] PS/2 Mouse driver initialized.");

    // 7. Desktop Compositor & GUI Shell
    let mut surface = Surface::new(screen_w, screen_h);
    let mut compositor = Compositor::new();
    let mut desktop = Desktop::new();

    // Open initial desktop application windows: Micro OS Dashboard, WiFi Settings & Terminal (Bash)
    desktop.launch_app(AppType::Dashboard, &mut compositor);
    desktop.launch_app(AppType::Wifi, &mut compositor);
    desktop.launch_app(AppType::Terminal, &mut compositor);

    println!("[+] MicroOS Graphical Desktop starting interactive event loop...");

    let mut last_left_button = false;
    let mut last_mouse_x = screen_w as i32 / 2;
    let mut last_mouse_y = screen_h as i32 / 2;

    // 8. Enable CPU Hardware Interrupts (STI)
    x86_64::instructions::interrupts::enable();
    println!("[+] CPU Interrupts enabled (STI). Ready for mouse and keyboard input.");

    loop {
        // A. Tick application animations & timers
        desktop.tick();
        compositor.tick();

        // B. Process Keyboard Inputs
        while let Some(decoded_key) = drivers::ps2_keyboard::pop_key() {
            match decoded_key {
                DecodedKey::Unicode(c) => {
                    compositor.handle_key(c);
                }
                DecodedKey::RawKey(k) => match k {
                    pc_keyboard::KeyCode::Return | pc_keyboard::KeyCode::NumpadEnter => {
                        compositor.handle_key('\n');
                    }
                    pc_keyboard::KeyCode::Backspace => {
                        compositor.handle_key('\x08');
                    }
                    pc_keyboard::KeyCode::Spacebar => {
                        compositor.handle_key(' ');
                    }
                    pc_keyboard::KeyCode::Tab => {
                        compositor.handle_key('\t');
                    }
                    pc_keyboard::KeyCode::ArrowUp => {
                        drivers::ps2_mouse::move_cursor_relative(0, -16);
                        compositor.handle_key('w');
                    }
                    pc_keyboard::KeyCode::ArrowDown => {
                        drivers::ps2_mouse::move_cursor_relative(0, 16);
                        compositor.handle_key('s');
                    }
                    pc_keyboard::KeyCode::ArrowLeft => {
                        drivers::ps2_mouse::move_cursor_relative(-16, 0);
                        compositor.handle_key('a');
                    }
                    pc_keyboard::KeyCode::ArrowRight => {
                        drivers::ps2_mouse::move_cursor_relative(16, 0);
                        compositor.handle_key('d');
                    }
                    pc_keyboard::KeyCode::F1 => {
                        let ms = ps2_mouse::get_mouse_state();
                        desktop.handle_click(ms.x, ms.y, &mut compositor, screen_w, screen_h);
                    }
                    _ => {}
                },
            }
        }

        // C. Process Mouse Inputs & Dragging
        let mouse_state = ps2_mouse::get_mouse_state();
        let mouse_x = mouse_state.x;
        let mouse_y = mouse_state.y;
        let left_clicked = mouse_state.left_button && !last_left_button;
        let left_released = !mouse_state.left_button && last_left_button;

        if left_clicked {
            desktop.handle_click(mouse_x, mouse_y, &mut compositor, screen_w, screen_h);
        } else if left_released {
            compositor.stop_drag();
        } else if mouse_state.left_button {
            let dx = mouse_x - last_mouse_x;
            let dy = mouse_y - last_mouse_y;
            if dx != 0 || dy != 0 {
                compositor.handle_drag(mouse_x, mouse_y);
            }
        }

        last_left_button = mouse_state.left_button;
        last_mouse_x = mouse_x;
        last_mouse_y = mouse_y;

        // D. Render Desktop Shell + Windows
        desktop.render(&mut surface, &compositor, screen_w, screen_h);

        // E. Render Mouse Cursor on Top
        surface.draw_mouse_cursor(mouse_x, mouse_y);

        // F. Blit Double-Buffered Surface to Screen Framebuffer
        {
            let mut fb_guard = FRAMEBUFFER.lock();
            if let Some(ref mut fb_cfg) = *fb_guard {
                surface.blit_to_framebuffer(fb_cfg);
            }
        }

        // G. Light CPU yield via halt
        x86_64::instructions::hlt();
    }
}

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    println!("\n[!] KERNEL PANIC: {}", info);
    loop {
        x86_64::instructions::hlt();
    }
}
