/*
 * process.c
 * Multithreaded OS Simulation for ECE 3056
 *
 * This file contains process data for the simulator.
 */

#include "os-sim.h"
#include "process.h"
#include <stdlib.h>

/*
 * Note: The operations must alternate: OP_CPU, OP_IO, OP_CPU, ...
 * In addition, the first and last operations must be OP_CPU.  Otherwise,
 * the simulator will not work.
 */

static op_t pid0_ops[] = {
    { OP_CPU, 4 },
    { OP_IO, 5 },
    { OP_CPU, 2 },
    { OP_IO, 1 },
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 5 },
    { OP_IO, 1 },
    { OP_CPU, 2 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 3 },
    { OP_IO, 2 },
    { OP_CPU, 4 },
    { OP_IO, 7 },
    { OP_CPU, 3 },
    { OP_IO, 2 },
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 4 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 2 },
    { OP_CPU, 4 },
    { OP_TERMINATE, 0 }
};

static op_t pid1_ops[] = {
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 1 },
    { OP_IO, 5 },
    { OP_CPU, 7 },
    { OP_IO, 3 },
    { OP_CPU, 2 },
    { OP_IO, 1 },
    { OP_CPU, 3 },
    { OP_IO, 8 },
    { OP_CPU, 3 },
    { OP_IO, 1 },
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 1 },
    { OP_IO, 5 },
    { OP_CPU, 4 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 2 },
    { OP_CPU, 1 },
    { OP_IO, 6 },
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 2 },
    { OP_CPU, 1 },
    { OP_TERMINATE, 0 }
};

static op_t pid2_ops[] = {
    { OP_CPU, 2 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 6 },
    { OP_CPU, 2 },
    { OP_IO, 7 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 3 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 4 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 6 },
    { OP_CPU, 2 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 2 },
    { OP_IO, 4 },
    { OP_CPU, 4 },
    { OP_TERMINATE, 0 }
};

static op_t pid3_ops[] = {
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 12 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_TERMINATE, 0 }
};

static op_t pid4_ops[] = {
    { OP_CPU, 10 }, 
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 2 },
    { OP_CPU, 7 },
    { OP_IO, 2 },
    { OP_CPU, 11 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 14 },
    { OP_CPU, 11 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 2 },
    { OP_CPU, 11 },
    { OP_TERMINATE, 0 }
};

static op_t pid5_ops[] = {
    { OP_CPU, 10 }, 
    { OP_IO, 1 },
    { OP_CPU, 10 },
    { OP_IO, 3 },
    { OP_CPU, 15 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 10 },
    { OP_IO, 2 },
    { OP_CPU, 16 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 12 },
    { OP_IO, 2 },
    { OP_CPU, 15 },
    { OP_IO, 3 },
    { OP_CPU, 8 },
    { OP_TERMINATE, 0 }
};

static op_t pid6_ops[] = {
    { OP_CPU, 7 }, 
    { OP_IO, 3 },
    { OP_CPU, 11 },
    { OP_IO, 1 },
    { OP_CPU, 15 },
    { OP_IO, 1 },
    { OP_CPU, 11 },
    { OP_IO, 3 },
    { OP_CPU, 10 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 13 },
    { OP_CPU, 11 },
    { OP_IO, 3 },
    { OP_CPU, 19 },
    { OP_IO, 10 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 11 },
    { OP_TERMINATE, 0 }
};

static op_t pid7_ops[] = {
    { OP_CPU, 6 }, 
    { OP_IO, 3 },
    { OP_CPU, 12 },
    { OP_IO, 3 },
    { OP_CPU, 17 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_IO, 4 },
    { OP_CPU, 12 },
    { OP_IO, 3 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_TERMINATE, 0 }
};

pcb_t processes[PROCESS_COUNT] = {
    { 0, "Montpelier", 96, PROCESS_NEW, pid0_ops, NULL, 5 },
    { 1, "Pierre", 80, PROCESS_NEW, pid1_ops, NULL, 1 },
    { 2, "Hartford", 89, PROCESS_NEW, pid2_ops, NULL, 6 },
    { 3, "Lansing", 99, PROCESS_NEW, pid3_ops, NULL, 8 },
    { 4, "Helena", 130, PROCESS_NEW, pid4_ops, NULL, 2 },
    { 5, "Concord",127, PROCESS_NEW, pid5_ops, NULL, 7 },
    { 6, "Trenton", 159, PROCESS_NEW, pid6_ops, NULL, 3 },
    { 7, "Bismark", 109, PROCESS_NEW, pid7_ops, NULL, 4 }
};


