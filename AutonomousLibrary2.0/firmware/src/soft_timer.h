#ifndef _SOFT_TIMER_PUBLIC_H
#define _SOFT_TIMER_PUBLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include <timers.h>

#define NUM_TIMERS 6
TimerHandle_t xTimers[ NUM_TIMERS ]; // moved this from top of soft_timer.c

void timerCreation( void );
void vNavigationTimerCallback( TimerHandle_t xTimer );
void vMotorSampleTimerCallback( TimerHandle_t xTimer );
void vTestCaseTimerCallback( TimerHandle_t xTimer );
void vMapTimerCallback( TimerHandle_t xTimer );
void vDestinationRequestTimerCallback( TimerHandle_t xTimer );
void vMotorThreadTimerCallback( TimerHandle_t xTimer );
void resetASoftwareTimer( int timerNum );

#endif