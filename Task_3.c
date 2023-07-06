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


QueueHandle_t xQueue1;

SemaphoreHandle_t mutex;






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

 for(;;)
	{
		 uint8_t buttonState = GPIO_read(PORT_0, PIN0);
 //Here starts the code for detecting an edge
	if ( buttonState != lastButton1State ) 
		{
		/* Create the semaphore to guard a shared resource.  As we are using
    the semaphore for mutual exclusion we create a mutex semaphore
    rather than a binary semaphore. */
			if( xSemaphoreTake( mutex, ( TickType_t ) 0 ) == pdTRUE )
			{
				  if (buttonState == PIN_IS_LOW) 
					 {
						button1RisingEdge = PIN_IS_LOW;
					 }
				  else 
					 {
						button1RisingEdge = PIN_IS_HIGH;
					 }
				 
					 lastButton1State = buttonState; 
				   edge1ChangeFlag = pdTRUE	;
					 //xSemaphoreGive( mutex );
					}
					else
					{
						//edgeDetectionFlag = pdFALSE;
					}		
			}
		  else
		  {
				  edge1ChangeFlag = pdFALSE;
		  }				
		/*Provide 50 tick delay to give the cpu access*/
		vTaskDelay(50);	
	}		
	vTaskDelete(Button1_Detect_Task_Handler);
}

/*  Task to be created */
void Button2_Detect_Task(void * pvParameters)
{
	for(;;)
	{
 BaseType_t xQueueSend(
                            xQueue1,
                            const void * pvItemToQueue,
                            TickType_t xTicksToWait
                         );
	/*Task code goes here*/
 uint8_t buttonState = GPIO_read(PORT_0, PIN1);
 //Here starts the code for detecting an edge
 	if ( buttonState != lastButton2State ) 
		{
		/* Create the semaphore to guard a shared resource.  As we are using
    the semaphore for mutual exclusion we create a mutex semaphore
    rather than a binary semaphore. */
			if( xSemaphoreTake( mutex, ( TickType_t ) 0 ) == pdTRUE )
			{
				  if (buttonState == PIN_IS_LOW) 
					 {
						button2RisingEdge = PIN_IS_LOW;
					 }
				  else 
					 {
						button2RisingEdge = PIN_IS_HIGH;
					 }
				 
					 lastButton2State = buttonState; 
				   edge2ChangeFlag = pdTRUE	;
					 //xSemaphoreGive( mutex );
					}
					else
					{
						//edgeDetectionFlag = pdFALSE;
					}		
			}
		  else
		  {
				  edge2ChangeFlag = pdFALSE;
		  }				
		/*Provide 50 tick delay to give the cpu access*/
		vTaskDelay(50);	
	}	
	vTaskDelete(Button2_Detect_Task_Handler);
}


/*  Task to be created */
void UARTPrint_Task(void * pvParameters)
{
	 /* The semaphore was created successfully and
       can be used. */
	uint8_t UART_loc_writtingLoopCounter = pdFALSE;
	uint32_t UART_loc_heavyLoadCounter   = pdFALSE;
	const char str[] = "Task 1 got access\n";
	uint8_t UART_loc_stringLength = strlen(str);
	for(;;)
	{
		UART_loc_writtingLoopCounter = pdFALSE;
		UART_loc_heavyLoadCounter    = pdFALSE;
		 /* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
		if( xSemaphoreTake( mutex, ( TickType_t ) 30 ) == pdTRUE )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */	
				while( UART_loc_writtingLoopCounter < 10 )
				{
					//printing the string
					(void)vSerialPutString(str, UART_loc_stringLength);
					//(void)xSerialPutChar('B');

					 UART_loc_writtingLoopCounter++;	
				}
					for(UART_loc_heavyLoadCounter = pdFALSE ; UART_loc_heavyLoadCounter < 5000; UART_loc_heavyLoadCounter++)
					{
						//Empty loop, To simulate heavy load
					}				
					/* We have finished accessing the shared resource.  Release the
						semaphore. */
				xSemaphoreGive( mutex );
			}
			else
			{
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
			}
	/*Provide 100 tick delay to give the cpu access*/
	vTaskDelay(100);			
	}
	 vTaskDelete(UARTPrint_Task_Handler);
}


/*  Task to be created */
void UARTConsumer_Task(void * pvParameters)
{
	 /* The semaphore was created successfully and
       can be used. */
	xQueue1 = xQueueCreate( 10, sizeof( unsigned long ) );
	const char str1[] = "Detect Rising Edge\n";
	const char str2[] = "Detect Falling Edge\n";
	uint8_t UART_loc_string1Length = strlen(str1);
	uint8_t UART_loc_string2Length = strlen(str2);
	for(;;)
	{			
		 /* See if we can obtain the semaphore.  If the semaphore is not
				available wait 0 ticks to see if it becomes free. */
  if(pdTRUE == edge1ChangeFlag)  
	{
		if( xSemaphoreTake( mutex, ( TickType_t ) 0 ) == pdTRUE )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */	
						if( PIN_IS_HIGH == button1RisingEdge )  
						{
							(void)vSerialPutString(str1, UART_loc_string1Length);			
						}
						else
						{
							(void)vSerialPutString(str2, UART_loc_string2Length);	
						}
					/* We have finished accessing the shared resource.  Release the
						semaphore. */
						edge1ChangeFlag = pdFALSE;
				xSemaphoreGive( mutex );
			}
			else
			{
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
			}
	}
	
	else if(pdTRUE == edge2ChangeFlag)
	{
		if( xSemaphoreTake( mutex, ( TickType_t ) 0 ) == pdTRUE )
			{
				/* We were able to obtain the semaphore and can now access the
				shared resource. */	
						if( PIN_IS_HIGH == button2RisingEdge)
						{
							(void)vSerialPutString(str1, UART_loc_string1Length);			
						}
						else
						{
							(void)vSerialPutString(str2, UART_loc_string2Length);	
						}
					/* We have finished accessing the shared resource.  Release the
						semaphore. */
						edge2ChangeFlag = pdFALSE;
				xSemaphoreGive( mutex );
			}
			else
			{
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
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
	 mutex = xSemaphoreCreateMutex();

	if(mutex != NULL)
	{
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
	}
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

