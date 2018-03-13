#include "wifi_send_thread.h"
#include "wifi_send_thread_public.h"
#include "debug.h"

void WIFI_SEND_THREAD_Initialize(void) {
    startUART();
    createWifiSendQueue();
}

void WIFI_SEND_THREAD_Tasks(void) {
    int messageID = 0;
    while(1) {
        WifiSendMessage txMessage = readFromWifiSendQueue();
        
        // Determine this message's payload length in bytes
        int messageLength = 0;
        int i;
        for (i = 0; i < MAX_PAYLOAD_LENGTH; i++) {
            if (txMessage.data[i] == 0)
                break;
            messageLength++;
        }
        
        uartWriteByte(MESSAGE_BEGIN);
        
        // Write this message's ID
        char temp;
        for (i = 7; i >= 0; i--) {
            temp = (messageID >> (i * 4)) & 0x0F;
            temp += '0';
            if (temp >= ':')
                temp += 7;
            uartWriteByte(temp);
        }
        
        // Write the payload's length
        for (i = 3; i >= 0; i--) {
            temp = (messageLength >> (i * 4)) & 0x0F;
            temp += '0';
            if (temp >= ':')
                temp += 7;
            uartWriteByte(temp);
        }
        
        // Write the message's payload
        for (i = 0; i < MAX_PAYLOAD_LENGTH; i++) {
            if (txMessage.data[i] == 0)
                break;
            uartWriteByte(txMessage.data[i]);
        }
        uartWriteByte(MESSAGE_END);
        messageID++;
    }
}

void startUART(void)
{
    PLIB_USART_Disable (USART_ID_1);
    PLIB_USART_BaudSetAndEnable(USART_ID_1, SYS_CLK_PeripheralFrequencyGet ( CLK_BUS_PERIPHERAL_1 ), 57600);  /*Desired Baud rate value*/
    PLIB_USART_InitializeModeGeneral(USART_ID_1,
        false,  /*Auto baud*/
        false,  /*LoopBack mode*/
        true,  /*Auto wakeup on start*/
        false,  /*IRDA mode*/
        false);  /*Stop In Idle mode*/
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_TRANSMIT);
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_RECEIVE);
    SYS_INT_SourceStatusClear(INT_SOURCE_USART_1_ERROR);
    PLIB_INT_SourceEnable( INT_ID_0, INT_SOURCE_USART_1_TRANSMIT );
    PLIB_INT_SourceEnable( INT_ID_0, INT_SOURCE_USART_1_RECEIVE );
    PLIB_USART_Enable(USART_ID_1);
    while (PLIB_USART_ReceiverDataIsAvailable(USART_ID_1))
    {
        PLIB_USART_ReceiverByteReceive(USART_ID_1);
    }
}

void uartWriteByte(char txByte) {
    //TODO: CHANGE THIS TO WRITE A BYTE INTO A QUEUE, AND FOR THE UART INTERRUPT TO WRITE IT ON TRANSMIT
    while (DRV_USART0_TransmitBufferIsFull()) {
        // Block until a space opens up
    }
    DRV_USART0_WriteByte(txByte);
}

QueueHandle_t wifiSendQueue;
void createWifiSendQueue(void) { 
    wifiSendQueue = xQueueCreate(10, sizeof(WifiSendMessage));
    if (wifiSendQueue == 0) {
        dbgHalt();
    }
}

WifiSendMessage readFromWifiSendQueue(void) {
    WifiSendMessage rxMessage;
    xQueueReceive(wifiSendQueue, &rxMessage, portMAX_DELAY);
    return rxMessage;
}

void sendJSONToServer(char* payload) {
    WifiSendMessage txMessage;
    int i;
    for (i = 0; i < MAX_PAYLOAD_LENGTH; i++) {
        txMessage.data[i] = payload[i];
    }
    xQueueSend(wifiSendQueue, &txMessage, portMAX_DELAY);   
}

void sendJSONToServerFromISR(char* payload) {
    WifiSendMessage txMessage;
    int i;
    for (i = 0; i < MAX_PAYLOAD_LENGTH; i++) {
        txMessage.data[i] = payload[i];
    }
    xQueueSendFromISR(wifiSendQueue, &txMessage, NULL);
}