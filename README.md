# Contents of this report
1. Queue
2. Thread
3. Semaphore
4. Preemtion
5. Testing
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
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
