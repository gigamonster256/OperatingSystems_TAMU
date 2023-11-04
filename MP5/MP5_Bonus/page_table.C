#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"
#include "virtual_address.H"
#include "vm_pool.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
unsigned long PageTable::shared_size = 0;
unsigned long PageTable::shared_page_table_frame = 0;
bool PageTable::initialized = false;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            const unsigned long _shared_size)
{
	assert(!initialized);
	assert((read_cr0() & INIT_VM_MASK) == 0);
	assert(_kernel_mem_pool != NULL);
	assert(_shared_size > 0);
	shared_size = _shared_size;

	// get number of PTEs needed for shared memory (round up)
	unsigned long shared_pte_no = (shared_size + PAGE_SIZE - 1) / PAGE_SIZE;
	// and number of page tables needed to hold them (round up)
	unsigned long shared_page_table_no = (shared_pte_no + ENTRIES_PER_PAGE - 1) / ENTRIES_PER_PAGE;
	// get that amount of frames (1 frame per page table)
	unsigned long shared_page_table_frame = _kernel_mem_pool->get_frames(shared_page_table_no);
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

PageTable::PageTable(ContFramePool * frame_pool)
{
	assert(initialized);
	assert(frame_pool != NULL);
	vm_pool_head = NULL;
	// get a frame for the page directory
	unsigned long page_directory_frame = frame_pool->get_frames(1);
	
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
	page_directory[ENTRIES_PER_PAGE - 1].present = 1;
	page_directory[ENTRIES_PER_PAGE - 1].page_frame = page_directory_frame;
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
   write_cr0(read_cr0() | INIT_VM_MASK);
}

PageTable::PDE & PageTable::get_PDE(const VirtualAddress va)
{
	assert(read_cr0() & INIT_VM_MASK);
	unsigned long pd_index = va.address() >> 22;
	unsigned long PD_address = 0x3ff << 22 | 0x3ff << 12;
	return ((PDE *)PD_address)[pd_index];
}

PageTable::PTE * PageTable::get_PT(const VirtualAddress va)
{
	assert(read_cr0() & INIT_VM_MASK);
	unsigned long pd_index = va.address() >> 22;
	unsigned long PT_address = 0x3ff << 22 | pd_index << 12;
	return (PTE *)PT_address;
}

PageTable::PTE & PageTable::get_PTE(const VirtualAddress va)
{
	assert(read_cr0() & INIT_VM_MASK);
	PTE * PT = get_PT(va);
	unsigned long pt_index = (va.address() >> 12) & 0x3ff;
	return PT[pt_index];
}

void PageTable::handle_fault(REGS * _r)
{
	// get VA that faulted
	VirtualAddress fault_address = faulting_address();
	// make sure VA is allocated by a vm pool
	VMPool * vm_pool = current_page_table->vm_pool_head;
	while (vm_pool != NULL)
	{
		if (vm_pool->is_legitimate(fault_address))
			break;
		vm_pool = vm_pool->next;
	}
	if (vm_pool == NULL)
	{
		// TODO: throw error (segfault?)
		Console::puts("Page fault at ");
		Console::putva(fault_address);
		Console::puts("\n");
		Console::puts("VA was not allocated by a VM pool registered to this PageTable\n");
		assert(false);
	}
	// get reference to PDE that faulted
	PDE & pde = get_PDE(fault_address);
	// if not present, allocate a page table
	if (!pde.present)
	{
		// get a frame for the page table from the backing vm pool
		unsigned long page_table_frame = vm_pool->frame_pool->get_frames(1);

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

	// get a frame for the page from the backing vm pool
	unsigned long page_frame = vm_pool->frame_pool->get_frames(1);

	// set up the PTE
	pte.present = 1;
	pte.page_frame = page_frame;
}

void PageTable::register_pool(VMPool * _vm_pool)
{
	// should probably check to make sure VM pools don't overlap
	if (vm_pool_head == NULL)
		vm_pool_head = _vm_pool;
	else {
		_vm_pool->next = vm_pool_head;
		vm_pool_head = _vm_pool;
	}
}

void PageTable::free_pages(VirtualAddress va, unsigned long size)
{
	for (unsigned long i = 0; i < size; i += PAGE_SIZE)
	{
		PTE & pte = get_PTE(va.offset(i));
		if (!pte.present)
			continue;
		// free the frame
		ContFramePool::release_frames(pte.page_frame);
		// clear the PTE
		pte = PTE();
	}
	// trigger flush of tlb by reloading cr3
	load();
}
