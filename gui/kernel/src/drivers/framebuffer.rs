use bootloader_api::info::{FrameBuffer, FrameBufferInfo, PixelFormat};
use spin::Mutex;

pub struct FramebufferConfig {
    pub width: usize,
    pub height: usize,
    pub stride: usize,
    pub bytes_per_pixel: usize,
    pub pixel_format: PixelFormat,
    pub buffer: *mut u8,
}

unsafe impl Send for FramebufferConfig {}
unsafe impl Sync for FramebufferConfig {}

pub static FRAMEBUFFER: Mutex<Option<FramebufferConfig>> = Mutex::new(None);

pub fn init(fb: &'static mut FrameBuffer) {
    let info: FrameBufferInfo = fb.info();
    let ptr = fb.buffer_mut().as_mut_ptr();

    let config = FramebufferConfig {
        width: info.width,
        height: info.height,
        stride: info.stride,
        bytes_per_pixel: info.bytes_per_pixel,
        pixel_format: info.pixel_format,
        buffer: ptr,
    };

    *FRAMEBUFFER.lock() = Some(config);
}
