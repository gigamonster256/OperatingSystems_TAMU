#include "assert.H"
#include "process.H"
#include "memory_manager.H"


Process::Process(Thread_Function first_thread, unsigned long stack_size, PageTable *page_table) { 
	// test memory manager	
	char * test = new char[100];
	Thread * first = new Thread(first_thread, test, stack_size);
	delete first;
	delete [] test;
}


