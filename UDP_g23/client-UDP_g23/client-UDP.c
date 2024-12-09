#if defined WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ECHOMAX 255

void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}
#if defined WIN32
void ClearWinSock() {
    WSACleanup();
}
#endif

int main() {
    #if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Error at WSAStartup");
    }
    #endif

    int sock;
    struct sockaddr_in serverAddr;
    char sendBuffer[ECHOMAX];
    char recvBuffer[sizeof(int)];
    int serverAddrLen = sizeof(serverAddr);
    int vocali;

    //Lettura del nome dell'host e della porta
    char host[100];
    printf("Inserisci il nome dell'host: ");
    scanf("%s", host);

    int PORT;
    printf("Inserisci la porta del server: ");
    scanf("%d", &PORT);
    getchar();

    //CREAZIONE SOCKET
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        ErrorHandler("socket() failed");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (strcmp(host, "localhost") == 0) {
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    } else {
        #if defined WIN32
        serverAddr.sin_addr.s_addr = inet_addr(host);
        if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
            ErrorHandler("inet_addr() failed");
        }
        #else
        if (inet_pton(AF_INET, host, &serverAddr.sin_addr) <= 0) {
            ErrorHandler("inet_pton() failed");
        }
        #endif
    }

    //Legge la stringa da inviare al server
    printf("Inserisci la stringa da inviare al server: ");
    scanf(" %[^\n]", sendBuffer);
    //Invia la stringa al server
    int sendLen = strlen(sendBuffer);
    if (sendto(sock, sendBuffer, sendLen, 0, (struct sockaddr *)&serverAddr, serverAddrLen) != sendLen) {
        ErrorHandler("sendto() failed");
    }
    //Ricezione di N dal server
    if (recvfrom(sock, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&serverAddr, &serverAddrLen) < 0) {
        ErrorHandler("recvfrom() failed");
    }
    memcpy(&vocali, recvBuffer, sizeof(vocali));
    //Stampa risultato
    printf("Stringa \"%s\" ricevuta dal server nome: %s indirizzo: %s\n", sendBuffer, host, inet_ntoa(serverAddr.sin_addr));
    printf("Numero di vocali nella stringa: %d\n", vocali);

    // Ritardo di 5 secondi per lasciare il messaggio visibile prima della chiusura del client
    #if defined WIN32
    Sleep(5000);//su Windows
    #else
    sleep(5);//su Linux
    #endif

    #if defined WIN32
    closesocket(sock);
    ClearWinSock();
    #else
    close(sock);
    #endif

    return 0;
}
