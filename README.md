# Contents of this report
1. Queue
2. Thread
3. Semaphore
4. Preemtion
5. Testing
## User thread library
1. Uthread class
* first we need an enum that represents the current state
* then we need the context object, which represents the snapshot of our progress
* we need a stack location, which is used for initializing the thread
* we need the function, which represents what my current tcb is handeling
* we need the argument of the function, we use these for the function
* tid and tcb state are not used but I use them for debugging
* these are still good to have since they represent the state of the tcb
2. Current function
* the current function is used by the semaphores
* since they don't have direct access to a tcb, but need the current context
* this is specifically used in the down function in semaphore
3. Yield function
* puts the current running context to the end of the ready queue
* pop the front of the ready queue to become the next running
* switch the context between the currently running and the next in line
4. Exit
* This is called when a tcb finishes executing
* It is then put into a **zombie queue** (global)
* Once the entire thread runner ends, the zombie queue items are freed
5. Create thread
* To create a thread, I need the function and the arguments
* So this is for creating a tcb
* A tcb is created with func, arg as parameters to the function
* In addition to that, a new stack location is allocated
* That stack location is used as the context for this tcb that is being created
6. Run
* First of all, I need an empty context, this represents the mother thread
* Once all threads finish running, mother thread runs, then terminates
* When the mother thread is encountered and there are tcbs avaialbe, it yields
* It first creates an empty tcb
* Then it creates a tcb that is the first tcb to run
* **any thread that yields to the mother tcb will result in mother tcb yielding**
* No more ready to run threads in the global ready queue means we are finished
## Semaphores
1. The semaphore object
* It has a blocked queue, which represents the threads waiting in line
* It has a count, which represents how many slots are available
* It has a lock, which represents whether or not the semaphore is locked
* If the semaphore is locked, the value of the lock is 1
2. Spinlock
* This is exactly the same as the spinlock covered in class
* The reason for this to exist is because of part 4, preemption
* Since the block and unblock are critical sections, they require locks
* The locking process involes a loop for setting the lock to 1
* The unlocking sets the lock to 0
3. Creating a semaphore
* The sem create function takes a count as its input
* We will create an empty queue, for the blocked items in this sempahore
* Then there is a count
* At last the lock is set to 0, since no critical section is currently running
4. Destroying a semaphore
* free the object
5. Semaphore down
* If the count is 0, which means that the thread that calls down needs to wait
* The semaphore pushing the current thread to its queue
* Block is then called
* The part after block will only be ran when up is called by another thread
* Count is then decreased
* The tricky part here involves the spinlock function and when it should be called
* We first lock the function fromt he start, until when block is called
* This is because if we don't unlock, then the semaphore will be permanently locked
* The enqueue into calling the block function is critical
* The next part is after the block is called
* Decreasing the count is also a critical section
* This is because decreasing when it comes to machine language is multiple steps
* This means that I need to unlock it after the decreasing has been done
* So in the case when the semaphore is blocked, lock and unlock is called twice
* When the semaphore is not blocked, lock and unlock is called once
* Every lock has to match with an unlock
6. Semaphore up
* We just dequeue one item from our queue, then plug it into the unblock function
* The entire portion is critical and no yields are called
* This means that we only need 1 lock from start to finish
7. Block
* Block is similar to yield but a little different
* Since we don't want the "downed" thread to run again, we push it into semaphore
* The downed thread is not going to be pushed back into the ready to run queue
* The context is then switched from the current one to the next available
* The current context is saved into the semaphore queue
8. Unblock
* The parameter represents the head of a semaphore queue
* This thread is set to ready state and queued to the ready-to-run queue
## Preemption
  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
1. Signal handeler
* This is basically a timer that sets a system timer
* The system timer will raise a signal when the time has reached
* The current context is yielded to the next because it has been running too long
2. Disable
* Set the signal blocker to the sigset object
* This means that when sigvtalrm is called, nothing happens
3. Enable
* Set the signal unblock to the sigset object
* This means that when the signal is not blocked
4. Start
* If preemption is set, we set the signal action to the signal handeler
* So when the signal happens, it automaticaly yields the current process to next
5. Stop
* The alarm is set to null
6. Usage in the thread library
* Both start and stop are ran inside of the run function
* Preemption starts after the initial set up has been done
* The initial set up are: making mother thread, queuing the first thread to ready
* This is because yield cannot be called when there are no threads even made
* Preemption ends when there are no more items in ready to run
* This means that the thread runner has finished
