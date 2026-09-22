#ifndef KINETIX_NETWORK_HPP
#define KINETIX_NETWORK_HPP

#include "protocolo.h"
#include <cstdio>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#endif

#ifdef _WIN32
static SOCKET g_sock = INVALID_SOCKET;
#else
using SocketHandle = int;
static const SocketHandle INVALID_SOCKET = -1;
static SocketHandle g_sock = INVALID_SOCKET;
#endif

// Conecta al servidor. Devuelve 0 si ok, -1 si error.
int net_conectar(const char *ip, int puerto);

// Envia un mensaje al servidor
void net_enviar(const char *msg);

// Recibe una linea del servidor (hasta '\n')
void net_recibir_linea(char *buf, int tam);

// Envia un comando y guarda la primera linea de respuesta en buf
void net_cmd(const char *comando, char *buf, int tam);

#endif //KINETIX_NETWORK_HPP