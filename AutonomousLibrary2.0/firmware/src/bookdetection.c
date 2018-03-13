#include "bookdetection.h"
#include "bookdetection_public.h"
#include "debug.h"
#include "queue.h"
#include "wifi_send_thread_public.h"
#include "wifi_receive_thread_public.h"
#include "soft_timer.h"
#include "navigation_public.h"

BOOKDETECTION_DATA bookdetectionData;

void BOOKDETECTION_Initialize ( void )
{
    createQueueDetection();
    dbgInit();
}

void BOOKDETECTION_Tasks ( void )
{
    Message_Detection rx;
    Message_Nav txNav;
    char json[256];
    while (1) 
    {
        if (isFinished())
            while (1)
            {
                ;
            }
        
        static unsigned int x = 0;
        static unsigned int y = 0;
        static unsigned int updateMapIndicator = 0;
        static int fullMap[40][40] = {0}, *p = fullMap;
        rx = readFromQueueDetection();
        
        if ( rx.type == 420)
        {
            sprintf(json, "{\"Type\": \"Request\", \"Request\": {\"map\": {\"xcoord\": %i, \"ycoord\": %i}}}", x, y);
            sendJSONToServer(json);
            y++;
            if (y >= 8)
            {
                x++;
                y = 0;
                if (x >= 40)
                    x = 0;
            }
            
            updateMapIndicator++;  
        }
        else
        {
            fullMap[0][0] = -10;
            fullMap[rx.map0.coordX][rx.map0.coordY] = rx.map0.conf;
            fullMap[rx.map1.coordX][rx.map1.coordY] = rx.map1.conf;
            fullMap[rx.map2.coordX][rx.map2.coordY] = rx.map2.conf;
            fullMap[rx.map3.coordX][rx.map3.coordY] = rx.map3.conf;
            fullMap[rx.map4.coordX][rx.map4.coordY] = rx.map4.conf; 
        }
        
        if (updateMapIndicator == 320)
        {
            updateMapIndicator = 0;
            txNav.map = p;
            txNav.type = 1;
            sendToQueueNav(txNav);
        }   
    }
}

QueueHandle_t xQueueDetection;

void createQueueDetection( void )
{
    xQueueDetection = xQueueCreate(200, sizeof(Message_Detection));
    if (xQueueDetection == 0)
    {
        dbgHalt();
    }
}

Message_Detection readFromQueueDetection( void )
{
    Message_Detection rxMessage;
    xQueueReceive(xQueueDetection, &rxMessage, portMAX_DELAY);
    return rxMessage;
}

void sendToQueueDetection( Message_Detection tx )
{
    xQueueSend( xQueueDetection, &tx, portMAX_DELAY);   
}

void sendToQueueFromISRDetection( Message_Detection tx, BaseType_t pxHigherPriorityTaskWoken )
{
    if (xQueueDetection == 0)
    {
        dbgHalt();
    }
    else
    {
        xQueueSendFromISR( xQueueDetection, &tx, pxHigherPriorityTaskWoken);
    }
}