#ifndef _BOOK_DETECTION_PUBLIC_H
#define _BOOK_DETECTION_PUBLIC_H

typedef struct {
    unsigned int coordX;
    unsigned int coordY;
    int conf;
} Map;

typedef struct {
    int type;
    Map map0;
    Map map1;
    Map map2;
    Map map3;
    Map map4;
} Message_Detection;

void createQueueDetection( void );
Message_Detection readFromQueueDetection( void );
void sendToQueueDetection( Message_Detection tx );
void sendToQueueFromISRDetection( Message_Detection tx, BaseType_t pxHigherPriorityTaskWoken );

#endif