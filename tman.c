/* 
 * File:   tman.c
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


#include <xc.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Kernel includes. */

#include "FreeRTOS.h"
#include "task.h"

/* App includes */
#include "../UART/uart.h"
#include "tman.h"


static TickType_t tman_ticks = 0;

static int tman_period;
static int last_index = 0;

void pvTMAN_Task(void *pvParam) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    vTaskDelay(1);
    const TickType_t xFrequency = tman_period;

    for (;;) {
       
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int act = 0;
            if (!tman_task_list[i].PERIOD == 0)
                act = (xLastWakeTime % (tman_task_list[i].PERIOD * tman_period)) - (tman_task_list[i].PHASE * tman_period);
            if (act == 0) {

                TaskHandle_t task_handle = xTaskGetHandle(tman_task_list[i].NAME);
                vTaskResume(task_handle);
            }            
        }
        
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Uncomment to test TMAN_TaskStats()
        if (tman_ticks > 20) {
            PrintStr("Testing TMAN_TaskStats(\"B\") - tman.c line 61\n\r");
            
            int* stats = TMAN_TaskStats("B");

            uint8_t message[80];
            sprintf(message, "Task %s - N. Activations: %d - Deadline Misses: %d\n\r", "B", stats[0], stats[1]);
            PrintStr(message);
            TMAN_Close();
        }
        
        tman_ticks++;
    }
}

/********************************************************************
 * Function: 	TMAN_Init()
 * Precondition: 
 * Input: 		 tick_ms
 * Returns:      TMAN_SUCCESS if Ok.
 * Side Effects:	 
 * Overview:     Initializes Task Manager Framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_Init(int tick_ms) {
    
    
    
    tman_period = tick_ms;
    xTaskCreate(pvTMAN_Task, (const signed char * const) "TMAN", 
                configMINIMAL_STACK_SIZE, NULL, PRIORITY, NULL);
    
    return TMAN_SUCCESS;
    
}

/********************************************************************
 * Function: 	TMAN_Close()
 * Precondition: 
 * Input: 		
 * Returns:      TMAN_SUCCESS if Ok.
 * Side Effects:	 
 * Overview:     Terminates Task Manager Framework.
 *		
 * Note:		 	Ends the scheduler
 * 
 ********************************************************************/

int TMAN_Close(){

    vTaskEndScheduler();
    
    return TMAN_SUCCESS;
}

/********************************************************************
 * Function: 	TMAN_TaskAdd()
 * Precondition: 
 * Input:        taskName 
 * Returns:      TMAN_SUCCESS if Ok.
 * Side Effects:	 
 * Overview:     Add a task to the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskAdd(char taskName[]) {

    strcpy(tman_task_list[last_index++].NAME, taskName);
    printf("Task <%s> adicionada.\n\r", taskName);
    return TMAN_SUCCESS;
}

/********************************************************************
 * Function: 	TMAN_TaskRegisterAttributes()
 * Precondition: 
 * Input: 		 taskName, attribute, value of the attribute
 * Attributes:   PERIOD, PHASE, DEADLINE, PRECEDENCE CONSTRAINTS
 * 
 * Returns:      TMAN_SUCCESS if Ok.
 *               TMAN_FAIL error code in case of failure (see tman.h)
 *               TMAN_FAIL_INVALID_ATTRIBUTE if attribute is not valid.
 *               TMAN_FAIL_TASK_NOT_ADDED if task was not added 
 *                                        to the framework
 * Side Effects:	 
 * Overview:     Register attributes (period, phase, deadline, 
 *               precedence constraints) for a task already added to 
 *               the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskRegisterAttributes(char taskName[], char attribute[], char value[]){
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (strcmp(tman_task_list[i].NAME, taskName) == 0) {
            if (strcmp(attribute, "PERIOD") == 0) {
                tman_task_list[i].PERIOD = atoi(value);
                if (!(tman_task_list[i].DEADLINE > 0))
                    tman_task_list[i].DEADLINE = atoi(value);
            } else if (strcmp(attribute, "PHASE") == 0) {
                tman_task_list[i].PHASE = atoi(value);
            } else if (strcmp(attribute, "DEADLINE") == 0) {
                tman_task_list[i].DEADLINE = atoi(value);
            } else if (strcmp(attribute, "PRECEDENCE") == 0) {
                // Verify if value is actually a task_name that exists, if not return TMAN_FAIL
                
                for (int j = 0; j < ARRAY_SIZE; j++) {
                    if (strcmp(tman_task_list[j].NAME, value) == 0) {
                        strcpy(tman_task_list[i].PRECEDENCE, value);
                        tman_task_list[j].SEMAPHORE = xSemaphoreCreateBinary(); // Create semaphore
                        tman_task_list[j].IS_PRECEDENT = 1;
                        return TMAN_SUCCESS;
                    }
                }
                
                
                return TMAN_FAIL;
            } else {
                return TMAN_FAIL_INVALID_ATTRIBUTE;
            }
            return TMAN_SUCCESS;
        }
    }
        
    return TMAN_FAIL_TASK_NOT_ADDED;

}

/********************************************************************
 * Function: 	TMAN_TaskWaitPeriod()
 * Precondition: 
 * Input: 		 
 * Returns:      
 * Side Effects:	 
 * Overview:     Called by a task to signal the termination of an 
 *               instance and wait for the next activation.
 * 
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskWaitPeriod(char * pvParameters){

    for (int i = 0; i < ARRAY_SIZE; i++) {
        
        if (strcmp(tman_task_list[i].NAME, pvParameters) == 0) {

            if (tman_task_list[i].NUM_ACTIVATIONS > 0) {

                // If it does precedence
                if (tman_task_list[i].IS_PRECEDENT == 1)
                    xSemaphoreGive(tman_task_list[i].SEMAPHORE);
                
                // If fails Deadline
                if (tman_ticks - tman_task_list[i].LAST_ACTIVATION > tman_task_list[i].DEADLINE){
                    
                    tman_task_list[i].DEADLINE_MISSES++;            
                }
            }
            break;
        }
    }
//    
    TaskHandle_t task_handle = xTaskGetHandle(pvParameters);    
    vTaskSuspend(task_handle);

    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (strcmp(tman_task_list[i].NAME, pvParameters) == 0) {
            
            tman_task_list[i].LAST_ACTIVATION = tman_ticks;
            
            // If it has precedence
            if (tman_task_list[i].PRECEDENCE != NULL) {
                // Has to take semaphore of the precedence_constraint task
                for (int j = 0; j < ARRAY_SIZE; j++) {

                    // if we found the task which is the precedence
                    if (strcmp(tman_task_list[i].PRECEDENCE, tman_task_list[j].NAME) == 0) {
                        xSemaphoreTake(tman_task_list[j].SEMAPHORE, portMAX_DELAY);
                        break;
                    }
                }
            }
            
            tman_task_list[i].NUM_ACTIVATIONS++;

            break;
        }
    }
    
}

/********************************************************************
 * Function: 	TMAN_TaskStats()
 * Precondition: 
 * Input: 		char taskName[]
 * Returns:      returns statistical information about a task. 
 * Side Effects:	 
 * Overview:     returns statistical information about a task.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int * TMAN_TaskStats(char taskName[]){
    
    static int ret[2];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if (strcmp(tman_task_list[i].NAME, taskName) == 0) {
            ret[0] = tman_task_list[i].NUM_ACTIVATIONS;
            ret[1] = tman_task_list[i].DEADLINE_MISSES;
            return ret;
        }
    }
    
    return ret;
}

/***************************************End Of File*************************************/