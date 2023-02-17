#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "private.h"

struct semaphore {
	/* TODO Phase 3 */
  queue_t blocked_queue;
  int my_count;
  int lock;
};
ATOMIC int test_and_set(int *mem)
{
    int oldval = *mem;
    *mem = 1;
    return oldval;
}
void spinlock_lock(int *lock)
{
    while (test_and_set(lock) == 1);
}
void spinlock_unlock(int *lock)
{
    *lock = 0;
}
sem_t sem_create(size_t count)
{
	/* TODO Phase 3 */
  blocked_queue = queue_create();
  my_count = count;
  lock = 0;
}

int sem_destroy(sem_t sem)
{
	/* TODO Phase 3 */
  // go through the entire blocked queue and delete everything
}

int sem_down(sem_t sem)
{
	/* TODO Phase 3 */
  spinlock_lock(sem->lock);
  while (sem->count == 0) {
      queue_enqueue(blocked_queue,uthread_current());
      uthread_block();
  }
  sem->count -= 1;
  spinlock_unlock(sem->lock);
}

int sem_up(sem_t sem)
{
	/* TODO Phase 3 */
  spinlock_lock(sem->lock);
  sem->count += 1;
  /* Wake up first in line */
  /* (if any)              */
  // first dequeue
  struct uthread_tcb* temp_tcb = calloc(1,sizeof(struct uthread_tcb));
  queue_dequeue(blocked_queue, (void**)(&temp_tcb));
  uthread_unblock(temp_tcb);
  spinlock_unlock(sem->lock);
}
