#include "memory_manager.H"
#include "vm_pool.H"

// global VMPool for the current process
VMPool * MemoryManager::current_pool = NULL;

// load in a new VMPool (switching processes)
void MemoryManager::load(VMPool * pool) {
	assert(pool != NULL);
	MemoryManager::current_pool = pool;
}

// get the VMPool for allocations
VMPool * MemoryManager::pool() {
	assert(MemoryManager::current_pool != NULL);
	return MemoryManager::current_pool;
}

//replace the operator "new"
void * operator new (size_t size) {
  unsigned long a = MemoryManager::pool()->allocate((unsigned long)size);
  return (void *)a;
}

//replace the operator "new[]"
void * operator new[] (size_t size) {
  unsigned long a = MemoryManager::pool()->allocate((unsigned long)size);
  return (void *)a;
}

//replace the operator "delete"
void operator delete (void * p, size_t s) {
  MemoryManager::pool()->release((unsigned long)p);
}

//replace the operator "delete[]"
void operator delete[] (void * p) {
  MemoryManager::pool()->release((unsigned long)p);
}
