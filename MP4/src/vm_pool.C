/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(virtual_addr  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
	// must be in virtual memory mode
    assert(read_cr0() & INIT_VM_MASK);
	// base address and size must be page aligned
	assert(_base_address % PAGE_SIZE == 0);
	assert(_size % PAGE_SIZE == 0);
	// normalize size to number of pages
	_size = _size / PAGE_SIZE;
	// frame pool and page table must be valid
	assert(_frame_pool != NULL);
	assert(_page_table != NULL);
	// must have at least 2 pages for management
	assert(MANAGEMENT_PAGES >= 2);
	// must have more pages than management overhead
	assert(_size > MANAGEMENT_PAGES);

	// save parameters
	base_address = _base_address;
	size = _size;
	frame_pool = _frame_pool;
	page_table = _page_table;
	next = NULL;

	// register this VM pool with the page table
	page_table->register_pool(this);

	// now we are registered, we can just make memory references
	// and the page table will get us a frame.
	// note that this only works because we are in virtual memory mode
	// as well as the fact that we always return true for is_legitimate
	// for the management pages

	// free region entries go in the first half of the management pages
	free_regions = (RegionEntry*)_base_address;
	// allocated region entries go in the second half of the management pages
	allocated_regions = (RegionEntry*)(_base_address + (MANAGEMENT_PAGES / 2) * PAGE_SIZE);

	// zero all region entries
	for (unsigned long i = 1; i < MANAGEMENT_PAGES * PAGE_SIZE / sizeof(RegionEntry); i++) {
		free_regions[i] = RegionEntry();
	}

	// set up the first region entry (skip the first few pages since we use them)
	free_regions[0].start_address = base_address + MANAGEMENT_PAGES * PAGE_SIZE;
	free_regions[0].size = size - MANAGEMENT_PAGES;
}

VMPool::~VMPool(){
	// TODO: free all allocated regions
	// TODO free management pages
	assert(false);
}

unsigned long VMPool::allocate(unsigned long _size) {
	// must be in virtual memory mode
	assert(read_cr0() & INIT_VM_MASK);
	// size must be non-zero
	assert(_size > 0);
	// normalize size to number of pages
	_size = (_size + PAGE_SIZE - 1) / PAGE_SIZE;
	// size must be less than or equal to the size of the pool
	assert(_size <= size - MANAGEMENT_PAGES);
	
	// find a free region that is big enough
	// we should probably iterate from the end of the free region to decrease fragmentation
	// but this is easier to code
	
	// loop over the free region entries
	for (unsigned long i = 0; i < (MANAGEMENT_PAGES / 2) * PAGE_SIZE / sizeof(RegionEntry); i++) {
		// check if the entry is big enough
		if (free_regions[i].size >= _size) {
			// loop over the allocated region entries and find an empty one
			// could look for adjacent free regions and merge them to decrease fragmentation
			// but that makes things more complex
			for (unsigned long j = 0; j < ((MANAGEMENT_PAGES + 1) / 2) * PAGE_SIZE / sizeof(RegionEntry); j++) {
				if (allocated_regions[j].size == 0) {
					// found an empty entry
					// set the start address and size
					allocated_regions[j].start_address = free_regions[i].start_address;
					allocated_regions[j].size = _size;
					// update the free region entry
					free_regions[i].start_address += _size * PAGE_SIZE;
					free_regions[i].size -= _size;
					// return the start address
					return allocated_regions[j].start_address;
				}
			}
			// if we get here then all allocated region entries are used due to fragmentation
			assert(false);
		}
	}
	// if we get here, we couldn't find a free region large enough due to 
	// fragmentation or too small of available memory
	assert(false);
	return 0; // make compiler happy
}

void VMPool::release(virtual_addr _start_address) {
	// must be in virtual memory mode
	assert(read_cr0() & INIT_VM_MASK);
	// address must be page aligned (it should be since we handed it out)
	assert(_start_address % PAGE_SIZE == 0);
	// address cannot be within the management pages
	assert(_start_address >= base_address + MANAGEMENT_PAGES * PAGE_SIZE);
	// address must be owned by us and currently allocated
	assert(is_legitimate(_start_address));
	
	// find the allocated region entry
	for (unsigned long i = 0; i < ((MANAGEMENT_PAGES + 1) / 2) * PAGE_SIZE / sizeof(RegionEntry); i++) {
		// found the entry
		if (allocated_regions[i].start_address == _start_address) {
			// find an open free region entry
			for (unsigned long j = 0; j < (MANAGEMENT_PAGES / 2) * PAGE_SIZE / sizeof(RegionEntry); j++) {
				if (free_regions[j].size == 0) {
					// found an empty entry
					// set the start address and size
					free_regions[j].start_address = allocated_regions[i].start_address;
					free_regions[j].size = allocated_regions[i].size;
					// clear the allocated region entry
					allocated_regions[i] = RegionEntry();
					// now call into the frame pool to release the frames
					// that could have been allocated for this region
					page_table->free_pages(_start_address, free_regions[j].size);
					// return
					return;
				}
			}
			// if we get here then all free region entries are used due to fragmentation
			assert(false);
		}
	}
	// will get here if _start_address is inside an allocated region but not the start
	assert(false);
}

bool VMPool::is_legitimate(virtual_addr _address) {
	// make sure address is within our pool
	if (_address < base_address) return false;
	if (_address >= base_address + size * PAGE_SIZE) return false;
	// first few pages are always valid (for ourselves)
	if (_address < base_address + MANAGEMENT_PAGES * PAGE_SIZE) return true;
	// anything else we need to make sure it has been allocated
	// loop over the allocated region entries
	for (unsigned long i = 0; i < ((MANAGEMENT_PAGES + 1) / 2) * PAGE_SIZE / sizeof(RegionEntry); i++) {
		// check if address is within this region
		if (allocated_regions[i].start_address <= _address && _address < allocated_regions[i].start_address + allocated_regions[i].size * PAGE_SIZE) {
			return true;
		}
	}
	return false;
}

