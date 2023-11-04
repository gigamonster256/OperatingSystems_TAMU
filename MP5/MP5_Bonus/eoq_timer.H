#ifndef _EOQ_TIMER_H_
#define _EOQ_TIMER_H_

#include "simple_timer.H"

class RRScheduler;

class EOQTimer : public SimpleTimer {
private:
	RRScheduler *scheduler;
	int id;
public:
	EOQTimer(RRScheduler *s, int _id, int hz);

	virtual void handle_interrupt(REGS *r) final;
};


#endif // _EOQ_TIMER_H_
