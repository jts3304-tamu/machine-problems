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
    // set paramaters of the system before building it
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

/**
 * @ref http://www.osdever.net/tutorials/view/implementing-basic-paging
 */
PageTable::PageTable() {
    // set up entries in the page directory and the page table
    unsigned long* page_directory = (unsigned long*)0x9C000;
    unsigned long* page_table = (unsigned long*)0x9D000;

    unsigned long address = 0;
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = address | 3;
        address = address + 4096;
    }

    page_directory[0] = ((unsigned long)page_table) | 3;

    for (int i = 0; i < 1024; i++) {
        page_table[i] = 0 | 2;
    }

    this->page_directory = page_directory;
    this->current_page_table = this;
    Console::puts("Constructed Page Table object\n");
}

void PageTable::load() {
    // load into memory to be used
    // (get from link)
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging() {
    // set the paging_enabled flag (a bit in the CR0 register)
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS* _r) {
    unsigned long faulting_address = read_cr2();
    unsigned int page_directory_index = (faulting_address >> 22) & 0x3FF;
    unsigned int page_table_index = (faulting_address >> 12) & 0x3FF;

    // Check if the page directory entry is present
    if (!(current_page_table->page_directory[page_directory_index] & 1)) {
        // Allocate a new page table for this directory entry if not present
        PageTable* new_page_table;
        memset(new_page_table, 0, 4096);
        unsigned long new_page_table_addr = reinterpret_cast<unsigned long>(new_page_table);
        current_page_table->page_directory[page_directory_index] = new_page_table_addr | 3;  // Mark as present and writable
    }

    unsigned long* curr_page_table = reinterpret_cast<unsigned long*>(current_page_table->page_directory[page_directory_index] & ~0xFFF);

    // Check if the page table entry is present
    if (!(curr_page_table[page_table_index] & 1)) {
        // Allocate a new page if not present
        char* new_page[4096];
        memset(new_page, 0, 4096);
        unsigned long new_page_addr = reinterpret_cast<unsigned long>(new_page);
        curr_page_table[page_table_index] = new_page_addr | 3;  // Mark as present and writable
    } else {
        // Handle other kinds of page faults, e.g., permissions
        // For simplicity, just halt the system or handle the error appropriately
        assert(false);
    }
}
