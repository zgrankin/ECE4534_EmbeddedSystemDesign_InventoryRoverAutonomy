#include "debug.h"

void dbgInit(void) {
    // Val Pins
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8);
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_7);
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_11);
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_8);
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_6);
//    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_1);
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_0);
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_10);
    
    // Loc Pins
    PLIB_PORTS_DirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_E, 0xFF);
    
    // LED
    PLIB_PORTS_PinDirectionOutputSet(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_3);
}

void dbgOutputVal(unsigned char outVal) {
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8, outVal & 1); // Bit 0
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_7, outVal & 1); // Bit 1
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_11, outVal & 1); // Bit 2
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_8, outVal & 1); // Bit 3
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_D, PORTS_BIT_POS_6, outVal & 1); // Bit 4
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_1, outVal & 1); // Bit 5
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_0, outVal & 1); // Bit 6
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_10, outVal & 1); // Bit 7
}

void dbgOutputLoc(unsigned char outVal) {
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_0, outVal & 1); // Bit 0
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_1, outVal & 1); // Bit 1
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_2, outVal & 1); // Bit 2
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_3, outVal & 1); // Bit 3
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_4, outVal & 1); // Bit 4
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_5, outVal & 1); // Bit 5
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_6, outVal & 1); // Bit 6
    outVal = outVal >> 1;
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_E, PORTS_BIT_POS_7, outVal & 1); // Bit 7
}

void dbgUARTVal ( unsigned char outVal )
{
    DRV_USART0_WriteByte( outVal );
}

void dbgHalt( void ) 
{
    vTaskSuspendAll();
    taskDISABLE_INTERRUPTS();
    toggleLED();
    while(1)
    {
        dbgOutputLoc(LOC_HALT);
    }
}

void toggleLED( void )
{
    PLIB_PORTS_PinToggle(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_3);
}

void toggleLEDOn( void )
{
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_3, 1);
}

void toggleLEDOff( void )
{
    PLIB_PORTS_PinWrite(PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_3, 0);
}