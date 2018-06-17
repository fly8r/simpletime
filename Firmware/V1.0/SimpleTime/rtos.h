/*
 * rtos.h
 *
 * Created: 06.11.2017 13:45:24
 *  Author: v.bandura
 */
#include <stdio.h>


extern  void    Idle(void);
extern  void    RTOS_Init(void);

typedef void    (*TPTR)(void);

extern  void    RTOS_SetTask(TPTR TS);
extern  void    RTOS_SetTimerTask(TPTR TS, uint16_t NewTime);
extern  void    RTOS_TaskManager(void);
extern  void    RTOS_TimerService(void);
