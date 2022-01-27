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

/*-----------------------------------------------------------*/

/*
 * Initialization of the framework.
 */
void TMAN_Init( int tick_ms);

/*
 * Terminate the framework.
 */
void TMAN_Close( void );

/*
 * Add a task to the framework
 */
void TMAN_TaskAdd( void );

/*
 * Register attributes (period, phase, deadline, precedence constraints)
 * for a task already added to the framework
 */
void TMAN_TaskRegisterAttributes( char taskName[], int period, int phase, int deadline  );

/*
 * Called by a task to signal the termination of an instance and 
 * wait for next activation
 */
void TMAN_TaskWaitPeriod( void );

/*
 * Returns statistical information about a task.
 * Provided information must include at least the number of activations,
 * but additional info (number of deadline misses) will be valued.
 */
void TMAN_TaskStats( void );

/*-----------------------------------------------------------*/

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

void taskBody( void ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        // Wait for the next cycle.
        TMAN_TaskWaitPeriod(xLastWakeTime)

        
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

    /* Finally start the scheduler. */
    vTaskStartScheduler();

    /* Will only reach here if there is insufficient heap available to start
    the scheduler. */

    return 0;
}
