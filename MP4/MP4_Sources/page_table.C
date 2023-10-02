#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
unsigned long PageTable::shared_page_table_frame = 0;
bool PageTable::initialized = false;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
	assert(!initialized);
	assert((read_cr0() & 0x80000000) == 0);
	assert(_kernel_mem_pool != NULL);
	assert(_process_mem_pool != NULL);
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;

	// get number of PTEs needed for shared memory (round up)
	unsigned long shared_pte_no = (shared_size + PAGE_SIZE - 1) / PAGE_SIZE;
	// and number of page tables needed to hold them (round up)
	unsigned long shared_page_table_no = (shared_pte_no + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;
	// get that amount of frames (1 frame per page table)
	unsigned long shared_page_table_frame = kernel_mem_pool->get_frames(shared_page_table_no);
	// and convert to continuous page tables
	PTE * shared_page_tables = (PTE *)(shared_page_table_frame * PAGE_SIZE);
	
	// init the page table(s)
	for (unsigned long i = 0; i < shared_pte_no; i++)
	{
		shared_page_tables[i] = PTE();
		shared_page_tables[i].present = 1;
		// direct mapped
		shared_page_tables[i].page_frame = i;
	}

	// save the pre-allocated page tables location
	PageTable::shared_page_table_frame = shared_page_table_frame;
	
	// signal we are done
	initialized = true;
}

PageTable::PageTable()
{
	assert(initialized);
	assert((read_cr0() & 0x80000000) == 0);
	// get a frame for the page directory
	unsigned long page_directory_frame = process_mem_pool->get_frames(1);
	
	// and convert to page directory
	page_directory = (PDE *)(page_directory_frame * PAGE_SIZE);
	
	// set up page directory
	for (unsigned long i = 0; i < ENTRIES_PER_PAGE - 1; i++)
	{
		page_directory[i] = PDE();
	}

	// map the shared page tables
	// calculate the number of page tables needed
	unsigned long shared_pte_no = (shared_size + PAGE_SIZE - 1) / PAGE_SIZE;
	unsigned long shared_page_table_no = (shared_pte_no + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;
	// set up the PDEs
	for (unsigned long i = 0; i < shared_page_table_no; i++)
	{
		page_directory[i].present = 1;
		page_directory[i].page_frame = shared_page_table_frame + i;
	}
	
	// set up last entry to point to page directory itself
	page_directory[1023].present = 1;
	page_directory[1023].page_frame = page_directory_frame;
}


void PageTable::load()
{
	// set current page table to us
	current_page_table = this;
	// set CR3 to point to the page directory
	write_cr3((unsigned long)page_directory);
}

void PageTable::enable_paging()
{
   // set CR0 top bit to enable paging
   write_cr0(read_cr0() | 0x80000000);
}

PageTable::PDE & PageTable::get_PDE(const unsigned long virtual_address)
{
	assert(read_cr0() & 0x80000000);
	unsigned long pd_index = virtual_address >> 22;
	unsigned long PD_address = 0x3ff << 22 | 0x3ff << 12;
	return ((PDE *)PD_address)[pd_index];
}

PageTable::PTE * PageTable::get_PT(const unsigned long virtual_address)
{
	assert(read_cr0() & 0x80000000);
	unsigned long pd_index = virtual_address >> 22;
	unsigned long PT_address = 0x3ff << 22 | pd_index << 12;
	return (PTE *)PT_address;
}

PageTable::PTE & PageTable::get_PTE(const unsigned long virtual_address)
{
	assert(read_cr0() & 0x80000000);
	PTE * PT = get_PT(virtual_address);
	unsigned long pt_index = (virtual_address >> 12) & 0x3ff;
	return PT[pt_index];
}

void PageTable::handle_fault(REGS * _r)
{
	unsigned long fault_address = read_cr2();
	// get reference to PDE that faulted
	PDE & pde = get_PDE(fault_address);
	// if not present, allocate a page table
	if (!pde.present)
	{
		// get a frame for the page table
		unsigned long page_table_frame = process_mem_pool->get_frames(1);

		// set up the PDE so the page table is mapped
		pde.present = 1;
		pde.page_frame = page_table_frame;

		// clear the page table
		PTE * page_table = get_PT(fault_address);
		for (unsigned long i = 0; i < ENTRIES_PER_PAGE; i++)
		{
			page_table[i] = PTE();
		}

		// set up last entry to point to page table itself
		page_table[ENTRIES_PER_PAGE - 1].present = 1;
		page_table[ENTRIES_PER_PAGE - 1].page_frame = page_table_frame;
	}
	// get reference to PTE that faulted
	PTE & pte = get_PTE(fault_address);
	assert(!pte.present);

	// get a frame for the page
	unsigned long page_frame = process_mem_pool->get_frames(1);

	// set up the PTE
	pte.present = 1;
	pte.page_frame = page_frame;
}

