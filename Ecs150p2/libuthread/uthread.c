#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"
queue_t allReadyTcbs;
// the current tcb that is executing
static struct uthread_tcb* current;
// states of our tcb
enum state {
	running,
	ready,
	blocked,
	zombie,
};
// by default should be 0, because it is global
static int tidAssignment;
struct uthread_tcb {
	enum state tcbState;
	int tid;
	uthread_ctx_t context;
	void* stackLocation;
	uthread_func_t function;
	void* functionArgs;
	int retVal;
	/* TODO Phase 2 */
};

struct uthread_tcb *uthread_current(void)
{
	return current;
	/* TODO Phase 2/3 */
}

void uthread_yield(void)
{
	// pop out the tcb on top
	struct uthread_tcb* nextTcb = calloc(1, sizeof(struct uthread_tcb));
	queue_dequeue(allReadyTcbs, (void**)(&nextTcb));
	// push the current running tcb to the very bottom of the ready to run queue
	struct uthread_tcb* currentRunning = uthread_current();
	currentRunning->tcbState = ready;
	queue_enqueue(allReadyTcbs, currentRunning);
	// save a temp for the previous running tcb
	struct uthread_tcb* previousRunningTcb = currentRunning;
	// change the nextTcb to current and set its state to running
	current = nextTcb;
	nextTcb->tcbState = running;
	uthread_ctx_switch(&(previousRunningTcb->context), &(nextTcb->context));
}



void uthread_exit(void)
{
	/* TODO Phase 2 */
	// the current one that is running
	struct uthread_tcb* running_Tcb = calloc(1, sizeof(struct uthread_tcb));
	running_Tcb = current;
	running_Tcb -> tcbState = zombie;

	uthread_ctx_destroy_stack(current->stackLocation);
	free(running_Tcb);



}

int uthread_create(uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	struct uthread_tcb* createTcb = calloc(1, sizeof(struct uthread_tcb));
	createTcb->tid = tidAssignment;
	tidAssignment += 1;
	createTcb->function = func;
	createTcb->functionArgs = arg;
	createTcb->stackLocation = uthread_ctx_alloc_stack();
	uthread_ctx_init(&(createTcb->context), createTcb->stackLocation, func, arg);
	createTcb->tcbState = ready;
	queue_enqueue(allReadyTcbs, &createTcb);
	return createTcb->tid;

}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	/* TODO Phase 2 */
	// store the current execution process
	allReadyTcbs = queue_create();
	struct uthread_tcb* orig = calloc(1,sizeof(struct uthread_tcb));
	orig->tid = tidAssignment;
	ucontext_t origContext;
	orig->context = origContext;
	tidAssignment++;
	current = orig;
	uthread_create(func,arg);
	
	


	// run the given process
	while(1){
		
		// yield will run the next available thread
		
		uthread_yield();
		// need to rid of the thread that has been finished
		// only orig context remains in the queue
		if(queue_length(allReadyTcbs) == 1){
			return 0;
		}
	}

}

void uthread_block(void)
{
	/* TODO Phase 3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 3 */
}
