/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include <queue.h>

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

TaskHandle_t Button1_Detect_Task_Handler     = NULL;
TaskHandle_t Button2_Detect_Task_Handler     = NULL;

TaskHandle_t UARTConsumer_Task_Handler     = NULL;
TaskHandle_t UARTPrint_Task_Handler        = NULL;


uint8_t   edge1ChangeFlag           = pdFALSE;
uint8_t   edge2ChangeFlag           = pdFALSE;

uint8_t 	button1RisingEdge   			 = pdFALSE;           
uint8_t 	button1FallingEdge  			 = pdFALSE; 

uint8_t 	button2RisingEdge   			 = pdFALSE;           
uint8_t 	button2FallingEdge  			 = pdFALSE;

uint8_t 	lastButton1State    			 = pdFALSE;
uint8_t 	lastButton2State    			 = pdFALSE;

const signed char   RisingDetectionString[] 	 = "Rising Edge\n";
const signed char   FallingDetectionString[] 	 = "Falling Edge\n";
const signed char   UARTPrintString[] 				 = "Hello UART\n";

const signed char   BUTTON_1_ID[] 	   = "BUTTON_1------->";
const signed char   BUTTON_2_ID[] 	   = "BUTTON_2------->";
const signed char   PRINT_TASK_ID[] 	 = "UART String---->";

//Creates a handler by which the queue can be referenced.
QueueHandle_t xQueue1;

#define UART_loc_string1Length			30
struct AMessage
{
//    signed char ucMessageID;
	  const signed char *ucID;
    const signed char *ucData;
};








/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

		
/*  Task to be created */
void Button1_Detect_Task(void * pvParameters)
{
	struct AMessage* pxPinterToButton1Message;
	pxPinterToButton1Message->ucID = BUTTON_1_ID;
 for(;;)
	{
		 uint8_t buttonState = GPIO_read(PORT_0, PIN0);
	//Here starts the code for detecting an edge
		if ( buttonState != lastButton1State ) 
			{
				if (buttonState == PIN_IS_LOW) 
				{
					pxPinterToButton1Message->ucData = FallingDetectionString;
				}
				else 
				{
					pxPinterToButton1Message->ucData = RisingDetectionString;
				}
				
				lastButton1State = buttonState;
			 /* Send the address of xMessage to the queue created to hold 10    pointers. */
			 xQueueSend( /* The handle of the queue. */
									 xQueue1,
									 /* The address of the variable that holds the address of xMessage.
									 sizeof( &xMessage ) bytes are copied from here into the queue. As the
									 variable holds the address of xMessage it is the address of xMessage
									 that is copied into the queue. */
									 ( void * ) &pxPinterToButton1Message,
									 ( TickType_t ) 0 );				
			}
		else
			{
				//edgeDetectionFlag = pdFALSE;
			}	
		/*Provide 50 tick delay to give the cpu access*/
		vTaskDelay(50);			
	 }		
	vTaskDelete(Button1_Detect_Task_Handler);
}

/*  Task to be created */
void Button2_Detect_Task(void * pvParameters)
{
	struct AMessage* pxPinterToButton2Message;
	pxPinterToButton2Message->ucID = BUTTON_2_ID;
 for(;;)
	{
		 uint8_t buttonState = GPIO_read(PORT_0, PIN1);
	//Here starts the code for detecting an edge
		if ( buttonState != lastButton1State ) 
			{
				if (buttonState == PIN_IS_LOW) 
				{
					pxPinterToButton2Message->ucData = FallingDetectionString;
				}
				else 
				{
					pxPinterToButton2Message->ucData = RisingDetectionString;
				}
				lastButton1State = buttonState; 
			 /* Send the address of xMessage to the queue created to hold 10    pointers. */
			 xQueueSend( /* The handle of the queue. */
									 xQueue1,
									 /* The address of the variable that holds the address of xMessage.
									 sizeof( &xMessage ) bytes are copied from here into the queue. As the
									 variable holds the address of xMessage it is the address of xMessage
									 that is copied into the queue. */
									 ( void * ) &pxPinterToButton2Message,
									 ( TickType_t ) 0 );
			}
		else
			{
				//edgeDetectionFlag = pdFALSE;
			}	
		/*Provide 50 tick delay to give the cpu access*/
			
		vTaskDelay(50);			
	 }		
	vTaskDelete(Button2_Detect_Task_Handler);
}


/*  Task to be created */
void UARTPrint_Task(void * pvParameters)
{
	struct AMessage* pxPointerToPrintMessage;
	pxPointerToPrintMessage->ucID = PRINT_TASK_ID;
 for(;;)
	{
		 pxPointerToPrintMessage->ucData = UARTPrintString;

			 /* Send the address of xMessage to the queue created to hold 10    pointers. */
		 xQueueSend( /* The handle of the queue. */
								 xQueue1,
								 /* The address of the variable that holds the address of xMessage.
								 sizeof( &xMessage ) bytes are copied from here into the queue. As the
								 variable holds the address of xMessage it is the address of xMessage
								 that is copied into the queue. */
								 ( void * ) &pxPointerToPrintMessage,
								 ( TickType_t ) 0 );
		/*Provide 100 tick delay to give the cpu access*/		
		vTaskDelay(100);			
	 }		
	 vTaskDelete(UARTPrint_Task_Handler);
}


/*  Task to be created */
void UARTConsumer_Task(void * pvParameters)
{


	
	struct AMessage *pxRxedPointer;
	for(;;)
	{			
			
		if( xQueue1 != NULL )
			{
						/* Queue was created and must be used. */
				/* Receive a message from the created queue to hold pointers.  Block for 10
				ticks if a message is not immediately available.  The value is read into a
				pointer variable, and as the value received is the address of the xMessage
				variable, after this call pxRxedPointer will point to xMessage. */
				if( xQueueReceive( xQueue1,
													&( pxRxedPointer ),
													( TickType_t ) 10 ) == pdPASS )
				{
					/* xRxedStructure now contains a copy of xMessage. */
					while( ( vSerialPutString(pxRxedPointer->ucID, UART_loc_string1Length) )   == pdFALSE   );
					while( ( vSerialPutString(pxRxedPointer->ucData, UART_loc_string1Length) ) == pdFALSE   );
				}					
				
			}

	/*Provide 50 tick delay to give the cpu access*/
	vTaskDelay(50);			
	}
	 vTaskDelete(UARTConsumer_Task_Handler);
}



/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
 {
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	//xSerialPortInitMinimal(ser19200 );
	/* Create a queue capable of containing 10 unsigned long values. */
	xQueue1 = xQueueCreate( 10, sizeof( struct AMessage * ) );

				/* Create Tasks here */
			
			xTaskCreate( 
									Button1_Detect_Task,
									"Button1_Detect_Task",
									configMINIMAL_STACK_SIZE,
									(void *) 1,
									1,												
									&Button1_Detect_Task_Handler );
									
											/* Create Tasks here */
			xTaskCreate( 
									Button2_Detect_Task,
									"Button2_Detect_Task",
									configMINIMAL_STACK_SIZE,
									(void *) 1,
									1,												/*tskIDLE_PRIORITY*/
									&Button2_Detect_Task_Handler );

		 xTaskCreate( 
									UARTConsumer_Task,
									"UARTConsumer_Task",
									configMINIMAL_STACK_SIZE,
									(void *) 1,
									1,												
									&UARTConsumer_Task_Handler );
									
											/* Create Tasks here */
//			xTaskCreate( 
//									UARTPrint_Task,
//									"UARTPrint_Task",
//									configMINIMAL_STACK_SIZE,
//									(void *) 1,
//									1,												/*tskIDLE_PRIORITY*/
//									&UARTPrint_Task_Handler );
			/* Now all the tasks have been started - start the scheduler.

			NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
			The processor MUST be in supervisor mode when vTaskStartScheduler is 
			called.  The demo applications included in the FreeRTOS.org download switch
			to supervisor mode prior to main being called.  If you are not using one of
			these demo application projects then ensure Supervisor mode is used here. */
			vTaskStartScheduler();
	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/

