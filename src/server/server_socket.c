#include "protocolo.h"
#include "server_socket.h"
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif
#include <stdio.h>
#include <string.h>
#include "gestor_log.h"

/* =============================================================
 *  server_socket.c  —  Implementacion de la capa de red
 * ============================================================= */

//Socket de escucha
static kinetix_socket_t g_socket_escucha = KINETIX_SOCKET_INVALIDO;

//Servidor_iniciar
int servidor_iniciar(int puerto) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        LOG_E("WSAStartup fallido.");
        return -1;
    }
#endif

    g_socket_escucha = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socket_escucha == KINETIX_SOCKET_INVALIDO) {
        LOG_E("No se pudo crear el socket de escucha.");
        return -1;
    }

    // Permite reusar el puerto inmediatamente tras reiniciar
    int opt = 1;
    setsockopt(g_socket_escucha, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&opt, sizeof(opt));

    struct sockaddr_in servidor;
    memset(&servidor, 0, sizeof(servidor));
    servidor.sin_family      = AF_INET;
    servidor.sin_addr.s_addr = INADDR_ANY;
    servidor.sin_port        = htons((unsigned short)puerto);

    if (bind(g_socket_escucha,
             (struct sockaddr *)&servidor, sizeof(servidor)) == SOCKET_ERROR) {
        LOG_E("Bind fallido.");
        closesocket(g_socket_escucha);
        return -1;
    }

    if (listen(g_socket_escucha, 1) == SOCKET_ERROR) {
        LOG_E("Listen fallido.");
        closesocket(g_socket_escucha);
        return -1;
    }

    char msg[64];
    snprintf(msg, sizeof(msg), "Servidor escuchando en puerto %d.", puerto);
    LOG_I(msg);
    printf("\n  >> Servidor iniciado. Esperando cliente en puerto %d...\n\n",
           puerto);
    return 0;
}

//Servidor_aceptar_cliente
kinetix_socket_t servidor_aceptar_cliente(void) {
    struct sockaddr_in cliente;
#ifdef _WIN32
    int tam = sizeof(cliente);
#else
    socklen_t tam = (socklen_t)sizeof(cliente);
#endif

    kinetix_socket_t sock_cliente = accept(
        g_socket_escucha, (struct sockaddr *)&cliente, &tam);

    if (sock_cliente == KINETIX_SOCKET_INVALIDO) {
        LOG_E("Accept fallido.");
        return KINETIX_SOCKET_INVALIDO;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Cliente conectado: %s:%d",
             inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port));
    LOG_AC(msg);
    printf("  >> Cliente conectado: %s:%d\n",
           inet_ntoa(cliente.sin_addr), ntohs(cliente.sin_port));

    return sock_cliente;
}

//Servidor_enviar
int servidor_enviar(kinetix_socket_t s, const char *msg) {
    int total   = (int)strlen(msg);
    int enviado = 0;
    while (enviado < total) {
        int n = send(s, msg + enviado, total - enviado, 0);
        if (n <= 0) return -1;
        enviado += n;
    }
    return 0;
}

//Servidor_recibir
int servidor_recibir(kinetix_socket_t s, char *buf, int tam) {
    int total = 0;
    while (total < tam - 1) {
        char c;
        int n = recv(s, &c, 1, 0);
        if (n <= 0) return -1;
        buf[total++] = c;
        if (c == '\n') break;
    }
    buf[total] = '\0';

    /* Eliminar '\n' y '\r' del final */
    while (total > 0 && (buf[total-1] == '\n' || buf[total-1] == '\r'))
        buf[--total] = '\0';

    return total;
}

//Servidor_atender_cliente
void procesar_comando(kinetix_socket_t cliente, const char *cmd);

void servidor_atender_cliente(kinetix_socket_t cliente) {
    char buf[PROTO_BUFF_SIZE];
    LOG_I("Inicio de sesion con cliente.");

    while (1) {
        int n = servidor_recibir(cliente, buf, sizeof(buf));
        if (n < 0) {
            LOG_A("Cliente desconectado inesperadamente.");
            break;
        }
        if (n == 0) continue;

        char log_msg[PROTO_BUFF_SIZE + 32];
        snprintf(log_msg, sizeof(log_msg), "Comando recibido: %s", buf);
        LOG_I(log_msg);

        if (strcmp(buf, CMD_EXIT) == 0) {
            servidor_enviar(cliente, RESP_BYE "\n");
            LOG_AC("Cliente ha cerrado la sesion.");
            break;
        }

        procesar_comando(cliente, buf);
    }

    closesocket(cliente);
}

// Servidor_cerrar
void servidor_cerrar(void) {
    if (g_socket_escucha != KINETIX_SOCKET_INVALIDO) {
        closesocket(g_socket_escucha);
        g_socket_escucha = KINETIX_SOCKET_INVALIDO;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    LOG_I("Servidor cerrado.");
}