pub mod boot_timer;
pub mod gdt;
pub mod idt;
pub mod pic;

pub fn init() {
    boot_timer::init();
    gdt::init();
    idt::init();
    pic::init();
}
