#ifndef _memory_manager_H_
#define _memory_manager_H_

// super simple memory manager that basically just keeps track of the
// current pool and makes new and delete available to the rest
// of the code 
//
// each process is responsible for creating its own pool and setting
// the current_pool variable to point to it

#include "vm_pool.H"

typedef unsigned long size_t;

class MemoryManager {
	static VMPool * current_pool;
public:
	static void load(VMPool * pool); 
	static VMPool * pool();
};

//replace the operator "new"
void * operator new (size_t size);

//replace the operator "new[]"
void * operator new[] (size_t size);

//replace the operator "delete"
void operator delete (void * p, size_t s);

//replace the operator "delete[]"
void operator delete[] (void * p);

#endif // _memory_manager_H_
