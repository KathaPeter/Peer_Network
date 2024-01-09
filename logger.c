
#include "logger.h"



int initPeer(logger_data * data, peer_buffer * pb);
int mainloop(logger_data * data, peer_buffer * pb);
int sumCMD(logger_data * data, peer_buffer * pb);
int receiveMessages(logger_data* data, peer_buffer * pb);


int main(void){
    
    logger_data data;
    peer_buffer buffer;
    int ret = -1;
    
    buffer.buf_stdin_max_len = STDIN_LIMIT;
    buffer.buf_send_max_len = 300;
    buffer.buf_recv_max_len = 300;
    
    buffer.buffer_stdin = (char*) malloc(buffer.buf_stdin_max_len);
    buffer.buffer_send =  (char*) malloc(buffer.buf_send_max_len);
    buffer.buffer_recv =  (char*) malloc(buffer.buf_recv_max_len);
    
    data.socket_fd = -1;
    data.countLogs = 0;
    
    if ( buffer.buffer_send != NULL && buffer.buffer_recv != NULL && buffer.buffer_stdin != NULL) {
        
        if ( initPeer(&data, &buffer) != (-1) ) {
            ret = mainloop(&data, &buffer);
        }
    }    
    
    //Dead code, unreachable
    
    return ret;
}

//ERROR -1
int initPeer(logger_data * data, peer_buffer * pb) {
   
    data->myPort = 8080;
    
    //struct MyPeer IP and Port
    memset(&data->myAddr, 0, sizeof(struct sockaddr_in));
    data->myAddr.sin_family = AF_INET;
    data->myAddr.sin_port = htons(data->myPort);
    data->myAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //create socket
    if((data->socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == (-1)){
        perror("Socket Error!");
        return -1;
    }
    
    //bind socket
    if(bind(data->socket_fd, (struct sockaddr*) &data->myAddr, sizeof(struct sockaddr)) == (-1)){
        perror("Socket Bind Error!");
        return -1;
    }
    
    return 0;
}


int mainloop(logger_data * data, peer_buffer * pb) {
    
    //FD SET
    fd_set fds;
    
    printf("\nPeer is online. Start communication..\n\n");
    
    
    printf("\n>");
    fflush(stdout);
        
    do{
        
        
        
        FD_ZERO(&fds);
        //Set Bits in Macro FD_SET
        FD_SET(0, &fds); //0 -> stdin
        FD_SET(data->socket_fd, &fds);
        
        int _select = select(data->socket_fd + 1, &fds, NULL, NULL, NULL);
        if(_select == (-1)){
            return -1;
        }
        
        if(FD_ISSET(0, &fds)){
            fgets(pb->buffer_stdin, pb->buf_stdin_max_len, stdin);
            pb->buffer_stdin[strlen(pb->buffer_stdin) -1] = 0; //delete '\n'
            
            //Check for "SUM" if true send start
            if(strcmp(pb->buffer_stdin, "SUM") == 0)
            {
                if ( sumCMD(data, pb) == (-1)) {
                    return -1;
                }
                
            }
            
            printf("\n>");
            fflush(stdout);
        }
      
        if(FD_ISSET(data->socket_fd, &fds)){
            //printf("Received something..\n");
            // testDebug(socket_fd);
            
            if ( receiveMessages(data, pb) == (-1) ) {
                printf("receive messages had an error\n");
                return -1;
            }
        }
    } while(1);
    
    return -1;
}

int sumCMD(logger_data * data, peer_buffer * pb) {
    
    short peerPort;
    struct sockaddr_in peerAddr;
    
    printf("Port?: ");
    scanf("%hd", &peerPort);
    
    printf("  Port: %d\n\n", peerPort);
    
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    peerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //send to ip:port
    return notifyPeers(
        data->socket_fd, data->myPort, &data->myAddr, pb,
        &peerAddr, 1, 
        NULL,
        START_PDU_TYPE_VALUE, -1
    );
}



int receiveMessages(logger_data * data, peer_buffer * pb) {
    
    char pdu_type;
    int buf_send_len;
    int buf_recv_len;
    struct sockaddr addr;
    unsigned int addr_len = sizeof(struct sockaddr);
    int sumOfEcho, ret;
    char * logMessage;
    int logID;
    
    
    buf_recv_len = recvfrom(data->socket_fd, pb->buffer_recv, pb->buf_recv_max_len, 0, &addr, &addr_len);
    if ( buf_recv_len == (-1)) {
        perror("Error Reading Receive");
        return -1;
    }
    
    pdu_type = (char) pb->buffer_recv[0];    
    
    //testDebug(pb->buffer_recv, buf_recv_len);
    //printf("\nReaded %d Bytes:\n", buf_recv_len);
    //printf("type-field: %d\n", pdu_type);
    
    
    switch (pdu_type) {
        
        case LOG_PDU_TYPE_VALUE: //LOG PDU
            
            //printf("case: LOG_PDU\n");
            unpackLogPDU(pb->buffer_recv, &logMessage);
            
            logID = data->countLogs++;
            printf("LOG: <%d> <%s>\n", logID,logMessage);
            
            
            return 0;
        
            
        case RESULT_PDU_TYPE_VALUE://RESULT PDU            
            unpackResultPDU(pb->buffer_recv, &sumOfEcho);
            printf("RESULT: <%d>\n", sumOfEcho);
           return 0;
            
            
        default:
            printf("Unknown pdu type\n");
            return -1;
    }
    
}




