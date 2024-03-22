#include "assert.H"
#include "console.H"
#include "exceptions.H"
#include "page_table.H"
#include "paging_low.H"

PageTable* PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool* PageTable::kernel_mem_pool = NULL;
ContFramePool* PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool* _kernel_mem_pool,
                            ContFramePool* _process_mem_pool,
                            const unsigned long _shared_size) {
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
}

PageTable::PageTable() {
    // initialize page_directory to a frame from kernel pool
    unsigned long page_dir_frame_no = process_mem_pool->get_frames(shared_size / (Machine::PAGE_SIZE * Machine::PT_ENTRIES_PER_PAGE) + 1);
    page_directory = (unsigned long*)(page_dir_frame_no * Machine::PAGE_SIZE);

    // set up direct mapped space
    unsigned long* shared_table = (unsigned long*)((page_dir_frame_no + 1) * Machine::PAGE_SIZE);
    unsigned long kernel_space_addr = 0;
    for (unsigned int i = 0; i < Machine::PT_ENTRIES_PER_PAGE; i++) {
        shared_table[i] = kernel_space_addr | 3;  // present bit, write bit, user-mode bit
        kernel_space_addr += Machine::PAGE_SIZE;
    }

    page_directory[0] = ((unsigned long)shared_table) | 3;                                 // present bit, write bit, user-mode bit
    page_directory[Machine::PT_ENTRIES_PER_PAGE - 1] = ((unsigned long)shared_table) | 3;  // present bit, write bit, user-mode bit

    // set up process space (not pre-mapped)
    for (unsigned int i = 1; i < Machine::PT_ENTRIES_PER_PAGE; i++) {
        page_directory[i] = 2;  // dont set present bit, set write bit, dont set user-mode bit
    }
}

void PageTable::load() {
    write_cr3((unsigned long)page_directory);
}

void PageTable::enable_paging() {
    write_cr0(read_cr0() | 0x80000000);
}

unsigned long* PageTable::pde_addr(unsigned long _addr) {
    unsigned long shifted = (_addr >> 20) & 0xFFC;

    return (unsigned long*)(0xFFFFF000 | shifted);
}

unsigned long* PageTable::pte_addr(unsigned long _addr) {
    unsigned long shifted = (_addr >> 10) & 0x3FFFFC;

    return (unsigned long*)(0xFFC00000 | shifted);
}

void PageTable::handle_fault(REGS* _r) {
    if (_r->err_code & 1) {
        // protection fault
        assert(false);
    }

    // use faulting address to determine whether it was a directory or table fault (do we need to allocate a new table or a new frame?)
    unsigned long fault_addr = read_cr2();
    Console::puts("page fault at ");
    // char* hex;
    // uint2hexstr(fault_addr, hex);
    // Console::puts(hex);
    // Console::puts("\n");
    unsigned long* pd = (unsigned long*)read_cr3();

    unsigned int dir_index = fault_addr >> 22;              // first 10 bits
    unsigned int table_index = (fault_addr >> 12) & 0x3FF;  // middle 10 bits

    // if page table isn't present, allocate it
    // allocate new frame to table (no need to check if it's already allocated, since we're handling a page fault)

    if (!(pd[dir_index] & 1)) {
        // allocate new frame to table
        unsigned long new_table_frame_no = kernel_mem_pool->get_frames(1);
        pd[dir_index] = (new_table_frame_no * Machine::PAGE_SIZE) | 3;  // present bit, write bit, user-mode bit
    }

    unsigned long* pt = (unsigned long*)(pd[dir_index] & 0xFFFFF000);

    // allocate new frame to process
    unsigned long new_frame_no = process_mem_pool->get_frames(1);
    if (new_frame_no == 0) {
        // no free frames, need to swap
        assert(false);
    }
    pt[table_index] = (new_frame_no * Machine::PAGE_SIZE) | 3;  // present bit, write bit, user-mode bit

    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool* _vm_pool) {
    assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page\n");
}
