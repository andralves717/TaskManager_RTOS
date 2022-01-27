/* 
 * File:   tman.h
 * Author: André Alves
 * Author: Eduardo Coelho
 *
 * Created on Jan 27, 2022
 * MPLAB X IDE v5.50 + XC32 v3.01
 *
 * Target: Digilent chipKIT MAx32 board 
 * 
 * Overview:
 *          Set of functions to Manage tasks for FreeRTOS
 
 * 
 * Revisions:
 *      2022-01-27: initial release
 */

#ifndef TMAN_H
#define	TMAN_H

#include <stdint.h>

// Define return codes
#define TMAN_SUCCESS 0
#define TMAN_FAIL -1

struct task_tman {
    char NAME[];
    int PERIOD;
    int PHASE;
    int DEADLINE;
    int PRECEDENCE_CONSTRAINTS;
};

static List_t * tman_task_list;


// Define prototypes (public interface)
int TMAN_Init(int tick_ms);
int TMAN_Close();
int TMAN_TaskAdd(char taskName[]);
int TMAN_TaskRegisterAttributes(char taskName[], int period, int phase, int deadline);
int TMAN_TaskWaitPeriod();
int TMAN_TaskStats();

#endif	/* TMAN_H */
