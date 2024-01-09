
#include "peer.h"
#include "time.h"



void resetAlgo(peer_data* data);
int initPeer(peer_data * data, peer_buffer * pb, int argc, char** argv);
void printList(struct sockaddr_in * list, int iCount);
int mainloop(peer_data * data, peer_buffer * pb);
int receiveMessages(peer_data * data, peer_buffer * pb);




//0 ./file 
//1 peerMyPort 
//2 myStorageSize 
//3 + (0) neigh0port 
//3 + (1) neigh1port 
//3 + (..) neigh2port 
int main(int argc, char** argv){
    
    peer_data data;
    peer_buffer buffer;
    int ret = -1;
    
    
    if (argc < 4 ) {
        printf("!Usage: to less param\n");
        return -1;
    }
    
    data.numberOfNeighbours = argc - 3;
    data.socket_fd = -1;
    
    buffer.buf_stdin_max_len = 0;
    buffer.buf_send_max_len = 300;
    buffer.buf_recv_max_len = 300;
    
    buffer.buffer_stdin = (char*) NULL;
    buffer.buffer_send =  (char*) malloc(buffer.buf_send_max_len);
    buffer.buffer_recv =  (char*) malloc(buffer.buf_recv_max_len);
    
    data.neighbours = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in) * (data.numberOfNeighbours));
    
    if ( buffer.buffer_send != NULL && buffer.buffer_recv != NULL && data.neighbours != NULL) {
        
        if ( initPeer(&data, &buffer, argc, argv) != (-1) ) {
            
            printf("\nPeerPort: %d\n", data.myPort);
            printf("\nPeerStorageSize: %d\n", data.myStorageSize);
            
            printList(data.neighbours, data.numberOfNeighbours);
            
            ret = mainloop(&data, &buffer);
        }
    }    
    
    //Dead code, unreachable
    
    return ret;
}

//ERROR -1
int initPeer(peer_data * data, peer_buffer * pb, int argc, char** argv) {
    
    struct sockaddr_in * currentPeer = NULL;
    short currenPeerPort = 0;
    
    sscanf(argv[1], "%hd", &data->myPort);
    sscanf(argv[2], "%d", &data->myStorageSize);
    
    //struct PeerList of Neighbours
    for (int i = 0 ; i < data->numberOfNeighbours;  i++) {
        
        currentPeer = data->neighbours + i;
        
        //neighbour IP and Port
        sscanf(argv[3 + i], "%hd", &currenPeerPort);
        currentPeer->sin_family = AF_INET;
        currentPeer->sin_port = htons(currenPeerPort);
        currentPeer->sin_addr.s_addr = inet_addr("127.0.0.1");
    }
   
    //struct MyPeer IP and Port
    memset(&data->myAddr, 0, sizeof(data->myAddr));
    data->myAddr.sin_family = AF_INET;
    data->myAddr.sin_port = htons(data->myPort);
    data->myAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    //struct Logger IP and Port
    memset(&data->loggerAddr, 0, sizeof(data->myAddr));
    data->loggerAddr.sin_family = AF_INET;
    data->loggerAddr.sin_port = htons(8080);
    data->loggerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    resetAlgo(data);
    
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

void resetAlgo(peer_data* data) {
    data->upwardSet = 0; //FALSE
    data->informed = 0;  //FALSE
    data->countPDU = 0;
    data->currentSum = data->myStorageSize;
}


void printList(struct sockaddr_in * list, int iCount) {
    int i;
    struct sockaddr_in * currItem;
    
    printf("\n\n ~~~ LIST ~~~\n");
    for ( i = 0 ; i < iCount ; i++ ) {
        currItem = list + i;
        
        printf(" [%d] %s:%hd \n", 
               i, 
               inet_ntoa(currItem->sin_addr),  //inet_ntoa: Internet host address to a string in IPv4 dotted-decimal notation
               ntohs(currItem->sin_port)  //network to host short
              );
    }
    printf("\n");
    
    
}


int mainloop(peer_data * data, peer_buffer * pb) {
    
    //FD SET
    fd_set fds;
    
    printf("\nPeer is online. Start communication..\n\n");
    
    do{
        
        
        
        FD_ZERO(&fds);
        //Set Bits in Macro FD_SET
        FD_SET(data->socket_fd, &fds);
        
        int _select = select(data->socket_fd + 1, &fds, NULL, NULL, NULL);
        if(_select == (-1)){
            perror("Error: select\n");
            return -1;
        }
      
        //printf("SELECT RETURNED!\n");
      
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


int receiveMessages(peer_data * data, peer_buffer * pb) {
    
    char pdu_type;
    int buf_send_len;
    int buf_recv_len;
    struct sockaddr addr;
    unsigned int addr_len = sizeof(struct sockaddr);
    int sumOfEcho, ret;
    int sumBefore, sumAfter;
    struct timespec _tmspec;
    
    
    buf_recv_len = recvfrom(data->socket_fd, pb->buffer_recv, pb->buf_recv_max_len, 0, &addr, &addr_len);
    if ( buf_recv_len == (-1)) {
        perror("Error Reading Receive");
        return -1;
    }
    
    
    
    _tmspec.tv_sec = 0;
    _tmspec.tv_nsec = rand() % (1000 * 1000 * 100);    //((10e6)*100);  //10^6nsec = 1ms
    nanosleep(&_tmspec ,NULL);
    
    
    pdu_type = (char) pb->buffer_recv[0];    
    
    //testDebug(pb->buffer_recv, buf_recv_len);
    //printf("\nReaded %d Bytes:\n", buf_recv_len);
    //printf("type-field: %s\n", getType(pdu_type));
    
    
    switch (pdu_type) {
        

        //IF ( PDU == ECHO )
        case ECHO_PDU_TYPE_VALUE://ECHO PDU
            unpackEchoPDU(pb->buffer_recv, &sumOfEcho);
            
            //aggregate sum
            
            sumBefore = data->currentSum;
            data->currentSum += sumOfEcho;
            sumAfter =  data->currentSum;
            
            //printf("Aggregate: Value: %d Before: %d After %d\n", sumOfEcho, sumBefore, sumAfter);
            
            //!!! fall through
        case INFO_PDU_TYPE_VALUE: //INFO PDU
            
            data->countPDU++;
            
            if ( data->informed == 0 ) {
                data->informed = 1;//TRUE
                data->upwardSet = 1;// TRUE
                memcpy( &data->upwardAddr, &addr, sizeof(struct sockaddr_in));
                
                ret = notifyPeers(data->socket_fd, data->myPort, &data->loggerAddr, pb,
                    data->neighbours, data->numberOfNeighbours, 
                    (struct sockaddr_in*) &addr,
                    INFO_PDU_TYPE_VALUE, -1
                );
                if ( ret == (-1) ) {
                    return -1;
                }
            }
            
            if (data->countPDU == data->numberOfNeighbours){
                
                if ( data->upwardSet == 0 ) {
                    ret =  notifyPeers(data->socket_fd, data->myPort, &data->loggerAddr, pb,
                        &data->loggerAddr, 1, 
                        NULL,
                        RESULT_PDU_TYPE_VALUE, data->currentSum
                    );    
                    
                } else {
                    ret =  notifyPeers(data->socket_fd, data->myPort, &data->loggerAddr, pb,
                        &data->upwardAddr, 1, 
                        NULL,
                        ECHO_PDU_TYPE_VALUE, data->currentSum
                    );    
                }
                
                if ( ret == (-1) ) {
                    return -1;
                }
                
                resetAlgo(data);
            }
            
            
            return 0;
        
            
        case START_PDU_TYPE_VALUE://START PDU
            
            //Sende Nachricht mit Content an alle direkten Nachbarn; flag = true; 
            
            data->informed = 1;//TRUE
            
            return notifyPeers(data->socket_fd, data->myPort, &data->loggerAddr, pb,
                data->neighbours, data->numberOfNeighbours, 
                NULL,
                INFO_PDU_TYPE_VALUE, -1
                );
            
            
        default:
            printf("Unknown pdu type\n");
            return -1;
    }
}




