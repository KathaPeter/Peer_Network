#include "common.h"


int packInfoPDU(char* buf, int sum) {
    buf[0] = INFO_PDU_TYPE_VALUE;
    return 1;
}

int packEchoPDU(char* buf, int sum) {
    int* pi = (int*) ((char*)buf + 1);
    
    buf[0] = ECHO_PDU_TYPE_VALUE;
    *pi =  sum;
    
    return 5;
}

int packStartPDU(char* buf, int sum) {
    buf[0] = START_PDU_TYPE_VALUE;    
    return 1;
}


char* map_string[] = {"Info","Echo","Log","Start","Result"};

char * getType(int t) {
    return map_string[t];
}

int packLogPDU(char* buf, short portFrom, short portTo, char pduType, int sum) {
    unsigned char len;
    
    if ( pduType == ECHO_PDU_TYPE_VALUE || pduType == RESULT_PDU_TYPE_VALUE) {
        sprintf(buf+2, "From: %5d To: %5d Type: %.6s Sum: %12d", portFrom, portTo, map_string[pduType], sum);
    } else {
        sprintf(buf+2, "From: %5d To: %5d Type: %.6s Sum: --", portFrom, portTo, map_string[pduType]);
    }
    
    len = strlen(buf + 2) + 1;//+1 cause EOS
    
    buf[0] = LOG_PDU_TYPE_VALUE;
    buf[1] = len;
    
    return 2 + len;
}

int packResultPDU(char* buf, int sum) {
    int* pi = (int*) ((char*)(buf + 1));
    buf[0] = RESULT_PDU_TYPE_VALUE;
    *pi = sum;
    
    return 5;
}

void unpackEchoPDU(char* buf, int* sum) {
    int *pi = (int*) ((char*)(buf + 1));
    *sum = *pi;
}

void unpackLogPDU(char* buf, char** data) {
    unsigned char len = buf[1];
    *data = buf + 2;
    data[len] = '\0';
}

void unpackResultPDU(char* buf, int* sum) {
      int* pi = (int*) ((char*)(buf + 1));
      *sum = *pi;
}


int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y) {
#define CMP(a, b) if (a != b) return a < b ? -1 : 1

    if ( x == NULL || y == NULL ) {
        if ( x == NULL && y == NULL ) {
            return 0;
        } else {
            CMP(x,y);
        }
    }
    
    
    CMP(x->sa_family, y->sa_family);

    /*if (x->sa_family == AF_UNIX) {
        struct sockaddr_un *xun = (void*)x, *yun = (void*)y;
        int r = strcmp(xun->sun_path, yun->sun_path);
        if (r != 0)
            return r;
    }else*/ 
    if (x->sa_family == AF_INET) {
        struct sockaddr_in *xin = (void*)x, *yin = (void*)y;
        CMP(ntohl(xin->sin_addr.s_addr), ntohl(yin->sin_addr.s_addr));
        CMP(ntohs(xin->sin_port), ntohs(yin->sin_port));
    } else if (x->sa_family == AF_INET6) {
        struct sockaddr_in6 *xin6 = (void*)x, *yin6 = (void*)y;
        int r = memcmp(xin6->sin6_addr.s6_addr, yin6->sin6_addr.s6_addr, sizeof(xin6->sin6_addr.s6_addr));
        if (r != 0)
            return r;
        CMP(ntohs(xin6->sin6_port), ntohs(yin6->sin6_port));
        CMP(xin6->sin6_flowinfo, yin6->sin6_flowinfo);
        CMP(xin6->sin6_scope_id, yin6->sin6_scope_id);
    } else {
        printf("unknown sa_family");
    }

#undef CMP
    return 0;
}

int(*fnArray[5])(char*,int) = {packInfoPDU, packEchoPDU, NULL, packStartPDU, packResultPDU};

int notifyPeers(
    int socket_fd, short myPort, struct sockaddr_in* loggerAddr, peer_buffer * pb,
    struct sockaddr_in * list, int listSize, 
    struct sockaddr_in * notThisOne,
    char pdu_type, int sum
) 
{
    int pdulen;
    char loggerPDU[300];
    int loggerPDULen;
    short otherPort;
    int ret;
    struct sockaddr_in * currentPeer;
    
    //pack PDU
    pdulen = fnArray[pdu_type](pb->buffer_send, sum);
    
    //printf("NOTIFY..\n");
    //printf("[SENDING %d BYTES]..\n\n", pdulen);
    
    for(int i = 0; i < listSize; i++){
        //iterrate peers
        currentPeer = list + i;
        
        if(sockaddr_cmp((struct sockaddr*) currentPeer, (struct sockaddr*) notThisOne) != 0 ) {
            
            
            otherPort = ntohs(currentPeer->sin_port);
            
            /*
             * send copy to logger
             */
            
           
            
            //pack LoggerPDU
            loggerPDULen = packLogPDU(loggerPDU, myPort, otherPort, pdu_type, sum);
            //printf("'Copy' to: <logger>\n");
            
            
            
            //send to logger
            ret = sendto(socket_fd, loggerPDU, loggerPDULen, 0, (struct sockaddr*) (loggerAddr), 
                         sizeof(struct sockaddr_in));
            if(ret == (-1)){
                perror("Error: send PDU to logger \n");
                return -1;
            }
            
            
            /*
             * send PDU to peer
             */
            
            //printf("Sending to: %hd\n", otherPort);
            
            //send to ip:port
            ret = sendto(socket_fd, pb->buffer_send, pdulen, 0, (struct sockaddr*) (currentPeer), 
                         sizeof(struct sockaddr_in));
            if(ret == (-1)){
                perror("Error: send PDU to peer \n");
                return -1;
            }
            
            
         
            
            
            //printf("DoneSending to: %hd\n", otherPort);
            
        }
    }
    
    //printf("Done NOTIFY\n");
    
    return 0;
}





