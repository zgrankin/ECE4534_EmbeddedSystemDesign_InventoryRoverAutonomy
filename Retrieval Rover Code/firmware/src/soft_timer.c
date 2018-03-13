#include "soft_timer.h"
#include "navigation_public.h"
#include "bookdetection_public.h"
#include "motorThread_public.h"
#include "debug.h"
#include <math.h>
#define RATIO 0.0032258064516129032258064516129
#define TICKSPERCYCLE 4
#define CYCLESPERREV 12
#define GEARRATIO 297.92


void timerCreation()
{
    xTimers[ 0 ] = xTimerCreate
              ( "Timer",
                pdMS_TO_TICKS( 1000 ),
                pdTRUE,
                ( void * ) 0,
                vNavigationTimerCallback
              );
    xTimers[ 1 ] = xTimerCreate
              ( "Timer",
                pdMS_TO_TICKS( 300 ),
                pdTRUE,
                ( void * ) 0,
                vMotorSampleTimerCallback
              );
    xTimers[ 2 ] = xTimerCreate
              ( "Timer",
                pdMS_TO_TICKS( 100000 ),
                pdTRUE,
                ( void * ) 0,
                vTestCaseTimerCallback
              );
    xTimers[ 3 ] = xTimerCreate
            ( "Timer",
            pdMS_TO_TICKS( 3000 ),
            pdTRUE,
            ( void * ) 0,
            vDestinationRequestTimerCallback
            );
    xTimers[ 4 ] = xTimerCreate
            ( "Timer",
            pdMS_TO_TICKS( 1000 ),
            pdTRUE,
            ( void * ) 0,
            vMotorThreadTimerCallback
            );

    unsigned int i;
    for (i = 0; i < NUM_TIMERS; i++)
    {
        xTimerStart( xTimers[ i ], 0 );
    }
}

void vNavigationTimerCallback( TimerHandle_t xTimer )
{
    Message_Nav tx;
    tx = getUSTx();
    sendToQueueFromISRNav(tx);
    setTriggerStatus(true);

    configASSERT( pxTimer );
}

void vMotorSampleTimerCallback( TimerHandle_t xTimer )
{
    sampleHasBeenTaken = true;
    
    float RMspd = DRV_TMR0_CounterValueGet();
    float LMspd = DRV_TMR3_CounterValueGet();

    RM.speed = TIMER1_SAMPLING_FREQ * TICKSPERCYCLE * RMspd /(CYCLESPERREV * GEARRATIO);    
    LM.speed = TIMER1_SAMPLING_FREQ * TICKSPERCYCLE * LMspd /(CYCLESPERREV * GEARRATIO);

    DRV_TMR0_CounterClear();
    DRV_TMR3_CounterClear();
    
    currTime = currTime + (1.0 / TIMER1_SAMPLING_FREQ); 
    
    configASSERT( pxTimer );
}

void vTestCaseTimerCallback( TimerHandle_t xTimer )
{
//    char request[256];
//    sprintf(request, "{\"Type\": \"Request\", \"Request\": \"testcase\"}");
//    sendJSONToServerFromISR(request);
}

void vDestinationRequestTimerCallback( TimerHandle_t xTimer )
{
    static char request[256];
    sprintf(request, "{\"Type\": \"Request\", \"Request\": \"BookDestination\"}");
    sendJSONToServerFromISR(request);
}

void vMotorThreadTimerCallback( TimerHandle_t xTimer )
{
    if (isReady())
        incSecondCount();
    if (getSecondCount() >= 2)
    {
        resetSecondCount();
        Message_Motor temp;
        temp.state = 3;
        sendToQueueMotor(temp);
    }
}

void resetASoftwareTimer( int timerNum )
{
    xTimerReset(xTimers[ timerNum ], 0);
}