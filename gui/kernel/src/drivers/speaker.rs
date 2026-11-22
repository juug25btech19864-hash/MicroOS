use x86_64::instructions::port::Port;

pub fn play_sound(freq_hz: u32) {
    if freq_hz == 0 {
        return;
    }
    let divisor = 1193180 / freq_hz;

    let mut pit_cmd = Port::<u8>::new(0x43);
    let mut pit_data = Port::<u8>::new(0x42);
    let mut speaker_port = Port::<u8>::new(0x61);

    unsafe {
        pit_cmd.write(0xB6);
        pit_data.write((divisor & 0xFF) as u8);
        pit_data.write(((divisor >> 8) & 0xFF) as u8);

        let tmp = speaker_port.read();
        if tmp != (tmp | 3) {
            speaker_port.write(tmp | 3);
        }
    }
}

pub fn stop_sound() {
    let mut speaker_port = Port::<u8>::new(0x61);
    unsafe {
        let tmp = speaker_port.read() & 0xFC;
        speaker_port.write(tmp);
    }
}

pub fn beep(freq_hz: u32) {
    play_sound(freq_hz);
    for _ in 0..500_000 {
        core::hint::spin_loop();
    }
    stop_sound();
}

pub fn play_startup_melody() {
    let notes = [523, 659, 784, 1046]; // C5, E5, G5, C6 (Major chord)
    for &note in &notes {
        play_sound(note);
        for _ in 0..300_000 {
            core::hint::spin_loop();
        }
        stop_sound();
        for _ in 0..50_000 {
            core::hint::spin_loop();
        }
    }
}
