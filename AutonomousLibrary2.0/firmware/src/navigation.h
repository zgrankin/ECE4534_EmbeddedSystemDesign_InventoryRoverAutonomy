#ifndef _NAVIGATION_H
#define _NAVIGATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "system_config.h"
#include "system_definitions.h"

#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

typedef enum
{
	NAVIGATION_STATE_INIT=0,
	NAVIGATION_STATE_SERVICE_TASKS,
} NAVIGATION_STATES;

typedef struct
{
    NAVIGATION_STATES state;


} NAVIGATION_DATA;

void NAVIGATION_Initialize ( void );
void NAVIGATION_Tasks( void );
void initTimers();
void initUltrasonics( void );
void startAdc( void );




#endif /* _NAVIGATION_H */

#ifdef __cplusplus
}
#endif