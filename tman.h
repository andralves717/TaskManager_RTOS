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
/* Kernel includes. */

#include "FreeRTOS.h"
#include "ConfigPerformance.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "semphr.h"


// Define return codes
#define TMAN_SUCCESS                     0
#define TMAN_FAIL                       -1
#define TMAN_FAIL_INVALID_ATTRIBUTE     -2
#define TMAN_FAIL_TASK_ALREADY_CREATED  -3
#define TMAN_FAIL_TASK_NOT_CREATED      -4
#define PRIORITY (tskIDLE_PRIORITY + 31)

#define ARRAY_SIZE                      6

typedef struct task_tman {
    char NAME[16];
    int PERIOD;
    int PHASE;
    int DEADLINE;
    int DEALINE_MISSES;
    char PRECEDENCE[16];
    int NUM_ACTIVATIONS;
    TickType_t LAST_ACTIVATION;
    SemaphoreHandle_t SEMAPHORE;
    int IS_PRECEDENT;
} task_tman;

static task_tman tman_task_list[ARRAY_SIZE];

static int tman_period;

void pvTMAN_Task(void *pvParam);
// Define prototypes (public interface)
int TMAN_Init(int tick_ms);
int TMAN_Close();
int TMAN_TaskAdd(char taskName[], uint32_t priority);
int TMAN_TaskRegisterAttributes(char taskName[], char attribute[], char value[]);
int TMAN_TaskWaitPeriod(char * pvParameters);
int TMAN_TaskStats();

#endif	/* TMAN_H */
