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

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

/* Create enum to track push button status */
enum pushButtonStates{
	lessThanTwoSecs,
	lessThanFourSecs,
	moreThanFourSecs
};

enum pushButtonStates pushButtonState;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/* "LED toggle" task implementation. */
void ledToggle( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );

    for( ;; )
    {
      /* Task code goes here. */
			
			switch (pushButtonState){
				case lessThanTwoSecs:
					// turn the LED off
					GPIO_write(PORT_0, PIN0, PIN_IS_LOW);
					break;
				
				case lessThanFourSecs:
					// turn the LED on
					GPIO_write(PORT_0, PIN0, PIN_IS_HIGH);
					// block the task for 400 ms
					vTaskDelay(400);
					// turn the LED off
					GPIO_write(PORT_0, PIN0, PIN_IS_LOW);
					// block the task for 400 ms
					vTaskDelay(400);
					break;

				case moreThanFourSecs:
					// turn the LED on
					GPIO_write(PORT_0, PIN0, PIN_IS_HIGH);
					// block the task for 100 ms
					vTaskDelay(100);
					// turn the LED off
					GPIO_write(PORT_0, PIN0, PIN_IS_LOW);
					// block the task for 100 ms
					vTaskDelay(100);
					break;
			}
    }
}

void buttonCheck( void * pvParameters ){
	
	unsigned char button = 0;
	configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
	while (1){
		button = GPIO_read(PORT_0, PIN1);
		if (button == PIN_IS_LOW){
			// button is pressed, could be in one of the three possible states
			vTaskDelay(2000);
			button = GPIO_read(PORT_0, PIN1);
			if (button == PIN_IS_LOW){
				// button is still pressed, could be in second or third state
				vTaskDelay(2000);
				button = GPIO_read(PORT_0, PIN1);
				if (button == PIN_IS_LOW){
					// button is still pressed after 4 secs, must be in the third state
					pushButtonState = moreThanFourSecs;
				}
				else {
					// button is released in between 2 and 4 secs, must be in the second state
					pushButtonState = lessThanFourSecs;
					// move the task to the blocked state so that the button could save its current state, simplest method to do so is the vTaskDelay()
					vTaskDelay(2000);
				}
			}
			else {
				// button is released before 2 seconds, must be in the first state
				pushButtonState = lessThanTwoSecs;
				//vTaskDelay(2000);
			}
			}
		else {
			// button is not pressed at all, considered to be in the first state
			pushButtonState = lessThanTwoSecs;
			//vTaskDelay(2000);
		}
	}
}

	/* Handlers declarations */
	TaskHandle_t ledToggleHandler = NULL;
	TaskHandle_t buttonCheckHandler = NULL;

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();


    /* Create Tasks here */
	
	xTaskCreate(
							ledToggle,       /* Function that implements the task. */
							"LED Toggle",          /* Text name for the task. */
							90,      /* Stack size in words, not bytes. */
							( void * ) 1,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							&ledToggleHandler
							);      /* Used to pass out the created task's handle. */
							
	xTaskCreate(
							buttonCheck,       /* Function that implements the task. */
							"button Check",          /* Text name for the task. */
							90,      /* Stack size in words, not bytes. */
							( void * ) 1,    /* Parameter passed into the task. */
							2,/* Priority at which the task is created. */
							&buttonCheckHandler
							);      /* Used to pass out the created task's handle. */						
							

	


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

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


