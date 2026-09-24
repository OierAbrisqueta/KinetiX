#include "Network.hpp"
#include <cstring>

#ifdef _WIN32
static SOCKET g_sock = INVALID_SOCKET;
#else
static SocketHandle g_sock = INVALID_SOCKET;
#endif

int net_conectar(const char *ip, int puerto) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sock == INVALID_SOCKET) return -1;

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(puerto);
    inet_pton(AF_INET, ip, &srv.sin_addr);

    if (connect(g_sock, (struct sockaddr *)&srv, sizeof(srv)) != 0) {
#ifdef _WIN32
        closesocket(g_sock);
#else
        close(g_sock);
#endif
        return -1;
    }
    return 0;
}

void net_enviar(const char *msg) {
    send(g_sock, msg, strlen(msg), 0);
}

void net_recibir_linea(char *buf, int tam) {
    int i = 0;
    while (i < tam - 1) {
        char c;
        if (recv(g_sock, &c, 1, 0) <= 0) break;
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    // Quitar el salto de linea del final
    if (i > 0 && buf[i-1] == '\n') buf[i-1] = '\0';
}

void net_cmd(const char *comando, char *buf, int tam) {
    char msg[PROTO_BUFF_SIZE];
    snprintf(msg, sizeof(msg), "%s\n", comando);
    net_enviar(msg);
    net_recibir_linea(buf, tam);
}

void net_desconectar() {
#ifdef _WIN32
    closesocket(g_sock);
    WSACleanup();
#else
    close(g_sock);
#endif
}