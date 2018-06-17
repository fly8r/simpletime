/*
 * rtos.c
 *
 * Created: 06.11.2017 13:45:13
 *  Author: fly8r
 */
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "rtos.h"

/************************************************************************/
/* VARS                                                                 */
/************************************************************************/
volatile static    TPTR    RTOS_TaskQueue[RTOS_TASK_QUEUE_SIZE+1];		// Task queue pointers
volatile static    struct  RTOS_TimerTaskQueue							// Timer structure
{
    TPTR        RunTask;
    uint16_t    Time;
} RTOS_TimerTaskQueue[RTOS_TIMER_TASK_QUEUE_SIZE+1];


/************************************************************************/
/* RTOS Initialization                                                  */
/************************************************************************/
inline void RTOS_Init(void)
{
    uint8_t i=0;

    // Initialization RTOS task queue
    for(i=0; i < RTOS_TASK_QUEUE_SIZE; i++)
    {
        RTOS_TaskQueue[i] = Idle;
    }

    // Initialization RTOS timer task queue
    for(i=0; i < RTOS_TIMER_TASK_QUEUE_SIZE; i++)
    {
        RTOS_TimerTaskQueue[i].RunTask = Idle;
        RTOS_TimerTaskQueue[i].Time = 0;
    }
}

/************************************************************************/
/* IDLE function                                                        */
/************************************************************************/
inline void Idle(void)
{
}

/************************************************************************/
/* RTOS Setup task into queue                                           */
/************************************************************************/
void RTOS_SetTask(TPTR TS)
{
    uint8_t     i=0;
    // Disable interrupts while processing queue
    // if interrupt was enabled
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Checking queue free
		while(RTOS_TaskQueue[i] != Idle)
		{
			i++;
			// If no free space - return
			if (i == RTOS_TASK_QUEUE_SIZE)
				return;
		}

		// Adding task into queue
		RTOS_TaskQueue[i] = TS;
	}

}

/************************************************************************/
/* RTOS Setup task into timer queue                                     */
/************************************************************************/
void RTOS_SetTimerTask(TPTR TS, uint16_t NewTime)
{
    uint8_t     i=0;

    // Disable interrupts while processing queue
    // if interrupt was enabled
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// Update time interval for task if exists in queue
		for(i=0; i<RTOS_TIMER_TASK_QUEUE_SIZE; i++) {
			// Find task in queue
			if(RTOS_TimerTaskQueue[i].RunTask == TS) {
				// Set new time for run
				RTOS_TimerTaskQueue[i].Time = NewTime;
				return;
			}
		}

		// Setup task into queue if not exists in queue
		for(i=0; i < RTOS_TIMER_TASK_QUEUE_SIZE; i++) {
			// Search free space in task queue
			if (RTOS_TimerTaskQueue[i].RunTask == Idle) {
				// Set task
				RTOS_TimerTaskQueue[i].RunTask = TS;
				RTOS_TimerTaskQueue[i].Time = NewTime;
				return;
			}
		}
	}
    // IF NO FREE SPACE IN QUEUE - IGNORE TASK!!!
}

/************************************************************************/
/* RTOS Task Manager                                                    */
/************************************************************************/
inline void RTOS_TaskManager(void)
{
    uint8_t	    i=0;
    TPTR	    RunTask=Idle;

    // Disable interrupts
    //RTOS_INTERRUPT_DISABLE();
	cli();

    // Get first task from queue
    RunTask = RTOS_TaskQueue[0];

    // If task is IDLE - run IDLE function
    if (RunTask == Idle) {
        //RTOS_INTERRUPT_ENABLE();
		sei();
        (Idle)();

        // If task is other function - run function
    } else {
        // Shift pointers in queue
        for(i=0; i < RTOS_TASK_QUEUE_SIZE-1; i++) {
			RTOS_TaskQueue[i] = RTOS_TaskQueue[i+1];
        }

        // Setup IDLE function into last cell of queue
        RTOS_TaskQueue[RTOS_TASK_QUEUE_SIZE-1] = Idle;

        // Enable interrupts
        //RTOS_INTERRUPT_ENABLE();
		sei();
        (RunTask)();
    }
}

/************************************************************************/
/* RTOS Timer service                                                   */
/************************************************************************/
inline void RTOS_TimerService(void)
{
    uint8_t     i;

    // Processing TASK queue
    for(i=0; i < RTOS_TIMER_TASK_QUEUE_SIZE; i++) {
        // If current task is IDLE - continue
        if(RTOS_TimerTaskQueue[i].RunTask == Idle) continue;

        // If current task is not IDLE
        if(RTOS_TimerTaskQueue[i].Time > 0) {
            // If time not left - decrement
            RTOS_TimerTaskQueue[i].Time--;
        } else {
            // Else - set task for run
            RTOS_SetTask(RTOS_TimerTaskQueue[i].RunTask);
            // Remove task from timer queue
            RTOS_TimerTaskQueue[i].RunTask = Idle;
        }
    }
}
