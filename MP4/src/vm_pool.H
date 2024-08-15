/*
    File: vm_pool.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/26/2010
    Update: 21/10/13

    Description: Management of the Virtual Memory Pool


*/

#ifndef _VM_POOL_H_                   // include file only once
#define _VM_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "utils.H"
#include "cont_frame_pool.H"
#include "page_table.H"
#include "paging_low.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* Forward declaration of class PageTable */
/* We need this to break a circular include sequence. */
class PageTable;

/*--------------------------------------------------------------------------*/
/* V M  P o o l  */
/*--------------------------------------------------------------------------*/

class VMPool { /* Virtual Memory Pool */
private:
	typedef PageTable::virtual_addr virtual_addr;

	static const unsigned long PAGE_SIZE = PageTable::PAGE_SIZE;
	static const unsigned long MANAGEMENT_PAGES = 2;
	// must have at least 2 pages for management
	static const unsigned long INIT_VM_MASK = PageTable::INIT_VM_MASK;

	typedef struct _regionEntry {
		unsigned long start_address; /* start virtual address of region */
		unsigned long size; /* size in pages */
		_regionEntry() : start_address(0), size(0) {}
	} RegionEntry;

	virtual_addr base_address; /* logical start address of pool */
	unsigned long size; /* size of pool in pages */
	PageTable* page_table; /* page table that maps logical to physical addresses */
	RegionEntry* free_regions; /* array of free regions */
	RegionEntry* allocated_regions; /* array of allocated regions */

public:
   VMPool* next; /* next virtual memory pool 
					(used by page table to check for validity of a faulting address)*/
   ContFramePool* frame_pool; /* frame pool that provides physical memory */
   VMPool(virtual_addr  _base_address,
          unsigned long  _size,
          ContFramePool *_frame_pool,
          PageTable     *_page_table);
   /* Initializes the data structures needed for the management of this
    * virtual-memory pool.
    * _base_address is the logical start address of the pool.
    * _size is the size of the pool in bytes.
    * _frame_pool points to the frame pool that provides the virtual
    * memory pool with physical memory frames.
    * _page_table points to the page table that maps the logical memory
    * references to physical addresses. */

   // destructor
   ~VMPool();

   unsigned long allocate(unsigned long _size);
   /* Allocates a region of _size bytes of memory from the virtual
    * memory pool. If successful, returns the virtual address of the
    * start of the allocated region of memory. If fails, returns 0. */

   void release(virtual_addr _start_address);
   /* Releases a region of previously allocated memory. The region
    * is identified by its start address, which was returned when the
    * region was allocated. */

   bool is_legitimate(virtual_addr _address);
   /* Returns the frame pool to allocate from  if the address is not valid. An address is not valid
    * if it is not part of a region that is currently allocated. */
 };

#endif
