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
#define PERIOD_1000MS           ( 1000 / portTICK_RATE_MS ) // 

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define PRIORITY_1        ( tskIDLE_PRIORITY + 1 )
#define PRIORITY_2        ( tskIDLE_PRIORITY + 2 )

//
//
///*
// * Prototypes and tasks
// */
//
//void pvDataAcq( void ) {
//    int voltage;
//    TickType_t xLastWakeTime = xTaskGetTickCount();
//    const TickType_t xFrequency = DATA_ACQ_PERIOD_MS;
//
//    for (;;) {
//        // Wait for the next cycle.
//        vTaskDelayUntil(&xLastWakeTime, xFrequency);
//
//        // Get one sample
//        IFS1bits.AD1IF = 0; // Reset interrupt flag
//        AD1CON1bits.ASAM = 1; // Start conversion
//        while (IFS1bits.AD1IF == 0); // Wait fo EOC
//
//        // Convert to 0..3.3V 
//        voltage = (int) (ADC1BUF0 *  100) / 1023;
//
//        xQueueSend(xQueue1, voltage, DATA_ACQ_PERIOD_MS);
//        
//    }
//}

void taskBody( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = PERIOD_1000MS;
    
    for (;;) {
        // Wait for the next cycle.
//        TMAN_TaskWaitPeriod(xLastWakeTime);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        printf("eu sou uma task muito bonita\n");
        
    }
}

void taskBody2(void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = PERIOD_1000MS;


    for (;;) {
        // Wait for the next cycle.
//        TMAN_TaskWaitPeriod(xLastWakeTime);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        printf("eu sou uma task muito inutil\n");

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
    
    printf("\n\n *********************************************\n\r");
    printf("Teste N�mero 10\n");
    /* Create the tasks defined within this file. */
    xTaskCreate(taskBody , (const signed char * const) "Task A", 
                configMINIMAL_STACK_SIZE, NULL, PRIORITY_1, NULL);
    xTaskCreate(taskBody2, (const signed char * const) "Task B", 
                configMINIMAL_STACK_SIZE, NULL, PRIORITY_2, NULL);
    
    printf("task create 1 e 2\n");
    
    

    
    TMAN_Init(PERIOD_MS);

    printf("TEste");
    
    vTaskStartScheduler();

    
    TMAN_TaskAdd("Task A");
    TMAN_TaskAdd("Task B");

    /* Finally start the scheduler. */
    

    /* Will only reach here if there is insufficient heap available to start
    the scheduler. */

    return 0;
}
