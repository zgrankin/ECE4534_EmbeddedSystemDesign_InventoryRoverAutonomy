#include "navigation.h"
#include "navigation_public.h"
#include "debug.h"
#include "soft_timer.h"
#include "queue.h"
#include "wifi_send_thread_public.h"
#include "wifi_receive_thread_public.h"
#include "motorThread_public.h"

#define M_PI 3.14159265358979323846


enum Movement {REQUEST, TURN, MOVE, STOP}; 
enum Movement roverState = REQUEST;

NAVIGATION_DATA navigationData;

void NAVIGATION_Initialize ( void )
{
    createQueueNav();
    dbgInit();
    timerCreation();
    initTimers();
    initUltrasonics(); 
}

void NAVIGATION_Tasks ( void )
{
    int testNum;
    Message_Nav rx;
    Message_Nav curLoc;
    Turn makeTurn;
    char json[256];
    int newLocX = 4;
    int newLocY = 4;
    int destinationX = 0;
    int destinationY = 0; //intermediate destinations
    while (1) 
    {
        rx = readFromQueueNav();
        // sprintf(json, "{\"Type\": \"Request\", \"Request\": %s}", rx.testcase);
        
        curLoc = getUSTx();
        if (rx.type == 0)
        {
            sprintf(json, "{\"Type\": \"Store\", \"USRetrieval\":%i, \"RoverLoc2\":[%i,%i]}", 
                    rx.dataUSFront, curLoc.loc.x, curLoc.loc.y);
            sendJSONToServer(json);
        }  
        
        static bool retrievalRoverReady = false;
        if (!retrievalRoverReady)
        {
            sprintf(json, "{\"Type\": \"Request\", \"Request\": \"RoverOneDone\"}");
            sendJSONToServer(json);
        }
        
        static bool weMovinBoi = false;
        if (rx.roverOneDone == 1 && !weMovinBoi)
        {
            moveForward(20);
            weMovinBoi = true;
        }
        else if (rx.type == 3 && retrievalRoverReady == false)
        {
            retrievalRoverReady = true;
        }
        

        static bool atNewLoc = true;
        static bool hasSent = false;
        static unsigned int alternate = 0;
        static Coordinate bookCoord;
        bookCoord.x = 0;
        bookCoord.y = 0;
        if (retrievalRoverReady)
            switch(roverState)
            {
                case REQUEST:
                {
                    // wifi-receive thread will get requested Destination Request from soft timer and supply a message with rx.type == 2
                    if (rx.type == 2 && 
                            ((rx.loc.x == curLoc.loc.x && rx.loc.y != curLoc.loc.y) || (rx.loc.x != curLoc.loc.x && rx.loc.y == curLoc.loc.y))
                            )
                        {
                        destinationX = rx.loc.x;
                        destinationY = rx.loc.y;
                        roverState = TURN;
                        hasSent = false;
                        sendJSONToServer("{\"Type:\" \"Store\", \"Message\": \"Leaving Request\"}");
                    }
                    break;
                }
                case TURN:
                {
                    if (!hasSent)
                    {
                        makeTurn = determineTurn(curLoc.loc.direction, curLoc.loc.x, curLoc.loc.y, destinationX, destinationY);
                        if (makeTurn.direction == 'l')
                            turnLeft(makeTurn.turnDegrees); // this will send to the motor queue
                        else if (makeTurn.direction == 'r')
                            turnRight(makeTurn.turnDegrees); // this will send to the motor queue
                        hasSent = true;
                    }
                    else if (rx.type == 3)
                    {
                        double radians;
                        radians = makeTurn.turnDegrees * (M_PI / 180);
                        if (makeTurn.direction == 'r' )
                            radians = radians * -1;
                        setDirection(radians);
                        hasSent = false;
                        roverState = MOVE;
                        sendJSONToServer("{\"Type:\" \"Store\", \"Message\": \"Leaving Turn\"}");
                    }
                    break;
                }
                case MOVE:
                {
                    static int cm = 0;
                    if(!hasSent)
                    {
                        hasSent = true;
                        cm = determineDistanceForward(curLoc.loc.x, curLoc.loc.y, destinationX, destinationY);
                        moveForward(cm);
                    }
                    else if (rx.type == 3)
                    {
                        hasSent = false;
                        setLocation(cm);
                        roverState = REQUEST;
                        sprintf(json, "{\"Type:\" \"Store\", \"Message\": \"Leaving Hell Move to Request - Num: %i\"}", cm);
                        sendJSONToServer("{\"Type:\" \"Store\", \"Message\": \"Leaving Hell Move to Request\"}");
                    }
                    break;
                }
                case STOP:
                {
                    break; 
                }
            }
        
        
        if(rx.testcase > 0)
        {
            testNum = rx.testcase;
        }
    }
}

QueueHandle_t xQueueNav;
volatile int usCountFront = 0;
volatile Message_Nav txUS;
volatile bool trigger = true;

void createQueueNav( void )
{ 
    txUS.loc.x = 5;
    txUS.loc.y = 15;
    txUS.loc.direction = M_PI / 2;
    txUS.loc.directionInDegrees = 90;
    xQueueNav = xQueueCreate (200, sizeof(Message_Nav));
    if (xQueueNav == 0)
    {
        dbgHalt();
    }
}

Message_Nav readFromQueueNav( void )
{
    Message_Nav rxMessage;
    xQueueReceive(xQueueNav, &rxMessage, portMAX_DELAY);
    return rxMessage;
}

void sendToQueueNav( Message_Nav tx ) 
{
    xQueueSend( xQueueNav, &tx, portMAX_DELAY);   
}

void sendToQueueFromISRNav( Message_Nav tx )
{
    if (xQueueSendFromISR( xQueueNav, &tx, NULL)) {
        
    }
    else if (xQueueNav == 0) {
        dbgHalt();
    }
}

void incUSCount(char temp)
{
    if (temp == 'f')
        usCountFront++;       
}

int getUSCount(char temp)
{
    if (temp == 'f')
        return usCountFront;
}

void resetUSCount(char temp)
{
    if (temp == 'f')
        usCountFront = 0;
    else if (temp == 'a')
        usCountFront = 0;
}

void setTx(char temp, float num)
{
    txUS.type = 0;
    if (temp == 'f')
        txUS.dataUSFront = num;
    else if (temp == 't')
        txUS.type = 0;
}

Message_Nav getUSTx()
{
    return txUS;
}

bool getTriggerStatus()
{
    return trigger;
}

void setTriggerStatus(bool set)
{
    if (set == true)
        trigger = true;
    else if (set == false)
        trigger = false;
}

void setDirection(double radians)
{
    txUS.loc.direction += radians;
    if (txUS.loc.direction < 0)
    {
        txUS.loc.direction += 2 * M_PI;
    }
    else if (txUS.loc.direction > (2 * M_PI))
    {
        txUS.loc.direction -= 2 * M_PI;
    }
    
    txUS.loc.directionInDegrees = ROUND_2_INT(txUS.loc.direction * (180 / M_PI));
}

void setLocation(int cm)
{
    double theCos = ROUND_2_INT(cos(txUS.loc.direction));
    double theSin = ROUND_2_INT(sin(txUS.loc.direction));
    
    txUS.loc.x += (cm * theCos);
    txUS.loc.y += (cm * theSin);
}

void turnRight(int degrees)
{
    Message_Motor temp;
    temp.state = 1;
    temp.turnDegrees = degrees;
    temp.turnDirection = 'r';
    sendToQueueMotor(temp);
}

void turnLeft(int degrees)
{
    Message_Motor temp;
    temp.state = 1;
    temp.turnDegrees = degrees;
    temp.turnDirection = 'l';
    sendToQueueMotor(temp);
}

void moveForward(int cm)
{
    Message_Motor temp;
    temp.state = 2;
    temp.distanceToMove = cm;
    sendToQueueMotor(temp); 
}

Turn determineTurn(double curDirection, int curLocX, int curLocY, int destLocX, int destLocY)
{
    Turn result, left, right;
    left.direction = 'l';
    right.direction = 'r';
    if ((curLocX == destLocX) && (destLocY > curLocY))
    {
        // this means need to go north
        left.turnDegrees = 90 - (curDirection * (180 / M_PI)); // would return 90 for my example
        right.turnDegrees = curDirection * (180 / M_PI) - 90;;
        if (left.turnDegrees < 0)
            left.turnDegrees += 360;
        if (right.turnDegrees < 0)
            right.turnDegrees += 360;
        result = left.turnDegrees <= right.turnDegrees ? left : right; 
    }
    else if ((curLocX == destLocX) && (destLocY < curLocY))
    {
        // this means need to go south
        left.turnDegrees = 270 - curDirection * (180 / M_PI);
        right.turnDegrees = curDirection * (180 / M_PI) - 270;
        if (left.turnDegrees < 0)
            left.turnDegrees += 360;
        if (right.turnDegrees < 0)
            right.turnDegrees += 360;
        result = left.turnDegrees <= right.turnDegrees ? left : right; 
    }
    else if ((curLocY == destLocY) && (destLocX > curLocX))
    {
        // this means need to go right
        left.turnDegrees = 0 - curDirection * (180 / M_PI);
        right.turnDegrees = curDirection * (180 / M_PI) - 0;
        if (left.turnDegrees < 0)
            left.turnDegrees += 360;
        if (right.turnDegrees < 0)
            right.turnDegrees += 360;
        result = left.turnDegrees <= right.turnDegrees ? left : right; 
    }
    else if ((curLocY == destLocY) && (destLocX < curLocX))
    {
        // this means need to go left
        left.turnDegrees = 180 - curDirection * (180 / M_PI);
        right.turnDegrees = curDirection * (180 / M_PI) - 180;
        if (left.turnDegrees < 0)
            left.turnDegrees += 360;
        if (right.turnDegrees < 0)
            right.turnDegrees += 360;
        result = left.turnDegrees <= right.turnDegrees ? left : right; 
    }
    return result;
}

int determineDistanceForward(int curLocX, int curLocY, int destLocX, int destLocY)
{
    int yDiff = destLocY - curLocY;
    int xDiff = destLocX - curLocX;
    if (curLocX == destLocX)
    {
        uint32_t temp = yDiff >> 31;     // make a mask of the sign bit
        yDiff ^= temp;                   // toggle the bits if value is negative
        yDiff += temp & 1;               // add one if value was negative
        return yDiff;
    }
    else if (curLocY == destLocY)
    {
        uint32_t temp = xDiff >> 31;     // make a mask of the sign bit
        xDiff ^= temp;                   // toggle the bits if value is negative
        xDiff += temp & 1;               // add one if value was negative
        return xDiff; 
    }
}

Coordinate determineNextLocation(Message_Nav point)
{
    char json[256];
    Coordinate coord;
    coord.x = point.loc.x;
    coord.y = point.loc.y;
    return coord;
//    static int iterations = 0;
//    Coordinate coord;
//    coord.done = false;
//    if (iterations == 0)
//    {
//        coord.x = 4;
//        coord.y = 35;
//        iterations++;
//    }
//    else if (iterations == 1)
//    {
//        coord.x = 35;
//        coord.y = 35;
//        iterations++;
//    }
//    else if (iterations == 2)
//    {
//        coord.x = 35;
//        coord.y = 4;
//        iterations++;
//    }
//    else if (iterations == 3)
//    {
//        coord.x = 4;
//        coord.y = 4;
//        iterations++;
//    }
}

void initUltrasonics( void )
{
    PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_7);
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8);
    PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_8);
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_11);
    PLIB_PORTS_PinDirectionInputSet(PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_1);
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_6);
}

void initTimers( void )
{
    DRV_TMR0_Initialize(); //RM encoder
    DRV_TMR0_Start();
    
    DRV_TMR1_Initialize();
    DRV_TMR1_Start();
    
    DRV_TMR2_Initialize();
    DRV_TMR2_Start();
    
    DRV_TMR3_Initialize(); //LM encoder
    DRV_TMR3_Start();
}