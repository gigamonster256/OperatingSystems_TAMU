/*
    File: page_table.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 16/12/07

    Description: Basic Paging.

*/

#ifndef _page_table_H_                   // include file only once
#define _page_table_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "exceptions.H"
#include "cont_frame_pool.H"

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

class VMPool;

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* P A G E - T A B L E  */
/*--------------------------------------------------------------------------*/

class PageTable {

private:

  /* THESE MEMBERS ARE COMMON TO ENTIRE PAGING SUBSYSTEM */
  static PageTable     * current_page_table; /* pointer to currently loaded page table object */
  static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
  static ContFramePool * kernel_mem_pool;    /* Frame pool for the kernel memory */
  static ContFramePool * process_mem_pool;   /* Frame pool for the process memory */
  static unsigned long   shared_size;        /* size of shared address space */
  static unsigned long   shared_page_table_frame; /* frame number of the shared page tables */
  static bool			 initialized;		 /* is the system set up for paging? */
  
  typedef struct _PTE {
	  union {
		  struct{
			unsigned long present : 1;
			unsigned long read_write : 1;
			unsigned long user_supervisor : 1;
			unsigned long reserved : 2;
			unsigned long accessed : 1;
			unsigned long dirty : 1;
			unsigned long reserved2 : 2;
			unsigned long available : 3;
			unsigned long page_frame : 20;
		  };
		  unsigned long raw;
	  };
	  _PTE() : raw(0) {}
  } PTE;
  static_assert(sizeof(PTE) == sizeof(unsigned long), "PTE size is not 4 bytes");

  typedef PTE PDE;

/* DATA FOR CURRENT PAGE TABLE */
  PDE * page_directory;     /* where is page directory located? physical address*/
  VMPool * vm_pool_head;         /* base memory pool for this page table */


public:
  // type definitions
  typedef Machine::physical_addr physical_addr;
  typedef Machine::virtual_addr virtual_addr;

  static const unsigned int PAGE_SIZE        = Machine::PAGE_SIZE; 
  /* in bytes */
  static const unsigned int ENTRIES_PER_PAGE = Machine::PT_ENTRIES_PER_PAGE; 
  /* in entries, duh! */
  static const unsigned int INIT_VM_MASK = 0x80000000;

  static void init_paging(ContFramePool * _kernel_mem_pool,
                          ContFramePool * _process_mem_pool,
                          const unsigned long _shared_size);
  /* Set the global parameters for the paging subsystem. */

  PageTable();
  /* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere! 
     Probably it is best to have it on the stack, as there is no 
     memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before* 
     paging has been enabled.
  */

  void load();
  /* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */

  static void enable_paging();
  /* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
     memory is accessed by addressing physical memory directly. After paging is
     enabled, memory is addressed logically. */

  // helpers to resolve recursive page table entries
  static PDE & get_PDE(const virtual_addr va);
  static PTE * get_PT(const virtual_addr va);
  static PTE & get_PTE(const virtual_addr va);

  static void handle_fault(REGS * _r);
  /* The page fault handler. */

  void register_pool(VMPool * _vm_pool);
  /* Register a virtual memory pool with the page table. */

  void free_pages(const virtual_addr va, const unsigned long size);
  /* If page is valid, release frame and mark page invalid. */

};

#endif

