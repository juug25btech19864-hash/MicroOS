use bootloader_api::info::{MemoryRegionKind, MemoryRegions};
use x86_64::structures::paging::{FrameAllocator, PhysFrame, Size4KiB};
use x86_64::PhysAddr;

pub struct BootInfoFrameAllocator {
    memory_regions: &'static MemoryRegions,
    region_idx: usize,
    current_addr: u64,
}

impl BootInfoFrameAllocator {
    /// # Safety
    /// Caller must ensure that passed memory_regions accurately reflect physical memory.
    pub unsafe fn init(memory_regions: &'static MemoryRegions) -> Self {
        let mut allocator = BootInfoFrameAllocator {
            memory_regions,
            region_idx: 0,
            current_addr: 0,
        };
        allocator.align_to_usable();
        allocator
    }

    fn align_to_usable(&mut self) {
        while self.region_idx < self.memory_regions.len() {
            let region = &self.memory_regions[self.region_idx];
            if region.kind == MemoryRegionKind::Usable {
                let aligned_start = (region.start + 4095) & !4095;
                if aligned_start + 4096 <= region.end {
                    self.current_addr = aligned_start;
                    return;
                }
            }
            self.region_idx += 1;
        }
    }
}

unsafe impl FrameAllocator<Size4KiB> for BootInfoFrameAllocator {
    fn allocate_frame(&mut self) -> Option<PhysFrame> {
        while self.region_idx < self.memory_regions.len() {
            let region = &self.memory_regions[self.region_idx];
            if region.kind == MemoryRegionKind::Usable && self.current_addr + 4096 <= region.end {
                let frame_addr = self.current_addr;
                self.current_addr += 4096;
                return Some(PhysFrame::containing_address(PhysAddr::new(frame_addr)));
            }
            self.region_idx += 1;
            if self.region_idx < self.memory_regions.len() {
                let next_region = &self.memory_regions[self.region_idx];
                self.current_addr = (next_region.start + 4095) & !4095;
            }
        }
        None
    }
}
