#ifndef _MOTOR_PUBLIC_H    
#define _MOTOR_PUBLIC_H

#define TIMER1_SAMPLING_FREQ 15.0
#define FORWARD 0
#define REVERSE 1
#define LEFT 0
#define RIGHT 1

#define TICKSPERINCH 186.0465116
#define TICKSPERCM 73.24665812

typedef struct {
    int motorID;
    float speed;
    int direction;
} MotorState;

typedef struct {
    int state; // 1 goes with turns
    // 2 goes with moving forward
    int turnDegrees;
    char turnDirection;
    int distanceToMove;
} Message_Motor;

MotorState LM;
MotorState RM;
bool sampleHasBeenTaken;
float currTime;


void createQueueMotor( void );
Message_Motor readFromQueueMotor( void );
void sendToQueueMotor( Message_Motor tx );
void sendToQueueFromISRMotor( Message_Motor tx, BaseType_t pxHigherPriorityTaskWoken );
void incTotalEncoderTicksLeft();
void incTotalEncoderTicksRight();
void incEncoderTicksLeft();
void incEncoderTicksRight();
void resetEncoderTicks(char choose);
int returnTotalEncoderTicks(char choose);
int returnLocalEncoderTicks(char choose);
int returnCapEncoderTicks(char choose);
void motorsStop();
void setReady(char choose, int set);
bool isReady();

void incSecondCount();
unsigned int getSecondCount();
void resetSecondCount();



#endif /* _EXAMPLE_FILE_NAME_H */
