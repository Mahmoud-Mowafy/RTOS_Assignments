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

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/
#define BUTTON_TASK_PERIODICITY				50
#define LED_TASK_PERIODICITY				50
/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

TaskHandle_t LedToggle_Task_Handler = NULL;
TaskHandle_t Button_Task_Handler    = NULL;

SemaphoreHandle_t mutex;

uint8_t ButtonStatus = pdFALSE;


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

void LedToggle_Task(void * pvParameters)
{
	 /* The semaphore was created successfully and
       can be used. */
	for(;;)
	{
	
	 /* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks to see if it becomes free. */
			
	  if( PIN_IS_HIGH == ButtonStatus )
	  {
	        if( xSemaphoreTake( mutex, ( TickType_t ) 10 ) == pdTRUE )
	  			{
	  				/* We were able to obtain the semaphore and can now access the
	  				shared resource. */
	  				//Toggle the LED
	  				GPIO_toggle(PORT_0, PIN1);
	  				ButtonStatus = pdFALSE;	
	  					/* We have finished accessing the shared resource.  Release the
	  						semaphore. */
	  				xSemaphoreGive( mutex );
	  			}
	  		else
	  			{
	  					/* We could not obtain the semaphore and can therefore not access
	  					the shared resource safely. */
	  			}
	  }
	  else
	  {
	  			//Do Nothing
	  }
	/*Provide 50 tick delay to give the cpu access*/
	vTaskDelay(LED_TASK_PERIODICITY);			
	}
	 vTaskDelete(LedToggle_Task_Handler);
}

/*  Task to be created */
void Button_Task(void * pvParameters)
{
	 uint8_t  ButtonPressed    = PIN_IS_LOW;
	 uint8_t  ButtonReleased   = PIN_IS_LOW;
	for(;;)
	{

	/*Task code goes here*/
		/*Check if the button is pressed*/
		if( PIN_IS_HIGH == GPIO_read(PORT_0, PIN0))
		{ 
			/*change the buttonPressed status to be high*/
			  ButtonPressed  = PIN_IS_HIGH;
		}
		/*Check if the button is pressed*/
	else if( ( PIN_IS_HIGH == ButtonPressed ) && ( PIN_IS_HIGH != GPIO_read(PORT_0, PIN0) ) )
		{
				ButtonReleased = PIN_IS_HIGH;
				ButtonPressed  = PIN_IS_LOW;
		}
	else if(ButtonReleased == PIN_IS_HIGH)
		{
		/* Create the semaphore to guard a shared resource.  As we are using
    the semaphore for mutual exclusion we create a mutex semaphore
    rather than a binary semaphore. */
			if( xSemaphoreTake( mutex, ( TickType_t ) 10 ) == pdTRUE )
			{
						ButtonStatus = pdTRUE;
					  xSemaphoreGive( mutex );
						ButtonReleased = PIN_IS_LOW;
			}
			else
			{
				
			}		
		}
		/*Provide 50 tick delay to give the cpu access*/
		vTaskDelay(BUTTON_TASK_PERIODICITY);	
	}		
	vTaskDelete(Button_Task_Handler);
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
 {
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	 mutex = xSemaphoreCreateMutex();


	
    /* Create Tasks here */
	
	xTaskCreate( 
							LedToggle_Task,
							"LedToggle_Task",
							configMINIMAL_STACK_SIZE,
							(void *) 1,
							1,												
							&LedToggle_Task_Handler );
							
							    /* Create Tasks here */
	xTaskCreate( 
							Button_Task,
							"Button_Task",
							configMINIMAL_STACK_SIZE,
							(void *) 1,
							1,												/*tskIDLE_PRIORITY*/
							&Button_Task_Handler );

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





GPIO.C

void GPIO_toggle(portX_t portName, pinX_t pinNum)
{
	switch(portName)
	{
		case PORT_0:
			  if(GET_BIT(IOPIN0, pinNum) == 0)
				{
					SET_BIT(IOSET0, pinNum);
				}
				else 
				{
					SET_BIT(IOCLR0, pinNum);
				}
			break;

		case PORT_1:
			  if(GET_BIT(IOPIN1, pinNum) == 0)
				{
					SET_BIT(IOSET1, pinNum);
				}
				else 
				{
					SET_BIT(IOCLR1, pinNum);
				}
	}
}

