#ifndef _MOTORTHREAD_H
#define _MOTORTHREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "system_config.h"
#include "system_definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
typedef enum
{
	MOTORTHREAD_STATE_INIT=0,
	MOTORTHREAD_STATE_SERVICE_TASKS,
} MOTORTHREAD_STATES;
typedef struct
{
    MOTORTHREAD_STATES state;
} MOTORTHREAD_DATA;


typedef struct{
    float derivParam;
    float integParam;
    float propParam;
    float integState;
    float lastError;
}PIDparams;

PIDparams Rpar;
PIDparams Lpar;

bool firstTime;

void MOTORTHREAD_Initialize ( void );
void MOTORTHREAD_Tasks( void );
void initOCs();
void initMotor();
int PIDdutyCycle(float setSpeed, float currSpeed, PIDparams* params);
void setLeftMotor(unsigned int direction, unsigned int speed);
void setRightMotor(unsigned int direction, unsigned int speed);
void motorsForward();
void motorsReverse();
void motorsRight();
void motorsLeft();
void turnLeft90();
void turnRight90();
void turnRight180();
void moveForwardCm(int cm);
void moveForwardInch(int inch);



#endif /* _MOTORTHREAD_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif

