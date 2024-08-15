/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
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

extern void operator delete(void * p);
extern void operator delete[](void * p);

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
	next_thread = NULL;
	Console::puts("Constructed Scheduler.\n");
}

Thread * Scheduler::thread_after(Thread * thread) {
	// we must be passed in a thread
	assert(thread != NULL);
	return (Thread*)thread->cargoPointer();
}

void Scheduler::yield() {
	// make sure there is a thread to yield to
	assert(next_thread != NULL);
	// pop the current thread off the queue
	Thread * curr = next_thread;
	next_thread = thread_after(curr);
	// clear the next thread pointer
	curr->setCargo(NULL);
	// start the thread
	Thread::dispatch_to(curr);
}

void Scheduler::resume(Thread * _thread) {
	// resume means a thread would like to be scheduled again
	// so we add it to the end of the queue
	add(_thread);
}

void Scheduler::add(Thread * _thread) {
	// the only threads that should be added to the queue are threads that
	// are not in the queue
	assert(thread_after(_thread) == NULL);
	// if there is no next thread, set the next thread to the new thread
	if (next_thread == NULL) {
		next_thread = _thread;
		return;
	}
	// otherwise, add the thread to the end of the queue
	Thread * curr = next_thread;
	while (thread_after(curr) != NULL) {
		curr = thread_after(curr);
	}
	// cannot schedule thread after itself
	assert(curr != _thread);
	curr->setCargo((char*)_thread);
}

void Scheduler::terminate(Thread * _thread) {
	// deallocate the thread's stack
	delete[] _thread->stackPointer();
	// deallocate the thread
	delete _thread;
	// yeild to the next thread
	yield();
}
