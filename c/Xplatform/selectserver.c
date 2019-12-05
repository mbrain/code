#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

# define MAX_CLIENTS 10

int main() {

    long rc;
    SOCKET acceptSocket;
    //SOCKET connectedSocket; 
    SOCKADDR_IN addr;
    char buf[256];
    char buf2[300];
    
    // zusätzliche Variabeln
    fd_set fdSet;
    SOCKET clients[MAX_CLIENTS];
    int i;
  
    // Winsock starten
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 0), &wsa);

    // Socket erstellen
    acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (acceptSocket == INVALID_SOCKET) return 1;
  
    // Socket binden
    memset( & addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;
    rc = bind(acceptSocket, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN));
    if (rc == SOCKET_ERROR) return 1;
  
    // In den listen Modus
    rc = listen(acceptSocket, 10);
    if (rc == SOCKET_ERROR) return 1;
  
    for (i = 0; i < MAX_CLIENTS; i++) clients[i] = INVALID_SOCKET;
  
    while (1) {
    
        FD_ZERO(&fdSet);
        FD_SET(acceptSocket, &fdSet);
    
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != INVALID_SOCKET) FD_SET(clients[i], & fdSet);
        }
    
        rc = select(0, & fdSet, NULL, NULL, NULL);
        if (rc == SOCKET_ERROR) return 1;
    
        if(FD_ISSET(acceptSocket, &fdSet)) {
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == INVALID_SOCKET) {
                      clients[i] = accept(acceptSocket, NULL, NULL);
                      printf("Neuen Client angenommen (%d)\n", i);
                      break;
                }
            }
        }
      
        // prüfen wlecher client sockets im fd_set sind
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == INVALID_SOCKET) continue;
            if (FD_ISSET(clients[i], & fdSet)) {
                rc = recv(clients[i], buf, 256, 0);
                if (rc == 0 || rc == SOCKET_ERROR) {
                    printf("Client %d hat die Verbindung geschlossen\n", i);
                    closesocket(clients[i]);
                    clients[i] = INVALID_SOCKET;
                } else {
                    buf[rc] = '\0';
                    printf("Client %d hat folgendes gesandt: %s\n", i, buf);
                    sprintf(buf2, "Du mich auch %s\n", buf);
                    send(clients[i], buf2, (int) strlen(buf2), 0);
                }
            }
        }
      
    }
}
