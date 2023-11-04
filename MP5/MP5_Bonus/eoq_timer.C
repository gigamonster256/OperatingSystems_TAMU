#include "eoq_timer.H"
#include "rr_scheduler.H"

EOQTimer::EOQTimer(RRScheduler *s, int _id, int hz) : SimpleTimer(hz) {
	scheduler = s;
	id = _id;
}

void EOQTimer::handle_interrupt(REGS *r) {
	// Call the scheduler's handle_interrupt() method
	// with the regs pointer and the id of the timer
	scheduler->handle_timer_interrupt(r, id);
}
