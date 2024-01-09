#include <sys/stat.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>


typedef struct  {
        int buf_stdin_max_len;
        int buf_send_max_len;
        int buf_recv_max_len;
        
        char * buffer_stdin;
        char * buffer_send;
        char * buffer_recv;
} peer_buffer;

#define INFO_PDU_TYPE_VALUE 0
struct InfoPDU {
   unsigned char pduType;		// =0
};

#define ECHO_PDU_TYPE_VALUE 1
struct EchoPDU {
   unsigned char pduType;		// =1
   unsigned int sum;
};

/*
 * send strings with EOS
 * count EOS in length
 */
#define MSG_MAX_LENGTH 255
#define LOG_PDU_TYPE_VALUE 2
struct LogPDU {           
    unsigned char pduType;      //= 2
    unsigned char stringlength;
    char msg[MSG_MAX_LENGTH];
};

#define START_PDU_TYPE_VALUE 3
struct StartPDU { 
     unsigned char pduType;      //= 3
};

#define RESULT_PDU_TYPE_VALUE 4
struct ResultPDU {
    unsigned char pduType;      //= 4
    unsigned int sum;
};

int packStartPDU(char* buf, int sum);
int packInfoPDU(char* buf, int sum);
int packEchoPDU(char* buf, int sum);
int packResultPDU(char* buf, int sum);

int packLogPDU(char* buf, short portFrom, short portTo, char pduType, int sum);

char* getType(int t);

void unpackEchoPDU(char* buf, int* sum);
void unpackLogPDU(char* buf, char** data);
void unpackResultPDU(char* buf, int* sum);


int notifyPeers(
    int socket_fd, short myPort, struct sockaddr_in* loggerAddr, peer_buffer * pb,
    struct sockaddr_in * list, int listSize, 
    struct sockaddr_in * notThisOne,
    char pdu_type, int sum
);
