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
#include "queue.h"
#include "list.h"

/* App includes */
#include "../UART/uart.h"
#include "tman.h"


static List_t * tman_task_list;
static TickType_t tman_ticks = 0;

static int tman_period;

void pvTMAN_Task(void *pvParam) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = tman_period;

    for (;;) {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        //tman_ticks++;
        printf("[TMAN] tick %d\n\r", tman_ticks++);
        
        ListItem_t * pvTmanTaskListIdx = tman_task_list->xListEnd.pxNext;

        for (int i = 0; i < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx = pvTmanTaskListIdx->pxNext, i++) {
            task_tman * pvItemTmp = (task_tman *) pvTmanTaskListIdx->pvOwner;

            if(pvItemTmp->PERIOD * pvItemTmp->NUM_ACTIVATIONS + pvItemTmp->PHASE < tman_ticks){
                TaskHandle_t task_handle = xTaskGetHandle(pvItemTmp->NAME);
                vTaskResume(task_handle);
            }

        }
    }

}
/********************************************************************
 * Function: 	TMAN_Init()
 * Precondition: 
 * Input: 		 tick_ms
 * Returns:      TMAN_SUCCESS if Ok.
 *               TMAN_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Initializes Task Manager Framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_Init(int tick_ms) {
    
    tman_task_list = (List_t *) pvPortMalloc(sizeof( List_t ));
    vListInitialise(tman_task_list);
    
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
 *               TMAN_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Terminates Task Manager Framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_Close(){

    ListItem_t * pvTmanTaskListIdx = tman_task_list->xListEnd.pxNext;
    
    while(pvTmanTaskListIdx != NULL){
        vPortFree(&pvTmanTaskListIdx->pvOwner);
        pvTmanTaskListIdx = pvTmanTaskListIdx->pxNext;
        vPortFree(&pvTmanTaskListIdx->pxPrevious);
    }
    vPortFree(&pvTmanTaskListIdx);
    vPortFree(&tman_task_list);
    
    
    return TMAN_SUCCESS;
}

/********************************************************************
 * Function: 	TMAN_TaskAdd()
 * Precondition: 
 * Input: 		
 * Returns:      TMAN_SUCCESS if Ok.
 *               TMAN_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Add a task to the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskAdd(char taskName[], uint32_t priority) {
    ListItem_t * pvTmanTaskListIdx = tman_task_list->xListEnd.pxNext;
    
    for(int i = 0; i < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx = pvTmanTaskListIdx->pxNext, i++){
        task_tman * pvItemTmp = (task_tman *) pvTmanTaskListIdx->pvOwner;
        if(strcmp(pvItemTmp->NAME, taskName) == 0){
            return TMAN_FAIL;
        }
    }
    
    ListItem_t * pxItem = (ListItem_t *) pvPortMalloc(sizeof ( ListItem_t ));
    task_tman * pvTaskTmanTmp = (task_tman *) pvPortMalloc(sizeof (task_tman));
    strcpy(pvTaskTmanTmp->NAME, taskName);
    pvTaskTmanTmp->NUM_ACTIVATIONS = 0;
    pxItem->pvOwner = pvTaskTmanTmp;
    vListInitialiseItem(pxItem);
    pvTmanTaskListIdx->xItemValue = priority;
    vListInsert(tman_task_list, pxItem);
    
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
 * Side Effects:	 
 * Overview:     Register attributes (period, phase, deadline, 
 *               precedence constraints) for a task already added to 
 *               the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskRegisterAttributes(char taskName[], char attribute[], char value[]){
    
    ListItem_t * pvTmanTaskListIdx = tman_task_list->xListEnd.pxNext;

    for (int i = 0; i < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx = pvTmanTaskListIdx->pxNext, i++) {
        task_tman * pvItemTmp = (task_tman *) pvTmanTaskListIdx->pvOwner;
        if (strcmp(pvItemTmp->NAME, taskName) == 0 ) {
            if( strcmp(attribute, "PERIOD") == 0 ){
                pvItemTmp->PERIOD = atoi(value);
            } else if (strcmp(attribute, "PHASE") == 0 ) {
                pvItemTmp->PHASE = atoi(value);
            } else if (strcmp(attribute, "DEADLINE") == 0 ) {
                pvItemTmp->DEADLINE = atoi(value);
            } else if (strcmp(attribute, "PRECEDENCE") == 0 ) {
                // Verify if value is actually a task_name that exists, if not return TMAN_FAIL
                ListItem_t * pvTmanTaskListIdx2 = tman_task_list->xListEnd.pxNext;
                for(int j = 0; j < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx2 = pvTmanTaskListIdx2->pxNext, j++){
                    task_tman * precedence_task = (task_tman *) pvTmanTaskListIdx2->pvOwner;
                    if(strcmp(precedence_task->NAME, value) == 0){
                        pvItemTmp->PRECEDENCE = value;
                        precedence_task->SEMAPHORE = xSemaphoreCreateBinary(); // Create semaphore
                        precedence_task->IS_PRECEDENT = 1;
                        return TMAN_SUCCESS;
                    }
                }
                return TMAN_FAIL;
            } else {
                return TMAN_FAIL;
            }
            return TMAN_SUCCESS;
        }
    }
    
    printf("Adicionado na task %s o atributo %s com o valor %d\n\r",taskName, attribute, value);
    
    return TMAN_FAIL;
    
    
}

/********************************************************************
 * Function: 	TMAN_TaskWaitPeriod()
 * Precondition: 
 * Input: 		 tick_ms
 * Returns:      TMAN_SUCCESS if Ok.
 *               TMAN_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Called by a task to signal the termination of an 
 *               instance and wait for the next activation.
 * 
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskWaitPeriod(char * pvParameters){
    vTaskSuspend(NULL);

    ListItem_t * pvTmanTaskListIdx = tman_task_list->xListEnd.pxNext;
    for (int i = 0; i < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx = pvTmanTaskListIdx->pxNext, i++) {
        task_tman * pvItemTmp = (task_tman *) pvTmanTaskListIdx->pvOwner;
        if (strcmp(pvItemTmp->NAME, pvParameters) == 0 ) {
            // If it has precedence
            if(pvItemTmp->PRECEDENCE != NULL){
                // Has to take semaphore of the precedence_constraint task
                ListItem_t * pvTmanTaskListIdx2 = tman_task_list->xListEnd.pxNext;
                for(int j = 0; j < tman_task_list->uxNumberOfItems; pvTmanTaskListIdx2 = pvTmanTaskListIdx2->pxNext, j++){
                    task_tman * precendence_task = (task_tman *) pvTmanTaskListIdx->pvOwner;
                    // if we found the task which is the precedence
                    if(strcmp(pvItemTmp->PRECEDENCE, precedence_task->NAME) == 0){
                        xSemaphoreTake( precedence_task->SEMAPHORE, portMAX_DELAY);
                        break;
                    }
                }
            // If it does precedence
            if(pvItemTmp->IS_PRECEDENT == 1){
                xSemaphoreGive( pvItemTmp->SEMAPHORE);
            }
            pvItemTmp->NUM_ACTIVATIONS++;
            break;
        }
    }
    
}

/********************************************************************
 * Function: 	TMAN_TaskStats()
 * Precondition: 
 * Input: 		
 * Returns:      returns statistical information about a task. 
 * Side Effects:	 
 * Overview:     returns statistical information about a task.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskStats(){
}

/***************************************End Of File*************************************/