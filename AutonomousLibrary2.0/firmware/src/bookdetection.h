#ifndef _BOOKDETECTION_H
#define _BOOKDETECTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"

#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif

typedef enum
{
	BOOKDETECTION_STATE_INIT=0,
	BOOKDETECTION_STATE_SERVICE_TASKS,
} BOOKDETECTION_STATES;

typedef struct
{
    BOOKDETECTION_STATES state;

} BOOKDETECTION_DATA;

void BOOKDETECTION_Initialize ( void );
void BOOKDETECTION_Tasks( void );
void startUARTDetection( void );

#endif /* _BOOKDETECTION_H */

#ifdef __cplusplus
}
#endif

