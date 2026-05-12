#ifndef KINETIX_SERVER_SOCKET_H
#define KINETIX_SERVER_SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET kinetix_socket_t;
#define KINETIX_SOCKET_INVALIDO INVALID_SOCKET
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef int kinetix_socket_t;
#define KINETIX_SOCKET_INVALIDO (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

int servidor_iniciar(int puerto);
kinetix_socket_t servidor_aceptar_cliente(void);
void servidor_atender_cliente(kinetix_socket_t cliente);
void servidor_cerrar(void);
int servidor_enviar(kinetix_socket_t s, const char *msg);
int servidor_recibir(kinetix_socket_t s, char *buf, int tam);

#endif