/*
 * student.c
 * Multithreaded OS Simulation for ECE 3056
 * Spring 2019
 *
 * This file contains the CPU scheduler for the simulation.
 * Name: Cameron Allotey
 * GTID: 903218636
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os-sim.h"


/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);
void enqueue(pcb_t *data);
pcb_t* dequeue(void);
void printq(void);


/*
 * running_processes[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * running_processes[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the running_processes[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  running_processes_mutex has been provided
 * for your use.
 */
static pcb_t **running_processes;
static pthread_mutex_t running_processes_mutex;

static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;
static unsigned int cpu_count;
static int p = 0;

//additional global variables for different scheduling types
unsigned int sched_type;
//linked list implementation of queue
typedef struct queue {
    pcb_t *head;
}queue;
static queue *myQ;

void enqueue(pcb_t *data) {
    pcb_t* temp;
    pthread_mutex_lock(&queue_mutex);
    if (myQ -> head == NULL) {
            //add to empty list
        data -> next = NULL;
        myQ -> head = data;
    } else if (data -> priority < myQ -> head -> priority) {
        data -> next = myQ -> head;
        myQ -> head = data;
    } else {
        temp = myQ -> head;
        while (temp -> next != NULL && temp -> next -> priority < data -> priority) {
            temp = temp -> next;
        }
        data -> next = temp -> next;
        temp -> next = data;
    }
    pthread_cond_broadcast(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

pcb_t* dequeue() {
    pcb_t *ret;
    pthread_mutex_lock(&queue_mutex);
    ret = myQ -> head;
    if (myQ -> head != NULL) {
        myQ -> head = myQ -> head -> next;
    }
    pthread_mutex_unlock(&queue_mutex);
    return ret;
}

void printq() {
    pthread_mutex_lock(&queue_mutex);
    printf("--------------QUEUE------------\n");
    pcb_t *curr = myQ -> head;
    while (curr != NULL) {
        printf("NAME: %s: %d", curr -> name, curr -> priority);
        curr = curr -> next;
    }
    printf("\n");
    pthread_mutex_unlock(&queue_mutex);
}

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which
 *  you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *  The running_processes array (see above) is how you access the currently running process indexed by the cpu id.
 *  See above for full description.
 *  context_switch() is prototyped in os-sim.h. Look there for more information
 *  about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    //printq();
    pcb_t *proc = dequeue();
    if (proc != NULL) {
        proc -> state = PROCESS_RUNNING;
    }
        //trying to figure out what causes invalid pcb to be scheduled by checking ready queue
    pthread_mutex_lock(&running_processes_mutex);
    running_processes[cpu_id] = proc;
    context_switch(cpu_id, proc);
    pthread_mutex_unlock(&running_processes_mutex);
    
}



 // * idle() is your idle process.  It is called by the simulator when the idle
 // * process is scheduled.
 // *
 // * This function should block until a process is added to your ready queue.
 // * It should then call schedule() to select the process to run on the CPU.
 
extern void idle(unsigned int cpu_id)
{
    pthread_mutex_lock(&queue_mutex);
    while (myQ -> head == NULL) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);
    schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
    //mt_safe_usleep(1000000);
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted 
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&running_processes_mutex);
    running_processes[cpu_id] -> state = PROCESS_READY;
    enqueue(running_processes[cpu_id]);
    pthread_mutex_unlock(&running_processes_mutex);
    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&running_processes_mutex);
    if (running_processes[cpu_id] != NULL) {
        running_processes[cpu_id] -> state = PROCESS_WAITING;
    }
    pthread_mutex_unlock(&running_processes_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&running_processes_mutex);
    running_processes[cpu_id] -> state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&running_processes_mutex);
    schedule(cpu_id);
}

/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is static priority, wake_up() may need
 *      to preempt the CPU with the lowest priority process to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with a higher priority than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *  To preempt a process, use force_preempt(). Look in os-sim.h for
 *  its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    if (process -> state == PROCESS_NEW) {
        if (sched_type == 0) {
                process -> priority = p++;
        } else if (sched_type == 2) {
            //so shortest job will have highest priority
            process -> priority = (int) (process -> time_remaining);
        }
    }
    process -> state = PROCESS_READY;
    enqueue(process);
    //USE LOGIC TO DETERMINE IF NEED TO PREEMPT
    pcb_t* lowest = NULL;
    if (sched_type == 1) {
        pthread_mutex_lock(&running_processes_mutex);
        int lowestId = -1;
        for (int i = 0; i < (int) cpu_count; i++) {
            if (running_processes[i] == NULL) {
                lowestId = i;
                break;
            }
            if (lowest == NULL) {
                lowest = running_processes[i];
            } else {
                if (running_processes[i] != NULL && running_processes[i]->priority > lowest->priority) {
                    lowest = running_processes[i];
                    lowestId = i;
                }
            }
        }
        pthread_mutex_unlock(&running_processes_mutex);
        if (lowestId != -1) {
            force_preempt((unsigned int) lowestId);
        }

    }
    pthread_cond_signal(&queue_not_empty);

}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -s command-line parameters.
 */
int main(int argc, char *argv[])
{
    if (argc == 3) {
        //parse arguments
        cpu_count = (unsigned int) atoi(argv[2]);
        myQ = (struct queue *) malloc(sizeof(struct queue));
        myQ -> head = NULL;
        if (argv[1][1] == 'f') {
            sched_type = 0;
        } else if (argv[1][1] == 'p') {
            sched_type = 1;
        } else if (argv[1][1] == 's') {
            sched_type = 2;
        } else {
            return -1;
        }
    } else {
        //invalid arguments
        return -1;
    }

    /* Uncomment this block once you have parsed the command line arguments
        to start scheduling your processes
    Allocate the running_processes[] array and its mutex */
    running_processes = calloc(1, sizeof(pcb_t*) * cpu_count);
    assert(running_processes != NULL);
    pthread_mutex_init(&running_processes_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}



