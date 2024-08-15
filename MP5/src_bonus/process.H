#ifndef _process_H_                   // include file only once
#define _process_H_


#include "thread.H"
#include "threads_low.H"
#include "page_table.H"

class Process {
private:
	unsigned long pid;
	PageTable* page_table;
	Thread *threads_head;
public:
	Process(Thread_Function first_thread, unsigned long stack_size, PageTable *page_table);
};

#endif // _process_H_
