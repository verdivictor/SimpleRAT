#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#define STANDARD_PORT "3490"
#define BACKLOG 10

int main()
{

    while(1){
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
            fprintf(stderr, "WSAStartup failed.\n");
            exit(1);
        }

        int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(servSock < 0){
            fprintf(stderr, "Socket error.\n");
            WSACleanup();
            exit(1);
        }

        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servAddr.sin_port = htons(atoi(STANDARD_PORT));

        if(bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
            fprintf(stderr, "Bind error.\n");
            WSACleanup();
            exit(1);
        }

        if(listen(servSock, BACKLOG) < 0){
            fprintf(stderr, "Listen error.\n");
            exit(1);
        }

        printf("%s", "Listening on socket for incoming connections.\n");

        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);

        int newsock = accept(servSock, (struct sockaddr*) &clntAddr, &clntAddrLen);
        if(newsock < 0){
            fprintf(stderr, "Accept connection error");
            continue;
        }

        char clntName[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
                printf("Handling client %s:%d\n", clntName, ntohs(clntAddr.sin_port));

        while(1){

            char command[256];
            printf("Give your command: ");
            fgets(command, sizeof(command), stdin);
            if (strcmp(command, "quit\n") == 0){
                closesocket(newsock);
                WSACleanup();
                break;
            }
            int len, bytes_sent, recvd;
            len = strlen(command);
            bytes_sent = send(newsock, command, len, 0);
            printf("Send return value: %d\n", bytes_sent);
            char buffer[500];
            while(1)
            {
                memset(buffer, 0, 500);
                recvd =  recv(newsock, buffer, 499, 0);
                if(recvd == 0){
                    closesocket(newsock);
                    WSACleanup();
                    break;
                }

                printf("%s" , buffer, recvd);
                printf("Strlen: %d", strlen(buffer));
                printf("\nNull term pos: 0x%0.4x\n", strstr(buffer, "\0"));
                if(strlen(buffer) < 499)
                    break;
            }
            memset(command, 0, sizeof(command));
        }
        closesocket(newsock);
        closesocket(servSock);

    }
    WSACleanup();
    return 0;
}
