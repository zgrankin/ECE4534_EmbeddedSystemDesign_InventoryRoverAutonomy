#ifndef WIFI_RECEIVE_THREAD_H
#define WIFI_RECEIVE_THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "system_config.h"
#include "system_definitions.h"
#include "jsmn.h"
#include "string.h"
#include "navigation_public.h"
#include "bookdetection_public.h"

void WIFI_RECEIVE_THREAD_Initialize ( void );
void WIFI_RECEIVE_THREAD_Tasks( void );

typedef struct {
    char data;
} WifiReceiveMessage;
void createWifiReceiveQueue(void);
WifiReceiveMessage readFromWifiReceiveQueue(void);

void executeMessage(char* message);
bool json_tokenize(char *js, char *result);
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
int simpleParse(char* message, char* result);
Message_Nav pathParse(char* message, char* result);
Message_Nav bookLocationParse(char* message, char* result);
Message_Nav roverOneDoneParse(char* message, char* result);


#endif