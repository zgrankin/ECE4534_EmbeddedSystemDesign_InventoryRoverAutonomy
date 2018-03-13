#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"

#define LOC_ENTER_TASK      0
#define LOC_ENTER_WHILE     1
#define LOC_PRE_SEND_Q      2
#define LOC_PRE_RECEIVE_Q   3
#define LOC_POST_SEND_Q     4
#define LOC_POST_RECEIVE_Q  5
#define LOC_ENTER_ISR       6
#define LOC_LEAVE_ISR       7
#define LOC_PRE_SEND_Q_ISR      0xa
#define LOC_PRE_RECEIVE_Q_ISR   0xb
#define LOC_POST_SEND_Q_ISR     0xc
#define LOC_POST_RECEIVE_Q_ISR  0xd
#define LOC_HALT                0xf

void dbgInit(void);
void dbgOutputVal(unsigned char outVal);
void dbgOutputLoc(unsigned char outVal);
void toggleLED( void );
void toggleLEDOn( void );
void toggleLEDOff( void );
void dbgUARTVal( unsigned char outVal);
void dbgHalt(void);

#endif