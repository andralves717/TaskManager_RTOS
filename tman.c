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

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* App includes */
#include "../UART/uart.h"
#include "tman.h"


/********************************************************************
 * Function: 	TMAN_Init()
 * Precondition: 
 * Input: 		 tick_ms
 * Returns:      TMAN_SUCCESS if Ok.
 *               UARTX_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Initializes Task Manager Framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_Init(int tick_ms) {

}

/********************************************************************
 * Function: 	TMAN_Close()
 * Precondition: 
 * Input: 		
 * Returns:      TMAN_SUCCESS if Ok.
 *               UARTX_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Terminates Task Manager Framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_Close(){
    
}

/********************************************************************
 * Function: 	TMAN_TaskAdd()
 * Precondition: 
 * Input: 		
 * Returns:      TMAN_SUCCESS if Ok.
 *               UARTX_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Add a task to the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskAdd(){
}

/********************************************************************
 * Function: 	TMAN_TaskRegisterAttributes()
 * Precondition: 
 * Input: 		 taskName, attribute, value of the attribute
 * Returns:      TMAN_SUCCESS if Ok.
 *               UARTX_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Register attributes (period, phase, deadline, 
 *               precedence constraints) for a task already added to 
 *               the framework.
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskRegisterAttributes(char taskName[], char attribute[], int value){
}

/********************************************************************
 * Function: 	TMAN_TaskWaitPeriod()
 * Precondition: 
 * Input: 		
 * Returns:      TMAN_SUCCESS if Ok.
 *               UARTX_FAIL error code in case of failure (see tman.h)
 * Side Effects:	 
 * Overview:     Called by a task to signal the termination of an 
 *               instance and wait for the next activation.
 * 
 *		
 * Note:		 	
 * 
 ********************************************************************/

int TMAN_TaskWaitPeriod(){
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


