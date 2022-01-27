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


/* App includes */
#include "../UART/uart.h"


#define SYSCLK  80000000L // System clock frequency, in Hz
#define PBCLOCK 40000000L // Peripheral Bus Clock frequency, in Hz

/* Set the tasks' period (in system ticks) */
#define DATA_ACQ_PERIOD_MS 	( 100 / portTICK_RATE_MS ) // 

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define ACQ_PRIORITY        ( tskIDLE_PRIORITY + 3 )
#define PROC_PRIORITY	    ( tskIDLE_PRIORITY + 2 )
#define OUT_PRIORITY	    ( tskIDLE_PRIORITY + 1 )

#define LAST_FIVE_ARRAY_LENGTH 5

QueueHandle_t xQueue1, xQueue2;


/*
 * Prototypes and tasks
 */

void pvDataAcq( void ) {
    int voltage;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = DATA_ACQ_PERIOD_MS;

    for (;;) {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Get one sample
        IFS1bits.AD1IF = 0; // Reset interrupt flag
        AD1CON1bits.ASAM = 1; // Start conversion
        while (IFS1bits.AD1IF == 0); // Wait fo EOC

        // Convert to 0..3.3V 
        voltage = (int) (ADC1BUF0 *  100) / 1023;

        xQueueSend(xQueue1, voltage, DATA_ACQ_PERIOD_MS);
        
    }
}

void pvDataProc( void ) {
    int voltage, average;
    int lastFive[LAST_FIVE_ARRAY_LENGTH] = {0, 0, 0, 0, 0};
    int index = 0;

    for (;;) {
        if (xQueueReceive(xQueue1, voltage, DATA_ACQ_PERIOD_MS) == pdPASS) {
            // Save value in array
            lastFive[index] = voltage;
            index++;
            if (index == LAST_FIVE_ARRAY_LENGTH) {
                index = 0;
            }
        }

        // Calculate average   
        average = 0;
        for (int i = 0; i < LAST_FIVE_ARRAY_LENGTH; i++) {
            average = average + lastFive[index];
        }
        average = (int) average / LAST_FIVE_ARRAY_LENGTH;

        xQueueSend(xQueue2, average, DATA_ACQ_PERIOD_MS);
        
    }
}

void pvDataOut(void) {
    int average;
    for (;;) {
        if (xQueueReceive(xQueue1, average, DATA_ACQ_PERIOD_MS) == pdPASS) {
            printf("Average: %d Celsius\n\r", average);
        }
    }
}


/*
 * Create the demo tasks then start the scheduler.
 */
int main_tman( void ) {

    xQueue1 = xQueueCreate(5, sizeof (int));
    xQueue2 = xQueueCreate(5, sizeof (int));
    
	// Init UART and redirect stdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
     // Disable JTAG interface as it uses a few ADC ports
    DDPCONbits.JTAGEN = 0;

    // Initialize ADC module
    // Polling mode, AN0 as input
    // Generic part
    AD1CON1bits.SSRC = 7; // Internal counter ends sampling and starts conversion
    AD1CON1bits.CLRASAM = 1; //Stop conversion when 1st A/D converter interrupt is generated and clears ASAM bit automatically
    AD1CON1bits.FORM = 0; // Integer 16 bit output format
    AD1CON2bits.VCFG = 0; // VR+=AVdd; VR-=AVss
    AD1CON2bits.SMPI = 0; // Number (+1) of consecutive conversions, stored in ADC1BUF0...ADCBUF{SMPI}
    AD1CON3bits.ADRC = 1; // ADC uses internal RC clock
    AD1CON3bits.SAMC = 16; // Sample time is 16TAD ( TAD = 100ns)
    // Set AN0 as input
    AD1CHSbits.CH0SA = 0; // Select AN0 as input for A/D converter
    TRISBbits.TRISB0 = 1; // Set AN0 to input mode
    AD1PCFGbits.PCFG0 = 0; // Set AN0 to analog mode
    // Enable module
    AD1CON1bits.ON = 1; // Enable A/D module (This must be the ***last instruction of configuration phase***)

    // Welcome message
    
    printf("\n\n *********************************************\n\r");

    /* Create the tasks defined within this file. */
    xTaskCreate(pvDataAcq, (const signed char * const) "Acq", configMINIMAL_STACK_SIZE, NULL, ACQ_PRIORITY, NULL);
    xTaskCreate(pvDataProc, (const signed char * const) "Proc", configMINIMAL_STACK_SIZE, NULL, PROC_PRIORITY, NULL);
    xTaskCreate(pvDataOut, (const signed char * const) "Out", configMINIMAL_STACK_SIZE, NULL, OUT_PRIORITY, NULL);

    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
    the scheduler. */

    return 0;
}
