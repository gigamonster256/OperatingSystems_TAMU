#include "rr_scheduler.H"
#include "eoq_timer.H"
#include "console.H"

// constructor
RRScheduler::RRScheduler() {
	Console::puts("RRScheduler::RRScheduler\n");
	// initialize the quantum timer
	quantum_timer = new EOQTimer(this, 0, 1000);
	// register the quantum timer with the interrupt controller
	InterruptHandler::register_handler(0, quantum_timer);
}

// dtor
RRScheduler::~RRScheduler() {
	delete quantum_timer;
}

void RRScheduler::yield() {
	Console::puts("RRScheduler::yield\n");
	assert(false);
}

void RRScheduler::resume(Thread *t) {
	Console::puts("RRScheduler::resume\n");
	assert(false);
}

void RRScheduler::add(Process *p) {
	Console::puts("RRScheduler::add\n");
	assert(false);
}

void RRScheduler::terminate(Process *p) {
	Console::puts("RRScheduler::terminate\n");
	assert(false);
}

void RRScheduler::handle_timer_interrupt(REGS *r, int timer_id) {
	assert(timer_id == 0);
	Console::puts("RRScheduler::handle_timer_interrupt\n");
}
