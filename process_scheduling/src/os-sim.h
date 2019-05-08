/*
 * os-sim.h
 * Multithreaded OS Simulation for ECE 3056
 *
 * The simulator library.
 *
 * YOU WILL NOT NEED TO MODIFY THIS FILE
 */

#ifndef __OS_SIM_H__
#define __OS_SIM_H__


/*
 * The process_state_t enum contains the possible states for a process.
 *
 */
typedef enum {
    PROCESS_NEW = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} process_state_t;


/*
 * The Process Control Block
 *
 *   pid  : The Process ID, a unique number identifying the process. (read-only)
 *
 *   name : A string containing the name of the process. (read-only)
 *
 *   time_remaining : An integer to be used by the shortest remaining
 *         time first algorithm. 
 *
 *   state : The current state of the process.  This should be updated by the
 *        student's code in each of the handlers.  See the task_state_t
 *        struct above for possible values.
 *
 *   pc : The "program counter" of the process.  This value is actually used
 *        by the simulator to simulate the process.  Do not touch.
 *
 *   next : An unused pointer to another PCB.  You may use this pointer to
 *        build a linked-list of PCBs.
 */
typedef enum { OP_CPU = 0, OP_IO, OP_TERMINATE } op_type;

typedef struct {
    op_type type;
    unsigned int time;
} op_t;


typedef struct _pcb_t {
    const unsigned int pid;
    const char *name;
    unsigned int time_remaining;
    process_state_t state;
    op_t *pc;
    struct _pcb_t *next;
    int priority;
} pcb_t;


/*
 * start_simulator() runs the OS simulation.  The number of CPUs (1-16) should
 * be passed as the parameter.
 */
extern void start_simulator(unsigned int cpu_count);


/*
 * context_switch() schedules a process on a CPU.  Note that it is
 * non-blocking.  It does not actually simulate the execution of the process;
 * it simply selects the process to simulate next.
 *
 *       cpu_id : the # of the CPU on which to execute the process
 *          pcb : a pointer to the process's PCB
 */
extern void context_switch(unsigned int cpu_id, pcb_t *pcb);


/*
 * force_preempt() preempts a running process before its timeslice expires.
 * It should be used by the SRTF scheduler to preempt lower
 * priority processes so that higher priority processes may execute.
 */
extern void force_preempt(unsigned int cpu_id);


/*
 * mt_safe_usleep() is a thread-safe implementation of the usleep() function.
 * See man usleep(3) for the behavior of this function.
 */
extern void mt_safe_usleep(long usec);


#endif /* __OS_SIM_H__ */

