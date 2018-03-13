#include "wifi_receive_thread.h"
#include "wifi_receive_thread_public.h"
#include "wifi_send_thread_public.h"
#include "debug.h"
#include <ctype.h>
#define MAX_PAYLOAD_LENGTH_RECEIVE 325

void WIFI_RECEIVE_THREAD_Initialize(void) {
    createWifiReceiveQueue();
}

void WIFI_RECEIVE_THREAD_Tasks(void) {
    bool good = false;
    int charIndex = 0;
    char message[MAX_PAYLOAD_LENGTH_RECEIVE];
    
    int index = -1;
    int missed = 0;
    int wrong = 0;
        
    while(1) {
        char data = readFromWifiReceiveQueue().data;
        if (good) {
            if (data == MESSAGE_END) {
                if (charIndex < MAX_PAYLOAD_LENGTH_RECEIVE)
                    message[charIndex] = 0;
                // Decode message ID
                int i;
                char temp;
                int thisIndex = 0;
                for (i = 0; i < 8; i++) {
                    temp = message[i] - '0';
                    if (temp >= 10)
                        temp -= 7;
                    thisIndex += temp << ((7-i) * 4);
                }   
                
                // Decode payload length
                int thisLength = 0;
                for (i = 0; i < 4; i++) {
                    temp = message[i + 8] - '0';
                    if (temp >= 10)
                        temp -= 7;
                    thisLength += temp << ((3-i) * 4);
                } 
                
                // Determine if the message is good
                if (1) { //thisLength == charIndex - 12
                    if (thisIndex - index > 1)
                        missed += thisIndex - index - 1;
                    index = thisIndex;
                    // Okay this is sketchy but I don't want the first 12 bytes of this string
                    executeMessage(message + 12 * sizeof(char));
                } else {
                    wrong++;
                }
                good = false;
                charIndex = 0;
            } 
            else if (data == MESSAGE_BEGIN) {
                // If we receive a begin byte in the middle of a message, scrap the old message
                wrong++;
                good = true;
                charIndex = 0;
            }
            else {
                if (charIndex >= MAX_PAYLOAD_LENGTH_RECEIVE) {
                    charIndex = charIndex;
                    good = false;
                    charIndex = 0;
                } 
                else {
                    message[charIndex] = data;
                    charIndex++;
                }
            }
        }
        else {
            if (data == MESSAGE_BEGIN) {
                good = true;
                charIndex = 0;
            }
        }
    }
}

QueueHandle_t wifiReceiveQueue;
void createWifiReceiveQueue(void) {
    wifiReceiveQueue = xQueueCreate(2000, sizeof(WifiReceiveMessage));
    if (wifiReceiveQueue == 0) {
        dbgHalt();
    }
}

void writeToWifiReceiveQueue(char data) {
    WifiReceiveMessage txMessage;
    txMessage.data = data;
    xQueueSendFromISR(wifiReceiveQueue, &txMessage, NULL);   
}

WifiReceiveMessage readFromWifiReceiveQueue(void) {
    WifiReceiveMessage rxMessage;
    xQueueReceive(wifiReceiveQueue, &rxMessage, portMAX_DELAY);
    return rxMessage;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

// This is where you should do what you need to with received server payloads
void executeMessage(char* message) 
{
    char *result;
    int a;
//    if ( simpleParse(message, result)) //json_tokenize(message, result) )
//    {
    if(strstr(message, "'testcase':") != NULL)
    {
        Message_Nav tx;
        a = simpleParse(message, result);
        tx.testcase = a;
        tx.type = 0;
        tx.dataUSFront = 4534;
        if (a != -1){
            sendToQueueNav(tx);
        }
    }
    else if (strstr(message, "BookDestination"))
    {
        Message_Nav tx;
        tx = pathParse(message, result);
        tx.type = 2;
        sendToQueueNav(tx);  
    }
    else if (strstr(message, "BookLocation"))
    {
        Message_Nav tx;
        tx = pathParse(message, result);
        tx.type = 5;
        sendToQueueNav(tx);  
    }
    else if (strstr(message, "RoverOneDone"))
    {
        Message_Nav tx;
        tx = roverOneDoneParse(message, result);
//        tx.type = 1000;
        sendToQueueNav(tx);
    }
}

int simpleParse(char* message, char* result)
{
    result = strstr(message, ":");
    if (result != NULL)
    {
        result = strstr(message, ":");
        result = &result[1];
        char temp[256];
        sprintf(temp, "%s", result);
        int a;
        a = temp[0] - '0';
        printf("%i", a);
        return a;
    }
    else
        return -1;
}

Message_Nav pathParse(char* message, char* result)
{
    Message_Nav point;
    char *p = message;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            point.loc.x = strtol(p, &p, 10); // Read a number, ...
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    result = strstr(message, ",");
    p = result;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            point.loc.y = strtol(p, &p, 10); // Read a number, ...
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    return point;
}

Message_Nav bookLocationParse(char* message, char* result)
{
    Message_Nav point;
    char *p = message;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            point.loc.x = strtol(p, &p, 10); // Read a number, ...
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    result = strstr(message, ",");
    p = result;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            point.loc.y = strtol(p, &p, 10); // Read a number, ...
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    return point;
}

Message_Nav roverOneDoneParse(char* message, char* result)
{
    Message_Nav done;
    char *p = message;
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            done.roverOneDone = strtol(p, &p, 10); // Read a number, ...
            break;
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    return done;
}

bool json_tokenize(char *js, char *result)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[256]; //no more than 256 tokens
    jsmn_init(&p);
    r = jsmn_parse(&p, js, strlen(js), t, sizeof(t)/sizeof(t[0]));
    if( r < 0 ) {
        return false;
    }
    
    /*Assume the top-level element is an object*/
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        return false;
    }
    
    for (i = 1; i < r; i++)
    {
        if (jsoneq(js, &t[i], "testcase") == 0) {
			if ( t[i+1].type == 4 )
            {
                strncpy(result, &js[t[i+1].start], t[i+1].end-t[i+1].start);
                //result[t[i+1].end-t[i+1].start] = '\0';
                return true;
            }
			i++;
		} 
        else if (jsoneq(js, &t[i], "map") == 0) {
			printf("- UID: %.*s\n", t[i+1].end-t[i+1].start,
					js + t[i+1].start);
			i++;
		} 
        else if (jsoneq(js, &t[i], "RetrievalCoords") == 0) {
			int j;
			printf("- Groups:\n");
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				printf("  * %.*s\n", g->end - g->start, js + g->start);
			}
			i += t[i+1].size + 1;
		} 
        else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
					js + t[i].start);
		}
    }
}