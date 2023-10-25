/*
    File: kernel.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 22/03/27


    This file has the main entry point to the operating system.

    MAIN FILE FOR MACHINE PROBLEM "KERNEL-LEVEL THREAD MANAGEMENT"

    NOTE: REMEMBER THAT AT THE VERY BEGINNING WE DON'T HAVE A MEMORY MANAGER. 
          OBJECT THEREFORE HAVE TO BE ALLOCATED ON THE STACK. 
          THIS LEADS TO SOME RATHER CONVOLUTED CODE, WHICH WOULD BE MUCH 
          SIMPLER OTHERWISE.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- COMMENT/UNCOMMENT THE FOLLOWING LINE TO EXCLUDE/INCLUDE SCHEDULER CODE */

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERNEL_POOL_START_FRAME ((2 MB) / Machine::PAGE_SIZE)
#define KERNEL_POOL_SIZE ((2 MB) / Machine::PAGE_SIZE)
#define PROCESS_POOL_START_FRAME ((4 MB) / Machine::PAGE_SIZE)
#define PROCESS_POOL_SIZE ((28 MB) / Machine::PAGE_SIZE)
/* definition of the kernel and process memory pools */

#define MEM_HOLE_START_FRAME ((15 MB) / Machine::PAGE_SIZE)
#define MEM_HOLE_SIZE ((1 MB) / Machine::PAGE_SIZE)
/* we have a 1 MB hole in physical memory starting at address 15 MB */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"         /* LOW-LEVEL STUFF   */
#include "console.H"
#include "gdt.H"
#include "idt.H"             /* EXCEPTION MGMT.   */
#include "irq.H"
#include "exceptions.H"    
#include "interrupts.H"

#include "simple_timer.H"    /* TIMER MANAGEMENT  */

#include "cont_frame_pool.H"      /* MEMORY MANAGEMENT */
#include "vm_pool.H"
#include "memory_manager.H"

#include "thread.H"          /* THREAD MANAGEMENT */
#include "process.H"         /* PROCESS MANAGEMENT */
#include "rr_scheduler.H"


/* SCHEDULRE and AUXILIARY HAND-OFF FUNCTION FROM CURRENT THREAD TO NEXT */
/*--------------------------------------------------------------------------*/

/* -- A POINTER TO THE SYSTEM SCHEDULER */
Scheduler * SYSTEM_SCHEDULER;


void pass_on_CPU(Thread * _to_thread) {
        /* We use a scheduler. Instead of dispatching to the next thread,
           we pre-empt the current thread by putting it onto the ready
           queue and yielding the CPU. */

        SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
        SYSTEM_SCHEDULER->yield();
}

/*--------------------------------------------------------------------------*/
/* A FEW THREADS (pointer to TCB's and thread functions) */
/*--------------------------------------------------------------------------*/

Thread * thread1;
Thread * thread2;
Thread * thread3;
Thread * thread4;

/* -- THE 4 FUNCTIONS fun1 - fun4 ARE LARGELY IDENTICAL. */

void fun1() {
    Console::puts("Thread: "); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts("\n");
    Console::puts("FUN 1 INVOKED!\n");

#ifdef _TERMINATING_FUNCTIONS_
    for(int j = 0; j < 10; j++) 
#else
    for(int j = 0;; j++) 
#endif
    {	
        Console::puts("FUN 1 IN BURST["); Console::puti(j); Console::puts("]\n");
        for (int i = 0; i < 10; i++) {
            Console::puts("FUN 1: TICK ["); Console::puti(i); Console::puts("]\n");
        }
        pass_on_CPU(thread2);
    }
}


void fun2() {
    Console::puts("Thread: "); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts("\n");
    Console::puts("FUN 2 INVOKED!\n");

#ifdef _TERMINATING_FUNCTIONS_
    for(int j = 0; j < 10; j++) 
#else
    for(int j = 0;; j++) 
#endif  
    {		
        Console::puts("FUN 2 IN BURST["); Console::puti(j); Console::puts("]\n");
        for (int i = 0; i < 10; i++) {
            Console::puts("FUN 2: TICK ["); Console::puti(i); Console::puts("]\n");
        }
        pass_on_CPU(thread3);
    }
}

void fun3() {
    Console::puts("Thread: "); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts("\n");
    Console::puts("FUN 3 INVOKED!\n");

    for(int j = 0;; j++) {
        Console::puts("FUN 3 IN BURST["); Console::puti(j); Console::puts("]\n");
        for (int i = 0; i < 10; i++) {
	    Console::puts("FUN 3: TICK ["); Console::puti(i); Console::puts("]\n");
        }
        pass_on_CPU(thread4);
    }
}

void fun4() {
    Console::puts("Thread: "); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts("\n");
    Console::puts("FUN 4 INVOKED!\n");

    for(int j = 0;; j++) {
        Console::puts("FUN 4 IN BURST["); Console::puti(j); Console::puts("]\n");
        for (int i = 0; i < 10; i++) {
	    Console::puts("FUN 4: TICK ["); Console::puti(i); Console::puts("]\n");
        }
        pass_on_CPU(thread1);
    }
}

ContFramePool * kernel_pool = NULL;

void colonel() {
	Console::puts("Colonel process running!\n");	
	for(;;);
    unsigned long n_info_frames =
      ContFramePool::needed_info_frames(PROCESS_POOL_SIZE);

    unsigned long process_mem_pool_info_frame =
      kernel_pool->get_frames(n_info_frames);

    ContFramePool process_mem_pool(PROCESS_POOL_START_FRAME,
                                   PROCESS_POOL_SIZE,
                                   process_mem_pool_info_frame);

    /* Take care of the hole in the memory. */
    process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);
	/* ---- INSTALL PAGE FAULT HANDLER -- */

    class PageFault_Handler : public ExceptionHandler {
      /* We derive the page fault handler from ExceptionHandler
	 and overload the method handle_exception. */
      public:
      virtual void handle_exception(REGS * _regs) {
        PageTable::handle_fault(_regs);
      }
    } pagefault_handler;

    /* ---- Register the page fault handler for exception no. 14
            with the exception dispatcher. */
	Console::puts("Registering page fault handler...\n");
    ExceptionHandler::register_handler(14, &pagefault_handler);
}

/*--------------------------------------------------------------------------*/
/* MAIN ENTRY INTO THE OS */
/*--------------------------------------------------------------------------*/

int main() {

    GDT::init();
    Console::init();
    IDT::init();
    ExceptionHandler::init_dispatcher();
    IRQ::init();
    InterruptHandler::init_dispatcher();

    /* -- SEND OUTPUT TO TERMINAL -- */ 
    Console::output_redirection(true);

    /* -- INITIALIZE THE TIMER (we use a very simple timer).-- */

    SimpleTimer timer(100000); /* timer ticks every 1s. */
    InterruptHandler::register_handler(0, &timer);


	/* -- INITIALIZE FRAME POOLS -- */

    ContFramePool kernel_mem_pool(KERNEL_POOL_START_FRAME,
                                  KERNEL_POOL_SIZE,
                                  0);
	// make kernel pool global (for colonel)
	kernel_pool = &kernel_mem_pool;

	// set up page fault handler
	class PageFault_Handler : public ExceptionHandler {
      /* We derive the page fault handler from ExceptionHandler
	 and overload the method handle_exception. */
      public:
      virtual void handle_exception(REGS * _regs) {
        PageTable::handle_fault(_regs);
      }
    } pagefault_handler;

    /* ---- Register the page fault handler for exception no. 14
            with the exception dispatcher. */
    ExceptionHandler::register_handler(14, &pagefault_handler);

	// init colonel page table
	Console::puts("Initializing page table...\n");
    PageTable::init_paging(kernel_pool, 4 MB);

	// place colonel page directory in kernel pool
	PageTable colonel_pt(kernel_pool);

	// load colonel page directory
	colonel_pt.load();

	// enable paging
	PageTable::enable_paging();

    Console::puts("Hello World!\n");

	// create a VMPool for colonel heap (located in kernel pool)
	Console::puts("Creating VMPool for colonel...\n");
	VMPool colonel_heap(512 MB, 256 MB, kernel_pool, &colonel_pt);

	// load the VMPool into the MemoryManager (this allows new to work)
	Console::puts("Loading colonel VMPool into MemoryManager...\n");
	MemoryManager::load(&colonel_heap);

	// create the colonel process
	Console::puts("Creating colonel process...\n");
	Process * colonel_process = new Process(colonel, 1024, &colonel_pt);
	
	// constructing scheduler
	Console::puts("Creating Round Robin scheduler...\n");
    SYSTEM_SCHEDULER = new RRScheduler();

	// enable interrupts to that timer can start
	// and create preemptive environment
    Machine::enable_interrupts();
	
	Console::puts("Adding colonel process to scheduler...\n");
	for(;;);
	SYSTEM_SCHEDULER->add(colonel_process);
	Console::puts("Starting scheduler...\n");
	SYSTEM_SCHEDULER->yield();

    assert(false); /* WE SHOULD NEVER REACH THIS POINT. */

    /* -- WE DO THE FOLLOWING TO KEEP THE COMPILER HAPPY. */
    return 1;
}
