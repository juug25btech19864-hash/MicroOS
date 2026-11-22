use pic8259::ChainedPics;
use spin::Mutex;

pub const PIC_1_OFFSET: u8 = 32;
pub const PIC_2_OFFSET: u8 = PIC_1_OFFSET + 8;

pub static PICS: Mutex<ChainedPics> =
    Mutex::new(unsafe { ChainedPics::new(PIC_1_OFFSET, PIC_2_OFFSET) });

#[derive(Debug, Clone, Copy)]
#[repr(u8)]
pub enum InterruptIndex {
    Timer = PIC_1_OFFSET,
    Keyboard = PIC_1_OFFSET + 1,
    PciNet = PIC_1_OFFSET + 11,
    Mouse = PIC_1_OFFSET + 12,
}

impl InterruptIndex {
    pub fn as_u8(self) -> u8 {
        self as u8
    }

    pub fn as_usize(self) -> usize {
        usize::from(self.as_u8())
    }
}

pub fn init() {
    unsafe {
        let mut pics = PICS.lock();
        pics.initialize();
        // Unmask required IRQs:
        // PIC1: IRQ0 (Timer), IRQ1 (Keyboard), IRQ2 (Cascade to Slave PIC)
        // PIC2: IRQ11 (VirtIO Net), IRQ12 (PS/2 Mouse)
        // Passing 0x00, 0x00 unmasks all hardware interrupts to guarantee responsiveness.
        pics.write_masks(0x00, 0x00);
    }
}
