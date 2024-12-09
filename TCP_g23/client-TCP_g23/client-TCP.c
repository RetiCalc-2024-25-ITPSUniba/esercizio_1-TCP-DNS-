#if defined _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib") // Link della libreria Winsock
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
typedef int SOCKET;
#endif

void ErrorHandler(const char *errorMessage) {
    perror(errorMessage);
}

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

int main() {
#if defined _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        ErrorHandler("WSAStartup fallito");
        return 1;
    }
#endif

    SOCKET sock;
    struct sockaddr_in serverAddr;
    char buffer[1024];
    int port;

    //Legge il nome dell'host (es. localhost) o l'indirizzo IP
    char serverHost[256];
    printf("Inserisci l'indirizzo dell'host (puoi usare localhost o un indirizzo IP): ");
    scanf("%255s", serverHost);

    //Legge la porta
    printf("Inserisci la porta del server: ");
    scanf("%d", &port);
    //Pulire il buffer di input
    getchar();

    //CREAZIONE DELLA SOCKET
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        ErrorHandler("Creazione del socket fallita");
        ClearWinSock();
        return 1;
    }

    //Assegnazioni indirizzo e porta
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    //Risoluzione del nome host
    #if !defined _WIN32
        if (inet_pton(AF_INET, serverHost, &serverAddr.sin_addr) <= 0) {
            // Se non è un indirizzo IP, tentiamo di risolvere l'host
            struct hostent *host = gethostbyname(serverHost);
            if (host == NULL) {
                ErrorHandler("Impossibile risolvere il nome dell'host");
                closesocket(sock);
                ClearWinSock();
                return 1;
            }
            serverAddr.sin_addr = *((struct in_addr *)host->h_addr);//assegna l'indirizzo IP risolto
        }
    #else
        //Usa inet_addr come alternativa a InetPton per ottenere l'IPsu Windows
        serverAddr.sin_addr.s_addr = inet_addr(serverHost);
        if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
            struct hostent *host = gethostbyname(serverHost);
            if (host == NULL) {
                ErrorHandler("Impossibile risolvere il nome dell'host");
                closesocket(sock);
                ClearWinSock();
                return 1;
            }
            serverAddr.sin_addr = *((struct in_addr *)host->h_addr);
        }
    #endif

    //CONNESSIONE AL SERVER
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        ErrorHandler("Connessione al server fallita");
        closesocket(sock);
        ClearWinSock();
        return 1;
    }

    //Invio messaggio iniziale al server
    char nomeClient[256];
    printf("Inserisci il tuo nome: ");
    fgets(nomeClient, sizeof(nomeClient), stdin);
    nomeClient[strcspn(nomeClient, "\n")] = 0;
    snprintf(buffer, sizeof(buffer), "messaggio iniziale per il server: %s", nomeClient);
    send(sock, buffer, strlen(buffer), 0);

    //Ricezione del messaggio di benvenuto dal server
    int byteRicevuti = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (byteRicevuti > 0) {
        buffer[byteRicevuti] = '\0';
        printf("Risposta dal server: %s\n", buffer);
    } else {
        printf("Errore durante la ricezione del messaggio dal server.\n");
    }

    //Ciclo per inviare stringhe al server
    while (1) {
        printf("Inserisci una frase: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send(sock, buffer, strlen(buffer), 0);

        //Ricezione di N (numero delle vocali)
        byteRicevuti = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (byteRicevuti > 0) {
            buffer[byteRicevuti] = '\0';
            printf("Numero di vocali ricevuto dal server: %s\n", buffer);

            // Controlla se il server ha terminato (numero di vocali pari)
            int vowelCount = atoi(buffer);
            if (vowelCount % 2 == 0) {
                printf("Il server ha terminato la sessione.\n");
                break;
            }
        } else {
            printf("Errore durante la ricezione del numero di vocali.\n");
            break;
        }
    }

    //CHIUSURA CONNESSIONE
    closesocket(sock);
    ClearWinSock();
    return 0;
}
