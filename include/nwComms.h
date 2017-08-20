#ifndef NWCOMMS_H
#define NWCOMMS_H
#include "LV_Lights.h"
#include "modbusTransaction.h"
#include "modbusRegisters.h"

#undef UNICODE

#define DEFAULT_BUFLEN              512
#define INVALID_SOCKET              (-1)
#define SOCKET_ERROR                (-1)
#define DEFAULT_PORT                502

#define NW_VALID_REQUEST              0
#define NW_CONNECTION_ACCEPTED        3
#define NW_NETWORK_ERROR            101
#define NW_TIMEOUT                  102
#define NW_PEER_RESET_CONNECTION    104

/* Windows includes 
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
*/

// Linux includes
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>


class NwComms {

public:
    NwComms ();
    ~NwComms();

    void init();
    void closeConnection();
    int getRequest(ModbusTransaction *req);
    int sendResponse(char *resp, int respSz);

private:
    int request;
    int listenSocket;
    int clientSocket;

    int parseRequest (char* reqBuf, ModbusTransaction* req);
    
};
#endif
