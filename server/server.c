#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

// Estructura para manejar la información de los clientes
typedef struct {
    int socket;
    char username[50];
} ClientInfo;

// Lista de clientes conectados (para manejar broadcasting)
ClientInfo clients[100];
int clientCount = 0;

void *handleClient(void *arg) {
    ClientInfo *client = (ClientInfo *)arg;
    // Lógica para recibir y enviar mensajes
    // Registrar usuario, manejar mensajes, cerrar conexión
    return NULL;
}

int main(int argc, char *argv[]) {
    int serverSock, clientSock, port;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    pthread_t tid;

    if (argc != 2) {
        printf("Uso: %s <puerto>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    // Crear socket
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("No se pudo crear el socket");
        exit(1);
    }

    // Configurar dirección del servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Enlazar el socket
    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error al enlazar");
        exit(1);
    }

    // Escuchar por conexiones entrantes
    listen(serverSock, 5);

    printf("Servidor escuchando en el puerto %d...\n", port);

    // Aceptar conexiones entrantes
    while (1) {
        clientAddrLen = sizeof(clientAddr);
        clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSock < 0) {
            perror("Error al aceptar la conexión");
            continue;
        }

        // Crear un hilo para manejar el cliente
        ClientInfo *newClient = malloc(sizeof(ClientInfo));
        newClient->socket = clientSock;
        // Aquí puedes agregar lógica para registrar el nombre de usuario
        pthread_create(&tid, NULL, handleClient, (void *)newClient);
    }

    close(serverSock);
    return 0;
}
