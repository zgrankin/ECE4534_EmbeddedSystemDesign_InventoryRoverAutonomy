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
    startAdc();
    initUltrasonics(); 
}

void NAVIGATION_Tasks ( void )
{
    int testNum;
    int *theMap;
    Message_Nav rx;
    Message_Nav curLoc;
    Turn makeTurn;
    char json[256];
    int newLocX = 5;
    int newLocY = 5;
    Coordinate coordHold;
    int destinationX = 0;
    int destinationY = 0; //intermediate destinations
    while (1) 
    {
        rx = readFromQueueNav();
        
        if (isFinished())
            while (1)
            {
                ;
            }
        

        if (rx.type == 0)
        {
            sprintf(json, "{\"Type\": \"Store\", \"USFront\":%i, \"USRight\":%i, \"USLeft\":%i, \"RoverLoc\":[%i,%i], \"TicksLeft\":%i, \"TicksRight\":%i}", 
                    rx.dataUSFront, rx.dataUSRight, rx.dataUSLeft, rx.loc.x, rx.loc.y, rx.tickLeft, rx.tickRight);
            sendJSONToServer(json);
        }
        
        else if (rx.type == 1)
        {
            theMap = rx.map;
            int i = 0;
            int x = 0;
            int y = 0;
            int a;
            char temp[256];
//            for ( x = 0; x < 40; x++ ) 
//                for (y = 0; y < 40; y++)
//                {
//                    a = (int)*(rx.map + i);
//                    sprintf(json, "{\"Type\": \"Store\", \"currentmap\": 1, \"x\": %i, \"y\": %i, \"conf\": %i}", x, y, a);
//                    sendJSONToServer(json);
//                    i++;
//                }
        }
        

        static bool atNewLoc = true;
        static bool hasSent = false;
        curLoc = getUSTx();
        switch(roverState)
        {
            case REQUEST:
            {
                // atNewLoc = false;
                if (curLoc.loc.x == newLocX && curLoc.loc.y == newLocY)
                {
                    atNewLoc = true;
                }
                
                // i have reached the new location so decide on the next coordinate
                if (atNewLoc)
                {
                    coordHold = determineNextLocation(theMap);
                    if (coordHold.x == newLocX && coordHold.y == newLocY)
                        atNewLoc = true;
                    else {
                        atNewLoc = false;
                        newLocX = coordHold.x;
                        newLocY = coordHold.y;
                    }
                }
                
                // alright I know where to go next, request a path to get there
//                if (!hasSent) // curLoc.loc.x != newLocX || curLoc.loc.y != newLocY) && 
//                {
                    sprintf(json, "{\"Type\": \"Store\", \"PathRequest\": [%i, %i]}", newLocX, newLocY);
                    sendJSONToServer(json);
                    // software timer 4 is making the request for the path
                    hasSent = true;
//                }
                
                // wifi-receive thread will get requested PathRequest from soft timer and supply a message with rx.type == 2
                if (rx.type == 2 && 
                        ((rx.loc.x == curLoc.loc.x && rx.loc.y != curLoc.loc.y) || (rx.loc.x != curLoc.loc.x && rx.loc.y == curLoc.loc.y))
                        )
                {
                    destinationX = rx.loc.x;
                    destinationY = rx.loc.y;
                    roverState = TURN;
                    hasSent = false;
                    sendJSONToServer("{\"Type\": \"Store\", \"Message\": \"Leaving Request\"}");
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
                    sendJSONToServer("{\"Type\": \"Store\", \"Message\": \"Leaving Turn\"}");
                }
            }
                break;
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
                    if (coordHold.done == true)
                    {
                        roverState = STOP;
//                        sendJSONToServer("{\"Type\": \"Store\", \"Message\": \"Finished Mapping.\"}");
                    }
                    else
                    {
                        setLocation(cm);
                        roverState = REQUEST;
                        sendJSONToServer("{\"Type\": \"Store\", \"Message\": \"Leaving Hell Move to Request\"}");
                    }
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
volatile int usCountRight = 0;
volatile int usCountLeft = 0;
volatile Message_Nav txUS;
volatile bool trigger = true;
volatile bool doneWithEverything = false;

void createQueueNav( void )
{ 
    txUS.loc.x = 5;
    txUS.loc.y = 5;
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
    else if (temp == 'r')
        usCountRight++;
    else if (temp == 'l')
        usCountLeft++;        
}

int getUSCount(char temp)
{
    int result;
    if (temp == 'f')
        result = usCountFront;
    else if (temp == 'r')
        result = usCountRight;
    else if (temp == 'l')
        result = usCountLeft; 
    return result;
}

void resetUSCount(char temp)
{
    if (temp == 'f')
        usCountFront = 0;
    else if (temp == 'r')
        usCountRight = 0;
    else if (temp == 'l')
        usCountLeft = 0; 
    else if (temp == 'a')
    {
        usCountFront = 0;
        usCountRight = 0;
        usCountLeft = 0;
    }
}

void setTx(char temp, float num)
{
    txUS.type = 0;
    if (temp == 'f')
        txUS.dataUSFront = num;
    else if (temp == 'r')
        txUS.dataUSRight = num;
    else if (temp == 'l')
        txUS.dataUSLeft = num;
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

Coordinate determineNextLocation(int *theMap)
{
    static int iterations = 0;
    int numWithLowConf = 0;
    static unsigned int x, y, i;
    i = 0;
    int temp;
    static Coordinate coord;
    coord.done = false;
    
    
    if (iterations == 0)
    {
        coord.x = 5;
        coord.y = 34;
    }
    else if (iterations == 1)
    {
        coord.x = 34;
        coord.y = 34;
    }
    else if (iterations == 2)
    {
        coord.x = 34;
        coord.y = 5;
    }
    else if (iterations == 3)
    {
        coord.x = 5;
        coord.y = 5;
    }    
    else if (iterations == 4 || iterations == 5) //else {}
    {
        char json[256];
        sprintf(json, "{\"Type\": \"Store\", \"ZackNav\":%i}", 1);
        sendJSONToServer(json);
        bool done = false;
        for ( x = 0; x < 40; x++ ) 
            for (y = 0; y < 40; y++)
            {
                temp = (int)*(theMap + i);
//                sprintf(json, "{\"Type\": \"Store\", \"MyPoint\": [%i, %i, %i]}", x, y, temp);
//                sendJSONToServer(json);
                if (temp > -5 && temp < 5 && !(x==0 && y==0))
                {
                    
                    
                    if (((int)*(theMap + i + 160) < 0) && done == false)
                    {
                        done = true;
                        coord.x = x + 4;
                        coord.y = y;
                        coord.done = false;
                    }
                    else if (((int)*(theMap + i - 160) < 0) && done == false)
                    {
                        done = true;
                        coord.x = x - 4;
                        coord.y = y;
                        coord.done = false;
                    }
                    else if (((int)*(theMap + i + 4) < 0) && done == false)
                    {
                        done = true;
                        coord.x = x;
                        coord.y = y + 4;
                        coord.done = false;
                    }
                    else if (((int)*(theMap + i - 4) < 0) && done == false)
                    {
                        done = true;
                        coord.x = x;
                        coord.y = y - 4;
                        coord.done = false;
                    }                    
                    numWithLowConf++;
                }
                i++;
            }
            

//        if (numWithLowConf < 50)
//        {
//            coord.x = 35;
//            coord.y = 4;
//            coord.done = true;
//        }
        sendJSONToServer("{\"Type\": \"Store\", \"Message\": \"Leaving The Low Conf Find State\"}");
    }
    else if (iterations == 6)
    {
        coord.x = 38;
        coord.y = 37;
    }
    else if (iterations == 7)
    {
        moveForward(5);
        char json[256];
        sprintf(json, "{\"Type\": \"Store\", \"RoverOneDone\":%i}", 1);
        sendJSONToServer(json);
        doneWithEverything = true;
    }
    
    iterations++;
    return coord;
}

void startAdc( void )
{
    DRV_ADC_Initialize();
    DRV_ADC_Open();
    DRV_ADC_Start();
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

bool isFinished()
{
    return doneWithEverything;
}

// sprintf(json, "{\"Type\": \"Store\", \"Zack\":{\"IR\": %i, \"US\": %i}}", rx.dataIR, rx.dataUS);
