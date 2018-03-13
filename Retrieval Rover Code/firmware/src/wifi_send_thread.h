#ifndef _WIFI_SEND_THREAD_H
#define _WIFI_SEND_THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "wifi_send_thread_public.h"

void WIFI_SEND_THREAD_Initialize ( void );
void WIFI_SEND_THREAD_Tasks( void );

void startUART(void);
void uartWriteByte(char);

typedef struct {
    char data[MAX_PAYLOAD_LENGTH];
} WifiSendMessage;
void createWifiSendQueue(void);
WifiSendMessage readFromWifiSendQueue(void);

#endif