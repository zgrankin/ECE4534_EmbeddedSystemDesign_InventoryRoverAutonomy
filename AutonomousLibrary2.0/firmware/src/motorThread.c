#include "motorThread.h"
#include "motorThread_public.h"
#include "navigation_public.h"
#include "soft_timer.h"
#include "wifi_receive_thread_public.h"
#include "wifi_send_thread_public.h"
#include "debug.h"
#include "queue.h"

#define TIMER2_PERIOD 500

MOTORTHREAD_DATA motorthreadData;

void MOTORTHREAD_Initialize ( void )
{
    createQueueMotor();
    initOCs();
    initMotor();
    dbgInit();
    PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_8);
    PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_9);
}

void MOTORTHREAD_Tasks ( void )
{
    Message_Nav nav;
    Message_Motor rx;
    while(1)
    {
        rx = readFromQueueMotor();
        switch(rx.state)
        {
            case 1: // turning state
                // motor turn left or right depending on char and amount of degrees
                rx.turnDegrees;
                rx.turnDirection;
                if (rx.turnDegrees > 100)
                {
                    turnRight180();
                }
                else if (rx.turnDirection == 'l' && rx.turnDegrees > 0)
                {
                    turnLeft90();
                }
                else if (rx.turnDirection == 'r' && rx.turnDegrees > 0)
                {
                    turnRight90();
                }
                else
                {
                    nav.type = 3;
                    sendToQueueNav(nav);
                    resetEncoderTicks('a');
                }
                break;
            case 2: // moving forward state
                // motor move forward certain number of inches
                moveForwardCm(rx.distanceToMove);
                break;
            case 3:
                setReady('a', 0);
                nav.type = 3;
                sendToQueueNav(nav);
                resetEncoderTicks('a');
                break;
            default:
                break;
        }
    }
}

QueueHandle_t xQueueMotor;
volatile int timeToSend;
volatile unsigned int encoderTickLeft = 0;
volatile unsigned int encoderTickRight = 0;
volatile unsigned int ticksL = 0;
volatile unsigned int ticksR = 0;
volatile unsigned int tickLCap = 650; //650 for right turn
volatile unsigned int tickRCap = 650; //650 for right turn
volatile bool readyL = false;
volatile bool readyR = false;
volatile unsigned int secondCount = 0;

void incSecondCount()
{
    secondCount++;
}

unsigned int getSecondCount()
{
    return secondCount;
}

void resetSecondCount()
{
    secondCount = 0;
}

void setReady(char choose, int set)
{
    if (choose == 'l' && set == 1)
        readyL = true;
    else if (choose == 'r' && set == 1)
        readyR = true;
    else if (choose == 'a' && set == 1)
    {
        readyL = true;
        readyR = true;
    }
    else if (choose == 'l' && set == 0)
        readyL = false;
    else if (choose == 'r' && set == 0)
        readyR = false;
    else if (choose == 'a' && set == 0)
    {
        readyL = false;
        readyR = false;
    }
}

bool isReady()
{
    return (readyL && readyR);
}

void incTotalEncoderTicksLeft()
{
    encoderTickLeft++;
}

void incTotalEncoderTicksRight()
{
    encoderTickRight++;
}

void incEncoderTicksLeft()
{
    ticksL++;
}

void incEncoderTicksRight()
{
    ticksR++;
}

void resetEncoderTicks(char choose)
{
    if (choose == 'l')
        ticksL = 0;
    else if (choose == 'r')
        ticksR = 0;
    else if (choose == 'a')
    {
        ticksL = 0;
        ticksR = 0;
        tickLCap = 0;
        tickRCap = 0;
    }
    else if (choose == 'c')
    {
        tickLCap = 0;
        tickRCap = 0;
    }
}

int returnTotalEncoderTicks(char choose)
{
    int result = 0;
    if (choose == 'l')
        result = encoderTickLeft;
    else if (choose == 'r')
        result = encoderTickRight;

    return result;
}

int returnLocalEncoderTicks(char choose)
{
    int result = 0;
    if (choose == 'l')
        result = ticksL;
    else if (choose == 'r')
        result = ticksR;
    return result;
}

int returnCapEncoderTicks(char choose)
{
    int result = 0;
    if (choose == 'l')
        result = tickLCap;
    else if (choose == 'r')
        result = tickRCap;
    return result;
}

void setRightMotor(unsigned int direction, unsigned int speed)
{    
    if(speed < 0)
        speed = 0;
    else if(speed > 100)
        speed = 100;
    
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_C, PORTS_BIT_POS_14, direction); //set direction
    DRV_OC0_PulseWidthSet(speed * (TIMER2_PERIOD / 100)); //set speed
    RM.direction = direction;
 }

void setLeftMotor(unsigned int direction, unsigned int speed)
{    
    if(speed < 0)
        speed = 0;
    else if(speed > 100)
        speed = 100;
    
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_1, direction); //set direction
    DRV_OC1_PulseWidthSet(speed * (TIMER2_PERIOD / 100)); //set speed
    LM.direction = direction;
}

void createQueueMotor()
{
    xQueueMotor = xQueueCreate(200, sizeof(Message_Motor));
    if (xQueueMotor == 0)
    {
        dbgHalt();
    }
}
Message_Motor readFromQueueMotor()
{
    Message_Motor rxMessage;
    xQueueReceive(xQueueMotor, &rxMessage, portMAX_DELAY);
    return rxMessage;
}
void sendToQueueMotor( Message_Motor tx )
{
    xQueueSend( xQueueMotor, &tx, portMAX_DELAY); 
}
void sendToQueueFromISRMotor( Message_Motor tx, BaseType_t pxHigherPriorityTaskWoken )
{
    if (xQueueMotor == 0)
    {
        dbgHalt();
    }
    else
    {
        xQueueSendFromISR( xQueueMotor, &tx, pxHigherPriorityTaskWoken);
    }
}

void initOCs()
{
    DRV_OC0_Initialize();
    DRV_OC0_Enable();
    DRV_OC0_Start();
    DRV_OC1_Initialize();
    DRV_OC1_Enable();
    DRV_OC1_Start();
}

void initMotor()
{
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_C, PORTS_BIT_POS_14); //enable setting right motor direction 
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_1); //enable setting left motor direction
    
    LM.motorID = LEFT;
    RM.motorID = RIGHT;
    currTime = 0;
    firstTime = true;
    sampleHasBeenTaken = false;
    Lpar.propParam = 50;
    Lpar.derivParam = 30;
    Lpar.integParam = 30;
    Lpar.integState = 0;
    Lpar.lastError = 0;
    Rpar.propParam = 50;
    Rpar.derivParam = 30;
    Rpar.integParam = 30;
    Rpar.integState = 0;
    Rpar.lastError = 0;
    
    motorsStop();
//    motorsForward();
//    motorsRight();
}

void motorsForward()
{
    setLeftMotor(FORWARD, 80); //78
    setRightMotor(FORWARD, 70); //70
}

void motorsReverse()
{
    setLeftMotor(REVERSE, 50);
    setRightMotor(REVERSE, 50);
}

void motorsRight()
{
    setLeftMotor(FORWARD, 73); //100 
    setRightMotor(REVERSE, 70); //96
}

void motorsLeft()
{
    setLeftMotor(REVERSE, 75);
    setRightMotor(FORWARD, 72);
}

void motorsStop()
{
    setLeftMotor(REVERSE, 0);
    setRightMotor(FORWARD, 0);
}

void moveForwardCm(int cm)
{
    resetEncoderTicks('a');
    double trueCm = cm * 2.5;
    static float temp = 0;
    temp = TICKSPERCM * trueCm;
    tickLCap = ROUND_2_INT(temp);
    tickRCap = tickLCap;
    motorsForward();
}

void moveForwardInch(int inch)
{
    resetEncoderTicks('a');
    static float temp = 0;
    temp = TICKSPERINCH * inch;
    tickLCap = ROUND_2_INT(temp);
    tickRCap = tickLCap;
    motorsForward();
}

void turnRight90()
{
    resetEncoderTicks('c');
    motorsRight();
    tickLCap = 660; //670
    tickRCap = 660; //670
}

void turnLeft90()
{
    resetEncoderTicks('c');
    motorsLeft();
    tickLCap = 640;
    tickRCap = 640;
}

void turnRight180()
{
    resetEncoderTicks('c');
    motorsRight();
    tickLCap = 670 * 2;
    tickRCap = 670 * 2;
}