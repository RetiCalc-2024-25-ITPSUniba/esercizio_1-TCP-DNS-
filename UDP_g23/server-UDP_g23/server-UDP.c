#if defined WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define PORT 53477
#define ECHOMAX 255

//Funzione per gestire gli errori
void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
    exit(EXIT_FAILURE);
}

#if defined WIN32
void ClearWinSock() {
    WSACleanup();
}
#endif

//Funzione per contare il numero di vocali in una stringa
int contaVocali(const char *str) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        char c = tolower(str[i]); //converte in minuscolo
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
            count++;
        }
    }
    return count;
}

int main() {
    #if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Error at WSAStartup");
    }
    #endif

    int sock;
    struct sockaddr_in serverAddr, clientAddr;
    unsigned int clientAddrLen;
    char buffer[ECHOMAX];
    int recvMsgSize;

    //CREAZIONE SOCKET
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        ErrorHandler("socket() failed");
    }

    //Costruzione dell'indirizzo del server
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        ErrorHandler("bind() failed");
    }
    printf("Server in ascolto sulla porta %d...\n", PORT);
    while (1) {
        clientAddrLen = sizeof(clientAddr);
        //ricezione dei dati dal client
        recvMsgSize = recvfrom(sock, buffer, ECHOMAX, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (recvMsgSize < 0) {
            ErrorHandler("recvfrom() failed");
        }
        buffer[recvMsgSize] = '\0';
        printf("Stringa ricevuta: %s\n", buffer);

        //Conta delle vocali della stringa
        int vocali = contaVocali(buffer);
        printf("Numero di vocali: %d\n", vocali);
        //Invio del numero delle vocali al client
        if (sendto(sock, (char *)&vocali, sizeof(vocali), 0, (struct sockaddr *)&clientAddr, clientAddrLen) != sizeof(vocali)) {
            ErrorHandler("sendto() failed");
        }
    }
    #if defined WIN32
    ClearWinSock();
    #endif

    return 0;
}
