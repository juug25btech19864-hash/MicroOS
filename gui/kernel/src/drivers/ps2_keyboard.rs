use pc_keyboard::{layouts, DecodedKey, HandleControl, KeyState, Keyboard, ScancodeSet1};
use spin::Mutex;

const KEY_BUFFER_SIZE: usize = 128;

pub struct KeyRingBuffer {
    buffer: [Option<DecodedKey>; KEY_BUFFER_SIZE],
    read_pos: usize,
    write_pos: usize,
}

impl KeyRingBuffer {
    const fn new() -> Self {
        Self {
            buffer: [None; KEY_BUFFER_SIZE],
            read_pos: 0,
            write_pos: 0,
        }
    }

    pub fn push(&mut self, key: DecodedKey) {
        let next = (self.write_pos + 1) % KEY_BUFFER_SIZE;
        if next != self.read_pos {
            self.buffer[self.write_pos] = Some(key);
            self.write_pos = next;
        }
    }

    pub fn pop(&mut self) -> Option<DecodedKey> {
        if self.read_pos == self.write_pos {
            None
        } else {
            let key = self.buffer[self.read_pos].take();
            self.read_pos = (self.read_pos + 1) % KEY_BUFFER_SIZE;
            key
        }
    }
}

pub static KEY_BUFFER: Mutex<KeyRingBuffer> = Mutex::new(KeyRingBuffer::new());

pub static KEYBOARD: Mutex<Option<Keyboard<layouts::Us104Key, ScancodeSet1>>> = Mutex::new(None);

pub fn init() {
    let keyboard = Keyboard::new(
        ScancodeSet1::new(),
        layouts::Us104Key,
        HandleControl::MapLettersToUnicode,
    );
    *KEYBOARD.lock() = Some(keyboard);
}

pub fn handle_scancode(scancode: u8) {
    if let Some(ref mut keyboard) = *KEYBOARD.lock() {
        if let Ok(Some(key_event)) = keyboard.add_byte(scancode) {
            if key_event.state == KeyState::Down {
                if let Some(key) = keyboard.process_keyevent(key_event) {
                    KEY_BUFFER.lock().push(key);
                }
            }
        }
    }
}

pub fn pop_key() -> Option<DecodedKey> {
    x86_64::instructions::interrupts::without_interrupts(|| {
        KEY_BUFFER.lock().pop()
    })
}
