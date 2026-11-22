use core::sync::atomic::{AtomicU64, Ordering};
use spin::Mutex;

#[derive(Debug, Clone, Copy)]
pub struct BootStage {
    pub name: &'static str,
    pub duration_ms: u64,
}

pub static BOOT_START_TSC: AtomicU64 = AtomicU64::new(0);
pub static BOOT_STAGES: Mutex<[Option<BootStage>; 8]> = Mutex::new([None; 8]);
pub static BOOT_STAGE_COUNT: Mutex<usize> = Mutex::new(0);

// Assumed 2.0 GHz baseline CPU frequency for cycle-to-ms conversion (2,000,000 cycles/ms)
const CYCLES_PER_MS: u64 = 2_000_000;

#[inline(always)]
pub fn read_tsc() -> u64 {
    unsafe {
        core::arch::x86_64::_rdtsc()
    }
}

pub fn init() {
    BOOT_START_TSC.store(read_tsc(), Ordering::SeqCst);
}

pub fn record_stage(name: &'static str, start_tsc: u64) {
    let now = read_tsc();
    let cycles = now.saturating_sub(start_tsc);
    let ms = (cycles / CYCLES_PER_MS).max(1);

    crate::println!("[BOOT PROFILER] {:<38} : {:>3} ms", name, ms);

    let mut count = BOOT_STAGE_COUNT.lock();
    if *count < 8 {
        let mut stages = BOOT_STAGES.lock();
        stages[*count] = Some(BootStage {
            name,
            duration_ms: ms,
        });
        *count += 1;
    }
}

pub fn print_boot_summary() {
    let total = get_total_boot_time_ms();
    crate::println!("==================================================");
    crate::println!("        AeroOS Boot Benchmarking Summary          ");
    crate::println!("==================================================");
    for opt in get_stages().iter() {
        if let Some(st) = opt {
            crate::println!("  * {:<36} : {:>3} ms", st.name, st.duration_ms);
        }
    }
    crate::println!("--------------------------------------------------");
    crate::println!("  Total Boot Duration                     : {:>3} ms", total);
    crate::println!("==================================================");
}

pub fn get_total_boot_time_ms() -> u64 {
    let now = read_tsc();
    let start = BOOT_START_TSC.load(Ordering::SeqCst);
    (now.saturating_sub(start) / CYCLES_PER_MS).max(1)
}

pub fn get_stages() -> [Option<BootStage>; 8] {
    *BOOT_STAGES.lock()
}
