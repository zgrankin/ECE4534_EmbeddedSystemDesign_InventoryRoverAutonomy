#include "system/common/sys_common.h"
#include "system_definitions.h"
#include "system_config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "navigation.h"
#include "navigation_public.h"
#include "bookdetection.h"
#include "bookdetection_public.h"
#include "motorThread.h"
#include "motorThread_public.h"
#include "debug.h"

enum UltrasonicStates {FRONTTRIG, FRONTCAP, RIGHTTRIG, RIGHTCAP, LEFTTRIG, LEFTCAP}; 
volatile enum UltrasonicStates SENSORSTATE = FRONTTRIG;

#define TICKSPERCYCLE 4
#define CYCLESPERREV 12
#define GEARRATIO 297.92

void IntHandlerDrvTmrInstance0(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_4);
}
void IntHandlerDrvTmrInstance1(void) // 10us timer, controls US sensors
{
    static float tempF, tempR, tempL;    
        
    if (getTriggerStatus())
    {   
        if (SENSORSTATE == FRONTTRIG || SENSORSTATE == FRONTCAP)
            SENSORSTATE = FRONTTRIG;
        setTriggerStatus(false);
        resetUSCount('a');
    }

    switch(SENSORSTATE) 
    {
        case FRONTTRIG:
        {
            PLIB_PORTS_PinWrite(PORTS_ID_0, TRIGGER_CHANNEL_FRONT, TRIGGER_BIT_FRONT, 1);
            DRV_TMR1_CounterClear();
            resetUSCount('a');
            SENSORSTATE = FRONTCAP;
            break;
        }
        case FRONTCAP:
        {
            PLIB_PORTS_PinWrite(PORTS_ID_0, TRIGGER_CHANNEL_FRONT, TRIGGER_BIT_FRONT, 0);
            if(PLIB_PORTS_PinGet(PORTS_ID_0, ECHO_CHANNEL_FRONT, ECHO_BIT_FRONT) == 1)
                incUSCount('f');
            else if(PLIB_PORTS_PinGet(PORTS_ID_0, ECHO_CHANNEL_FRONT, ECHO_BIT_FRONT) == 0 && getUSCount('f') > 0)
            {
                float holder;
                holder = ((getUSCount('f') * 1e-5) * 171.5) * 1000; // convert to seconds, multiply by mm/s
                if (holder < 950) {
                    setTx('f', (holder + tempF) / 2);
                    tempF = holder;
                }
                SENSORSTATE = FRONTTRIG;
            }
            break;
        }
       default:
           break;
    }

    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_5);
}

void IntHandlerDrvTmrInstance2(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_2);
}
void IntHandlerDrvTmrInstance3(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_3);
}

void IntHandlerDrvUsartInstance0(void)
{
    if(SYS_INT_SourceStatusGet(INT_SOURCE_USART_1_RECEIVE)) 
    {
        if (!DRV_USART0_ReceiverBufferIsEmpty())
        {
            writeToWifiReceiveQueue(PLIB_USART_ReceiverByteReceive(USART_ID_1));
        }
    }
    DRV_USART_TasksTransmit(sysObj.drvUsart0);
    DRV_USART_TasksError(sysObj.drvUsart0);
    DRV_USART_TasksReceive(sysObj.drvUsart0);
}
 
void IntHandlerDrvUsartInstance1(void)
{
    if (PLIB_USART_ReceiverDataIsAvailable(USART_ID_2))
    {
        
    }
    DRV_USART_TasksTransmit(sysObj.drvUsart1);
    DRV_USART_TasksError(sysObj.drvUsart1);
    DRV_USART_TasksReceive(sysObj.drvUsart1);
}

// right motor
void IntHandlerExternalInterruptInstance0(void)
{
    BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
    incEncoderTicksRight();
    incTotalEncoderTicksRight();
    if (returnLocalEncoderTicks('r') >= returnCapEncoderTicks('r'))
    {
        setRightMotor(FORWARD, 0);
        setReady('r', 1);
    }
    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_EXTERNAL_1);
}

// left motor
void IntHandlerExternalInterruptInstance1(void)
{
    BaseType_t pxHigherPriorityTaskWoken=pdFALSE;
    incEncoderTicksLeft();
    incTotalEncoderTicksLeft();
    if (returnLocalEncoderTicks('l') >= returnCapEncoderTicks('l'))
    {
        setLeftMotor(FORWARD, 0);
        setReady('l', 1);
    }
    PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_EXTERNAL_2);
}