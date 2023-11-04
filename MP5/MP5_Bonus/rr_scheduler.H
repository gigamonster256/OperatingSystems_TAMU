#ifndef _RR_SCHEDULER_H_
#define _RR_SCHEDULER_H_

#include "scheduler.h"
#include "eoq_timer.h"

class EOQTimer;

class RRScheduler : public Scheduler {
private:
	EOQTimer *quantum_timer;
public:
	RRScheduler();
	virtual ~RRScheduler();

	virtual void yield();

	virtual void resume(Thread *thread);

	virtual void add(Process *process);

	virtual void terminate(Process *process);

	void handle_timer_interrupt(REGS *r, int timer_id);
};

#endif
