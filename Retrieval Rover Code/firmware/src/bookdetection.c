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

}

void BOOKDETECTION_Tasks ( void )
{
    while (1) 
    {
  
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