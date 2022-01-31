/*
 * <> e <>, Jan/2022
 *
 * FREERTOS demo for ChipKit MAX32 board
 * - Creates two periodic tasks
 * - One toggles Led LD4, other is a long (interfering)task that 
 *      activates LD5 when executing 
 * - When the interfering task has higher priority interference becomes visible
 *      - LD4 does not blink at the right rate
 *
 * Environment:
 * - MPLAB X IDE v5.45
 * - XC32 V2.50
 * - FreeRTOS V202107.00
 *
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "tman.h"


/* App includes */
#include "../UART/uart.h"


#define SYSCLK  80000000L // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz

/* Set the tasks' period (in system ticks) */
#define PERIOD_MS                (    1 / portTICK_RATE_MS ) //
#define PERIOD_2MS               (    2 / portTICK_RATE_MS ) //
#define PERIOD_5MS               (    5 / portTICK_RATE_MS ) //
#define PERIOD_50MS              (   50 / portTICK_RATE_MS ) //
#define PERIOD_500MS             (  500 / portTICK_RATE_MS ) // 
#define PERIOD_1000MS            ( 1000 / portTICK_RATE_MS ) // 
#define PERIOD_3000MS            ( 3000 / portTICK_RATE_MS ) // 

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define PRIORITY_A        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_B        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_C        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_D        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_E        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_F        ( tskIDLE_PRIORITY +  2 )
#define PRIORITY_TASKS    ( tskIDLE_PRIORITY +  2 )

void taskBody( void * pvParameters ) {
    for (;;) {
        // Wait for the next cycle.
        TMAN_TaskWaitPeriod((char *) pvParameters);
        int tcks = xTaskGetTickCount();
//        printf("\x1B[31;4m[%s]\x1B[0m Tick: %d \n\r", ((char *) pvParameters), tcks);
        PrintStr(((char *) pvParameters));
        PrintStr(", ");
        char buffer[10];
        PrintStr(itoa(buffer, tcks, 10));
        PrintStr("\n\r");
//        int i, j,k;
//        for (i = 0; i < 5; i++)
//            for (j = 0; j < 5; j++)
//                k = i+j;
        
    }
}

///*
// * Create the demo tasks then start the scheduler.
// */
int main_tman( void ) {
    
	// Init UART and redirect stdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
     // Disable JTAG interface as it uses a few ADC ports
    DDPCONbits.JTAGEN = 0;


    // Welcome message
    
    printf("\n\n*********************************************\n\r");
    printf("Teste Número 17\n\r");
    /* Create the tasks defined within this file. */
    xTaskCreate(taskBody, (const signed char * const) "Task A",
                configMINIMAL_STACK_SIZE, (void *) "Task A", PRIORITY_A, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task B",
                configMINIMAL_STACK_SIZE, (void *) "Task B", PRIORITY_B, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task C",
                configMINIMAL_STACK_SIZE, (void *) "Task C", PRIORITY_C, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task D",
                configMINIMAL_STACK_SIZE, (void *) "Task D", PRIORITY_D, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task E",
                configMINIMAL_STACK_SIZE, (void *) "Task E", PRIORITY_E, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task F",
                configMINIMAL_STACK_SIZE, (void *) "Task F", PRIORITY_F, NULL);
        
    TMAN_Init(PERIOD_1000MS);
    
    int err;

    
    if (TMAN_TaskAdd("Task A", PRIORITY_A) == TMAN_FAIL_TASK_ALREADY_CREATED){
        printf("Task Already Created\nExiting\n");
        return -1;
    }
        
    if (TMAN_TaskAdd("Task B", PRIORITY_B) == TMAN_FAIL_TASK_ALREADY_CREATED) {
        printf("Task Already Created\nExiting\n");
        return -1;
    }
    
    if (TMAN_TaskAdd("Task C", PRIORITY_C) == TMAN_FAIL_TASK_ALREADY_CREATED) {
        printf("Task Already Created\nExiting\n");
        return -1;
    }

    if (TMAN_TaskAdd("Task D", PRIORITY_D) == TMAN_FAIL_TASK_ALREADY_CREATED) {
        printf("Task Already Created\nExiting\n");
        return -1;
    }
    
    if (TMAN_TaskAdd("Task E", PRIORITY_E) == TMAN_FAIL_TASK_ALREADY_CREATED) {
        printf("Task Already Created\nExiting\n");
        return -1;
    }

    if (TMAN_TaskAdd("Task F", PRIORITY_F) == TMAN_FAIL_TASK_ALREADY_CREATED) {
        printf("Task Already Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task A", "PERIOD", "5");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE){
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if ( err == TMAN_FAIL_TASK_NOT_CREATED){
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task A", "PHASE", "0");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task A", "DEADLINE", "5");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task B", "PERIOD", "2");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task B", "PHASE", "1");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task B", "DEADLINE", "2");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task C", "PERIOD", "3");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task C", "PHASE", "1");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task C", "DEADLINE", "3");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    err = TMAN_TaskRegisterAttributes("Task D", "PERIOD", "4");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task D", "PHASE", "1");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task D", "DEADLINE", "4");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task E", "PERIOD", "5");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task E", "PHASE", "1");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task E", "DEADLINE", "5");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task F", "PERIOD", "6");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task F", "PHASE", "3");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }

    err = TMAN_TaskRegisterAttributes("Task F", "DEADLINE", "6");
    if (err == TMAN_FAIL_INVALID_ATTRIBUTE) {
        printf("Invalid Attribute\nExiting\n");
        return -1;
    } else if (err == TMAN_FAIL_TASK_NOT_CREATED) {
        printf("Task Not Created\nExiting\n");
        return -1;
    }
    
    TMAN_TaskRegisterAttributes("Task A", "PRECEDENCE", "Task B");
    
    
    
    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
    the scheduler. */

    return 0;
}
