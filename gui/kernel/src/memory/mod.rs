pub mod allocator;
pub mod frame_allocator;

use bootloader_api::info::MemoryRegions;
use frame_allocator::BootInfoFrameAllocator;
use x86_64::structures::paging::OffsetPageTable;
use x86_64::structures::paging::PageTable;
use x86_64::VirtAddr;

pub fn init(physical_memory_offset: u64, memory_regions: &'static MemoryRegions) {
    let mut mapper = unsafe { init_offset_page_table(physical_memory_offset) };
    let mut frame_allocator = unsafe { BootInfoFrameAllocator::init(memory_regions) };

    allocator::init_heap(&mut mapper, &mut frame_allocator)
        .expect("Heap initialization failed");
}

/// # Safety
/// Caller must ensure physical_memory_offset is valid and mapped to physical memory 0.
unsafe fn init_offset_page_table(physical_memory_offset: u64) -> OffsetPageTable<'static> {
    use x86_64::registers::control::Cr3;

    let (level_4_table_frame, _) = Cr3::read();
    let phys = level_4_table_frame.start_address();
    let virt = VirtAddr::new(physical_memory_offset + phys.as_u64());
    let page_table_ptr: *mut PageTable = virt.as_mut_ptr();

    OffsetPageTable::new(&mut *page_table_ptr, VirtAddr::new(physical_memory_offset))
}
