#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct per_session_data {
    int socket;
    char username[50];
};

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    struct per_session_data *psd = (struct per_session_data *)user;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            // Conexión establecida, registrar el usuario
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_RECEIVE:
            // Recibir mensaje
            printf("Mensaje recibido: %s\n", (char *)in);
            break;
        case LWS_CALLBACK_CLOSED:
            // El cliente se desconectó
            printf("El usuario %s se desconectó\n", psd->username);
            break;
        default:
            break;
    }
    return 0;
}

// Definir el protocolo WebSocket
static struct lws_protocols protocols[] = {
    {
        "chat", callback_chat, sizeof(struct per_session_data),
    },
    { NULL, NULL, 0 }
};

int main() {
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;  // Puerto WebSocket
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        printf("Error al crear el contexto WebSocket\n");
        return -1;
    }

    // Iniciar el bucle de eventos
    while (1) {
        lws_service(context, 0); // Este bucle maneja las conexiones
    }

    lws_context_destroy(context);
    return 0;
}
