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
#define PERIOD_MS           ( 1 / portTICK_RATE_MS ) // 
#define PERIOD_500MS           ( 500 / portTICK_RATE_MS ) // 
#define PERIOD_1000MS           ( 1000 / portTICK_RATE_MS ) // 
#define PERIOD_3000MS           ( 3000 / portTICK_RATE_MS ) // 

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define PRIORITY_1        ( tskIDLE_PRIORITY + 1 )
#define PRIORITY_2        ( tskIDLE_PRIORITY + 2 )

void taskBody( void * pvParameters ) {
    for (;;) {
        // Wait for the next cycle.
        TMAN_TaskWaitPeriod((char *) pvParameters);
        int tcks = xTaskGetTickCount();
        printf("[%s] Tick: %d \n\r", ((char *) pvParameters), tcks);
        int i, j,k;
        for (i = 0; i < 5; i++)
            for (j = 0; j < 5; j++)
                printf("\r");
        
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
    printf("Teste Número 15\n\r");
    /* Create the tasks defined within this file. */
    xTaskCreate(taskBody, (const signed char * const) "Task A", 
                configMINIMAL_STACK_SIZE, (void *) "Task A", PRIORITY_1, NULL);
    xTaskCreate(taskBody, (const signed char * const) "Task B", 
                configMINIMAL_STACK_SIZE, (void *) "Task B", PRIORITY_2, NULL);
    
    printf("task create 1 e 2\n\r");
    
    TMAN_Init(PERIOD_3000MS);


    
    TMAN_TaskAdd("Task A", PRIORITY_1);
    TMAN_TaskAdd("Task B", PRIORITY_2);
    
    TMAN_TaskRegisterAttributes("Task A", "PERIOD", "5");
    TMAN_TaskRegisterAttributes("Task B", "PERIOD", "7");
    TMAN_TaskRegisterAttributes("Task A", "PHASE", "0");
    TMAN_TaskRegisterAttributes("Task B", "PHASE", "1");
    TMAN_TaskRegisterAttributes("Task A", "DEADLINE", "5");
    TMAN_TaskRegisterAttributes("Task B", "DEADLINE", "7");

    TMAN_TaskRegisterAttributes("Task A", "PRECEDENCE", "Task B");
    

    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
    the scheduler. */

    return 0;
}
