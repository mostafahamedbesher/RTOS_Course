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
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );

TaskHandle_t Detect_rising_edge_Handler = NULL;
TaskHandle_t Detect_falling_edge_Handler = NULL;
TaskHandle_t Uart_Send_Task_Handler = NULL;
TaskHandle_t Send_Periodic_string_Handler = NULL;
signed const char Periodic_string[] = "Task3 ";
signed const char Rising_string[] = "Rising Edge ";
signed const char Falling_string[] = "Falling Edge ";
// queue handler										
QueueHandle_t Queue1;
signed char UART_Buffer[15] = "";

/*-----------------------------------------------------------*/




void Detect_rising_edge_Task( void * pvParameters )
{
		uint32_t Currrent_state = 0;
		uint32_t Previous_state = 0;
    for( ;; )
    {
				//read port0 pin0 state
				Currrent_state = GPIO_read(PORT_0,PIN0);
				// check if rising edge happens
				if(Currrent_state == 1 && Previous_state == 0)
				{
					//check for available spaces in queue
					if(uxQueueSpacesAvailable(Queue1) != 0)
					{
						//send rising edge string to queue
						xQueueSend(Queue1,( void * )Rising_string,( TickType_t )10);
					}
					
				}
				
				Previous_state = Currrent_state;
				
				
				vTaskDelay(200);
			
    }
}



void Detect_falling_edge_Task( void * pvParameters )
{

    uint32_t Currrent_state = 0;
		uint32_t Previous_state = 0;
    for( ;; )
    {
			//read port0 pin1 state
				Currrent_state = GPIO_read(PORT_0,PIN1);
				// check if falling edge happens
				if(Currrent_state == 0 && Previous_state == 1)
				{
					if(uxQueueSpacesAvailable(Queue1) != 0)
					{
						//send Falling edge string to queue
						xQueueSend(Queue1,( void * )Falling_string,( TickType_t )10);
					}
				}
				
				Previous_state = Currrent_state;
				
				
				vTaskDelay(200);
			
    }
			
		
}


void Uart_Send_Task( void * pvParameters )
{
    for( ;; )
    {
			//if data recieved successfully from queue1
			if(xQueueReceive(Queue1,UART_Buffer,( TickType_t ) 10) == pdTRUE)
			{
				//compare if periodic task to send because it has different string size
				if(strcmp(UART_Buffer,Periodic_string) == 0)
				{
					vSerialPutString(UART_Buffer,6);
				}
				else
				{
					//Send Data using UART
					vSerialPutString(UART_Buffer,13);
				}
				
			}
			
			vTaskDelay(100);
				
		}
			
		
}


void Send_Periodic_string( void * pvParameters )
{

    for( ;; )
    {	
			if(uxQueueSpacesAvailable(Queue1) != 0)
			{
				
				//send string every 100ms
				xQueueSend(Queue1,( void * )Periodic_string,( TickType_t )10);
			}
			vTaskDelay(100);
		}
			
}





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
                    Detect_rising_edge_Task,       /* Function that implements the task. */
                    "Detect_rising_edge_Task",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Detect_rising_edge_Handler );      /* Used to pass out the created task's handle. */
										
		xTaskCreate(
                    Detect_falling_edge_Task,       /* Function that implements the task. */
                    "Detect_falling_edge_Task",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Detect_falling_edge_Handler );      /* Used to pass out the created task's handle. */
										
		xTaskCreate(
                    Uart_Send_Task,       /* Function that implements the task. */
                    "Uart_Send_Task",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Uart_Send_Task_Handler );      /* Used to pass out the created task's handle. */
										
		xTaskCreate(
                    Send_Periodic_string,       /* Function that implements the task. */
                    "Send_Periodic_string",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &Send_Periodic_string_Handler );      /* Used to pass out the created task's handle. */
		
										
		Queue1 = xQueueCreate( 3,13);
                              
										
							

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


