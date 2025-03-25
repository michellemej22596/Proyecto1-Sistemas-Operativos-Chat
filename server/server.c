#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENTES 100
#define MAX_MENSAJE 512

struct per_session_data {
    char username[50];
    char estado[20];
};

static struct lws *clientes[MAX_CLIENTES];
static struct per_session_data *datos[MAX_CLIENTES];
static int cliente_count = 0;

void broadcast_mensaje(const char *msg) {
    for (int i = 0; i < cliente_count; ++i) {
        if (!clientes[i]) continue;
        unsigned char buf[LWS_PRE + MAX_MENSAJE];
        size_t len = strlen(msg);
        if (len >= MAX_MENSAJE) len = MAX_MENSAJE - 1;
        memcpy(&buf[LWS_PRE], msg, len);
        lws_write(clientes[i], &buf[LWS_PRE], len, LWS_WRITE_TEXT);
    }
}

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    struct per_session_data *psd = (struct per_session_data *)user;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            if (cliente_count < MAX_CLIENTES) {
                clientes[cliente_count] = wsi;
                datos[cliente_count] = psd;
                strcpy(psd->username, "Anónimo");
                strcpy(psd->estado, "ACTIVO");
                cliente_count++;
                printf("🟢 Nuevo cliente conectado (%d)\n", cliente_count);
            }
            break;

        case LWS_CALLBACK_RECEIVE: {
            char *mensaje = (char *)in;
            printf("📨 Recibido: %s\n", mensaje);

            if (strncmp(mensaje, "USERNAME:", 9) == 0) {
                strncpy(psd->username, mensaje + 9, sizeof(psd->username) - 1);
                psd->username[sizeof(psd->username) - 1] = '\0';

                char aviso[100];
                snprintf(aviso, sizeof(aviso), "🧑 %s se ha unido al chat.", psd->username);
                broadcast_mensaje(aviso);
            }
            else if (strncmp(mensaje, "STATUS:", 7) == 0) {
                strncpy(psd->estado, mensaje + 7, sizeof(psd->estado) - 1);
                psd->estado[sizeof(psd->estado) - 1] = '\0';

                char estado_msg[100];
                snprintf(estado_msg, sizeof(estado_msg), "📢 %s ahora está %s", psd->username, psd->estado);
                broadcast_mensaje(estado_msg);
            }
            else {
                char envio[300];
                snprintf(envio, sizeof(envio), "%s: %s", psd->username, mensaje);
                broadcast_mensaje(envio);
            }
            break;
        }

        case LWS_CALLBACK_CLOSED:
            for (int i = 0; i < cliente_count; ++i) {
                if (clientes[i] == wsi) {
                    char msg[100];
                    snprintf(msg, sizeof(msg), "❌ %s se ha desconectado.", datos[i]->username);
                    broadcast_mensaje(msg);

                    for (int j = i; j < cliente_count - 1; ++j) {
                        clientes[j] = clientes[j + 1];
                        datos[j] = datos[j + 1];
                    }
                    cliente_count--;
                    break;
                }
            }
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "chat",
        callback_chat,
        sizeof(struct per_session_data),
        MAX_MENSAJE
    },
    { NULL, NULL, 0, 0 }
};

int main(void) {
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "❌ Error al crear el contexto WebSocket\n");
        return -1;
    }

    printf("🌐 Servidor WebSocket listo en ws://localhost:8080\n");

    while (1) {
        lws_service(context, 0);
    }

    lws_context_destroy(context);
    return 0;
}