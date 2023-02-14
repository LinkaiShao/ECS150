#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
struct itimerval timer;
sigset_t ss;
void sig_handler(int signum){
	uthread_yield();
	
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = HZ * 100;
	timer.it_value = timer.it_interval;
	// set the timer again
	setitimer(ITIMER_VIRTUAL,&timer,NULL);
}
void preempt_disable(void)
{
	/* TODO Phase 4 */
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &ss, NULL);
}

void preempt_enable(void)
{
	/* TODO Phase 4 */
	sigemptyset(&ss);
	sigaddset(&ss, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &ss, NULL);
}

void preempt_start(bool preempt)
{
	/* TODO Phase 4 */
	if(preempt){
		struct sigaction sa;
		sa.sa_handler = sig_handler;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGVTALRM,&sa,NULL);
		
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = HZ * 100;
		timer.it_value = timer.it_interval;
		// set the timer 
		setitimer(ITIMER_VIRTUAL,&timer,NULL);
	}
}

void preempt_stop(void)
{
	/* TODO Phase 4 */
	// no more signal handeling
	sigaction(SIGVTALRM,NULL,NULL);
}

