#include "common.h"

#define STDIN_LIMIT 255  //((2^8) -1)


typedef struct {
    int socket_fd;
    short myPort;
    struct sockaddr_in myAddr; //IPv4
    int countLogs;
} logger_data;
