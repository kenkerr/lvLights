#include "nwComms.h"
#include <iomanip>
#include "modbusTransaction.h"
#include "modbusRegisters.h"


// Constructor
NwComms::NwComms() {
    int            iResult;

    listenSocket = INVALID_SOCKET;
    clientSocket = INVALID_SOCKET;

    struct sockaddr_in serverAddr;


    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        cout << "socket failed with error: \n" << strerror(errno) << endl; 
        return;
    }

    cout << "listenSocket created, ID = " << listenSocket << endl;

//    Initialization the TCP network interfacee
    memset (&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(DEFAULT_PORT);


    // Setup the TCP listening socket
    iResult = bind(listenSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
    if (iResult < 0 ) {
        cout << "bind failed with error: \n" << strerror(errno) << endl;
        close (listenSocket);
        return;
    }

    cout << "listenSocket bound, result = " << iResult << endl;
};

NwComms::~NwComms(){
    cout << "NwComms> executing destructor\n";
};

void NwComms::closeConnection() {
    close (clientSocket);
    clientSocket = INVALID_SOCKET;
    return;
}

int NwComms::getRequest(ModbusTransaction *t){

    int                 i;
    int                 iResult;
    int                 nfds;                // highest socket # for select statement
    int                 nRcvBytes;
    int                 recvbuflen = DEFAULT_BUFLEN;
    int                 SelectResults;

    char                recvbuf[DEFAULT_BUFLEN];

    fd_set              fdSet;

    struct timeval      delayTmr;
    struct sockaddr     clientAddr;
    socklen_t           clientAddrLen;

    // Listen for incoming connection requests.
    if (listen(listenSocket, 1) < 0) {
        cout << "listen failed with error: " << strerror(errno) << endl;
        close (listenSocket);
        return NW_NETWORK_ERROR;
    }


#if 0
    // No longer need server socket
    close (listenSocket);
#endif


    // Initialize FD_SETS for the select function.
    FD_ZERO(&fdSet);                 // Clear the fd_set before select.

    if (INVALID_SOCKET == clientSocket) {
        cout << "Waiting on connect request on listen-socket; ID " << listenSocket << endl;
        FD_SET(listenSocket, &fdSet);         // Assign Our Socket with fdread set.
        nfds = listenSocket +1;
    }
    else {
        cout << "Waiting on data on client-socket; ID " << clientSocket << endl;
        FD_SET(clientSocket, &fdSet); // Assign Our Socket with fdread set.
        nfds = clientSocket +1;
    }

    // Seting up timer
    delayTmr.tv_sec  = 1;                                               // seconds
    delayTmr.tv_usec = 0;                                               // miliseconds

    // Select function.
    SelectResults = SOCKET_ERROR;
    SelectResults = select (nfds, &fdSet, NULL, NULL, &delayTmr);

    cout << "nwComms> SelectResults = " << SelectResults << endl;

    // Checking select's results.
    if (SelectResults == SOCKET_ERROR)
    {
        cout << "Select error\n";
        return NW_NETWORK_ERROR;
    }

    if (SelectResults > 0)
    {
        if (INVALID_SOCKET == clientSocket) {
            // That means we have some readable sockets.
            // Checking if our socket (listenSocket) exists in fdread SET.
            if (FD_ISSET(listenSocket, &fdSet))
            {
                // Accept a client socket
                clientAddrLen = sizeof(clientAddr);
                clientSocket = accept(listenSocket, &clientAddr, &clientAddrLen);
                if (clientSocket == INVALID_SOCKET) {
                    cout << "accept failed with error: \n" << strerror(errno) << endl;
                    close (listenSocket);
                    return NW_NETWORK_ERROR;
                }
                cout << "Client request accepted, received data:" << endl << endl;
                return NW_CONNECTION_ACCEPTED;
            }
            else {
                cout << "Expected a connection request" << endl;
                return NW_NETWORK_ERROR;
            }
        }
        else {
            if (FD_ISSET(clientSocket, &fdSet))
            {

                // Recieving data from socket
                memset(&recvbuf, 0, sizeof(&recvbuf));
                nRcvBytes = recv(clientSocket, recvbuf, recvbuflen, 0);
                if (nRcvBytes > 0) {
                    cout << "Bytes received: " << nRcvBytes << endl;
                    cout << "...bytes " << endl;
                    cout << showbase << internal << setfill('0');
                    for (i = 0; i < nRcvBytes; i++) {
                        cout << hex << setw(4) << (int) recvbuf[i] << " ";
                    }
                    cout << endl;

                    iResult = t->parseStream (nRcvBytes, recvbuf);
                    cout << "\n\tStatus Return:\t" << iResult       << endl;
                    cout << "\tFC: \t\t"         << t->getFC()    << endl;
                    cout << "\tStart HR: \t"     << t->getStAddr() << endl;
                    cout << "\t# HRs: \t\t"      << t->getNRegs() << endl;
                }
                else if (nRcvBytes == 0) {
                    cout << "Peer reset connection\n";
                    return NW_PEER_RESET_CONNECTION;
                } else  {
                    cout << "recv failed with error: \n" << strerror(errno);
                    close (clientSocket);
                    return NW_NETWORK_ERROR;
                }
            }
            else {
                cout << "Expected activity on clientSocket" << endl;
                return NW_NETWORK_ERROR;
            }
        }
    } else if (SelectResults == 0) {                // check for timeout
        cout << "timeout waiting for client request" << endl;

        return NW_TIMEOUT;                          // 
    }


#if 0
            // Echo the buffer back to the sender
            iSendResult = send(clientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", strerror(errno));
                close (clientSocket);
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
#endif

    cout << "getRequest> normal successful return" << endl;
    return NW_VALID_REQUEST;
}


int NwComms::sendResponse (char *resp, int respSz){

    cout << "sendResponse> sending response" << endl << "\t ";
    int nBytesSent = send (clientSocket, resp, respSz, 0);
    cout << "sendResponse> sent " << nBytesSent << " bytes" << endl << "\t ";
    for (int i=0; i<respSz; i++) {
        cout << (int) resp[i] << " ";
    }

}
