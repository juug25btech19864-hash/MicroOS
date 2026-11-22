extern crate alloc;

use alloc::format;
use alloc::string::String;
use core::sync::atomic::Ordering;
use spin::Mutex;
use x86_64::instructions::port::Port;
use crate::arch::idt::SYSTEM_TICKS;

const CMOS_ADDRESS_PORT: u16 = 0x70;
const CMOS_DATA_PORT: u16 = 0x71;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct DateTime {
    pub year: u16,
    pub month: u8,
    pub day: u8,
    pub hour: u8,
    pub minute: u8,
    pub second: u8,
}

impl DateTime {
    pub const fn default() -> Self {
        Self {
            year: 2026,
            month: 11,
            day: 17,
            hour: 22,
            minute: 41,
            second: 0,
        }
    }

    pub fn format_time_24(&self) -> String {
        format!("{:02}:{:02}", self.hour, self.minute)
    }

    pub fn format_time_12(&self) -> String {
        let (h, ampm) = if self.hour == 0 {
            (12, "AM")
        } else if self.hour < 12 {
            (self.hour, "AM")
        } else if self.hour == 12 {
            (12, "PM")
        } else {
            (self.hour - 12, "PM")
        };
        format!("{:02}:{:02} {}", h, self.minute, ampm)
    }

    pub fn format_date(&self) -> String {
        let month_name = match self.month {
            1 => "Jan", 2 => "Feb", 3 => "Mar", 4 => "Apr",
            5 => "May", 6 => "Jun", 7 => "Jul", 8 => "Aug",
            9 => "Sep", 10 => "Oct", 11 => "Nov", 12 => "Dec",
            _ => "Nov",
        };
        format!("{} {:02}", month_name, self.day)
    }

    pub fn format_full(&self) -> String {
        let month_name = match self.month {
            1 => "Jan", 2 => "Feb", 3 => "Mar", 4 => "Apr",
            5 => "May", 6 => "Jun", 7 => "Jul", 8 => "Aug",
            9 => "Sep", 10 => "Oct", 11 => "Nov", 12 => "Dec",
            _ => "Nov",
        };
        format!("{} {:02}, {} {:02}:{:02}:{:02}", month_name, self.day, self.year, self.hour, self.minute, self.second)
    }
}

unsafe fn read_cmos_reg(reg: u8) -> u8 {
    let mut addr_port = Port::<u8>::new(CMOS_ADDRESS_PORT);
    let mut data_port = Port::<u8>::new(CMOS_DATA_PORT);
    addr_port.write(reg & 0x7F);
    data_port.read()
}

fn is_updating() -> bool {
    unsafe {
        let mut addr_port = Port::<u8>::new(CMOS_ADDRESS_PORT);
        let mut data_port = Port::<u8>::new(CMOS_DATA_PORT);
        addr_port.write(0x0A);
        (data_port.read() & 0x80) != 0
    }
}

fn bcd_to_bin(val: u8) -> u8 {
    ((val >> 4) * 10) + (val & 0x0F)
}

pub fn read_hardware_rtc() -> DateTime {
    // Wait until update-in-progress is clear (with spinloop timeout)
    let mut spins = 0;
    while is_updating() && spins < 1000 {
        core::hint::spin_loop();
        spins += 1;
    }

    let mut sec = unsafe { read_cmos_reg(0x00) };
    let mut min = unsafe { read_cmos_reg(0x02) };
    let mut hr = unsafe { read_cmos_reg(0x04) };
    let mut day = unsafe { read_cmos_reg(0x07) };
    let mut mo = unsafe { read_cmos_reg(0x08) };
    let mut yr = unsafe { read_cmos_reg(0x09) };
    let mut century = unsafe { read_cmos_reg(0x32) };

    // Consistent read pass
    for _ in 0..4 {
        if is_updating() {
            continue;
        }
        let s2 = unsafe { read_cmos_reg(0x00) };
        let m2 = unsafe { read_cmos_reg(0x02) };
        let h2 = unsafe { read_cmos_reg(0x04) };
        let d2 = unsafe { read_cmos_reg(0x07) };
        let mo2 = unsafe { read_cmos_reg(0x08) };
        let y2 = unsafe { read_cmos_reg(0x09) };
        let c2 = unsafe { read_cmos_reg(0x32) };

        if sec == s2 && min == m2 && hr == h2 && day == d2 && mo == mo2 && yr == y2 {
            century = c2;
            break;
        }
        sec = s2;
        min = m2;
        hr = h2;
        day = d2;
        mo = mo2;
        yr = y2;
        century = c2;
    }

    let reg_b = unsafe { read_cmos_reg(0x0B) };
    let is_binary = (reg_b & 0x04) != 0;
    let is_24h = (reg_b & 0x02) != 0;

    if !is_binary {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        let pm_bit = hr & 0x80;
        hr = ((hr & 0x70) >> 4) * 10 + (hr & 0x0F) | pm_bit;
        day = bcd_to_bin(day);
        mo = bcd_to_bin(mo);
        yr = bcd_to_bin(yr);
        if century > 0 && century <= 99 {
            century = bcd_to_bin(century);
        }
    }

    if !is_24h {
        let is_pm = (hr & 0x80) != 0;
        let mut clean = hr & 0x7F;
        if is_pm {
            if clean < 12 {
                clean += 12;
            }
        } else if clean == 12 {
            clean = 0;
        }
        hr = clean;
    }

    let full_year = if century >= 19 && century <= 21 {
        (century as u16) * 100 + (yr as u16)
    } else {
        2000 + (yr as u16)
    };

    DateTime {
        year: full_year,
        month: mo.clamp(1, 12),
        day: day.clamp(1, 31),
        hour: hr % 24,
        minute: min % 60,
        second: sec % 60,
    }
}

struct RtcCache {
    last_ticks: u64,
    time: DateTime,
}

static RTC_CACHE: Mutex<Option<RtcCache>> = Mutex::new(None);

pub fn get_datetime() -> DateTime {
    let current_ticks = SYSTEM_TICKS.load(Ordering::Relaxed);
    let mut cache_guard = RTC_CACHE.lock();

    if let Some(ref cache) = *cache_guard {
        // Refresh every ~50 ticks (0.5 seconds) to avoid CMOS port stalls
        if current_ticks.saturating_sub(cache.last_ticks) < 50 {
            return cache.time;
        }
    }

    let dt = read_hardware_rtc();
    *cache_guard = Some(RtcCache {
        last_ticks: current_ticks,
        time: dt,
    });
    dt
}
