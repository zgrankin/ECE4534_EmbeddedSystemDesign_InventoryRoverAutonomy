#ifndef WIFI_SEND_THREAD_PUBLIC_H
#define WIFI_SEND_THREAD_PUBLIC_H

#define MESSAGE_BEGIN   '~'
#define MESSAGE_END     '%'
#define MAX_PAYLOAD_LENGTH  256

void sendJSONToServer(char* payload);
void sendJSONToServerFromISR(char* payload);

#endif