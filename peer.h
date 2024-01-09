
#include "common.h"





typedef struct {
    int socket_fd;
    short myPort;
    int myStorageSize;
    struct sockaddr_in myAddr; //IPv4
    struct sockaddr_in loggerAddr; //IPv4
    
    struct sockaddr_in * neighbours;
    int numberOfNeighbours;
    
    int upwardSet;//BOOL 0 = FALSE , else = TRUE
    struct sockaddr_in upwardAddr; //IPv4
    int informed;//BOOL 0 = FALSE , else = TRUE
    int countPDU; //== #Neighbours => send upward
    int currentSum; //aggregate storageSize here
} peer_data;


