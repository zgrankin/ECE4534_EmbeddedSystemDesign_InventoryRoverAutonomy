#ifndef _NAVIGATION_PUBLIC_H
#define _NAVIGATION_PUBLIC_H

#define ECHO_CHANNEL_FRONT PORT_CHANNEL_G
#define ECHO_BIT_FRONT PORTS_BIT_POS_7
#define ECHO_CHANNEL_RIGHT PORT_CHANNEL_D
#define ECHO_BIT_RIGHT PORTS_BIT_POS_8
#define ECHO_CHANNEL_LEFT PORT_CHANNEL_F
#define ECHO_BIT_LEFT PORTS_BIT_POS_1

#define TRIGGER_CHANNEL_FRONT PORT_CHANNEL_G
#define TRIGGER_BIT_FRONT PORTS_BIT_POS_8
#define TRIGGER_CHANNEL_RIGHT PORT_CHANNEL_D
#define TRIGGER_BIT_RIGHT PORTS_BIT_POS_11
#define TRIGGER_CHANNEL_LEFT PORT_CHANNEL_D
#define TRIGGER_BIT_LEFT PORTS_BIT_POS_6

#define ROUND_2_INT(f) ((int)(f >= 0.0 ? (f + 0.5) : (f - 0.5)))

typedef struct {
    int x;
    int y;
    bool done;
} Coordinate;

typedef struct {
    int x;
    int y;
    double direction;
    int directionInDegrees;
} RoverLocation;

typedef struct{
    int turnDegrees;
    char direction;
} Turn;

typedef struct {
    unsigned int type; // type = 0 means getting ultrasonic data and sending it up
    // type = 1 means we're getting map data so store that jazz
    // type = 2 means we received a destination from the server
    // type = 3 means the motor is ready for a new direction
    unsigned int dataUSFront;
    unsigned int dataUSRight;
    unsigned int dataUSLeft;
    RoverLocation loc; 
    int testcase;
    int* map;
    unsigned int tickLeft;
    unsigned int tickRight;
    
} Message_Nav;

void createQueueNav( void );
Message_Nav readFromQueueNav( void );
void sendToQueueNav( Message_Nav tx );
void sendToQueueFromISRNav( Message_Nav tx );

bool getTriggerStatus();
void setTriggerStatus(bool set);
void incUSCount(char temp);
int getUSCount(char temp);
void resetUSCount(char temp);
void setTx(char temp, float distance);
void setDirection(double radians);
void setLocation(int cm);
Message_Nav getUSTx();

void moveForward(int cm);
void turnRight(int degrees);
void turnLeft(int degrees);
Turn determineTurn(double curDirection, int curLocX, int curLocY, int destLocX, int destLocY);
int determineDistanceForward(int curLocX, int curLocY, int destLocX, int destLocY);
Coordinate determineNextLocation(int *theMap);

bool isFinished();



#endif