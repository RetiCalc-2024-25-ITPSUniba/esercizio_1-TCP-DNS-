#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> //per tolower()

#define PROTOPORT 53477
#define QLEN 6

//stampa il messaggio di errore
void ErrorHandler(char *errorMessage) {
    printf("%s", errorMessage);
}

//Pulizia della libreria Winsock
void ClearWinSock() {
    #if defined WIN32
    WSACleanup();
    #endif
}

//Funzione per contare il numero di vocali di una stringa
int contaVocali(const char *str) {
    int count = 0;
    char c;
    while ((c = *str++) != '\0') {
        c = tolower(c); //Converte il carattere in minuscolo
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
            count++;
        }
    }
    return count;
}

int main(int argc, char *argv[]) {
    int port;
    if (argc > 1) {
        port = atoi(argv[1]);
    } else {
        port = PROTOPORT; //solo se la porta non viene definita
    }

    //la porta non può essere negativa
    if (port < 0) {
        printf("Numero di porta errato %s \n", argv[1]);
        return 0;
    }

    //Inizializza winsock (Windows)
    #if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        ErrorHandler("Errore durante l'avvio di WSAStartup()\n");
        return 0;
    }
    #endif

    //CREAZIONE DELLA SOCKET e GESTIONE ERRORE
    int MySocket;
    MySocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (MySocket < 0) {
        ErrorHandler("Creazione socket fallita.\n");
        ClearWinSock();
        return -1;
    }

    // ASSEGNAZIONE DELL'INDIRIZZO E DELLA PORTA ALLA SOCKET
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1");
    sad.sin_port = htons(port);

    if (bind(MySocket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        ErrorHandler("bind() fallita.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    //SOCKET MESSA IN ASCOLTO
    if (listen(MySocket, QLEN) < 0) {
        ErrorHandler("listen() fallita.\n");
        closesocket(MySocket);
        ClearWinSock();
        return -1;
    }

    // ACCETTARE UNA NUOVA CONNESSIONE
    struct sockaddr_in cad; // struttura per l'indirizzo del client
    int clientSocket; // descrittore di socket per il client
    int clientLun; // la dimensione dell'indirizzo del client

    printf("In attesa di una connessione dal client...\n");

    while (1) {
        clientLun = sizeof(cad); // imposta la dimensione dell'indirizzo del client

        if ((clientSocket = accept(MySocket, (struct sockaddr *)&cad, &clientLun)) < 0) {
            ErrorHandler("accept() fallita.\n");
            closesocket(MySocket);
            ClearWinSock();
            return 0;
        }

        // clientSocket è connesso a un client
        struct hostent *clientHost;
        clientHost = gethostbyaddr((const char *)&cad.sin_addr.s_addr, sizeof(cad.sin_addr.s_addr), AF_INET);
        if (clientHost != NULL) {
            printf("Gestendo il client: %s\n", clientHost->h_name);
        } else {
            printf("Gestendo il client: %s\n", inet_ntoa(cad.sin_addr));
        }

        //Leggi e visualizza il primo messaggio dal client
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            printf("Messaggio ricevuto: %s\n", buffer);
        } else {
            printf("Impossibile ricevere il messaggio dal client.\n");
        }

        //Invia una risposta al client
        char response[1024];
        snprintf(response, sizeof(response), "benvenuto client:%s", inet_ntoa(cad.sin_addr));
        send(clientSocket, response, strlen(response), 0);

        //Ciclo per contare le vocali del messaggio del client
        int vocaliContate = 0;
        do {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                printf("Secondo messaggio ricevuto: %s\n", buffer);

                //Conta le vocali della stringa ricevuta
                vocaliContate = contaVocali(buffer);
                printf("Numero di vocali nella stringa ricevuta: %d\n", vocaliContate);

                //Se il numero di vocali è dispari
                if (vocaliContate % 2 != 0) {
                    char vocaliStr[16];
                    sprintf(vocaliStr, "%d", vocaliContate);
                    send(clientSocket, vocaliStr, strlen(vocaliStr), 0);
                }
            }
        } while (vocaliContate % 2 != 0);

        //Chiudi la connessione con il client
        closesocket(clientSocket);
    }

    // Chiude la socket
    closesocket(MySocket);
    ClearWinSock();
    return 0;
}
