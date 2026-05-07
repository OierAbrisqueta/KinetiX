#ifndef KINETIX_SERVER_SOCKET_H
#define KINETIX_SERVER_SOCKET_H

#include <winsock2.h>

typedef SOCKET kinetix_socket_t;
#define KINETIX_SOCKET_INVALIDO INVALID_SOCKET

int servidor_iniciar(int puerto);
kinetix_socket_t servidor_aceptar_cliente(void);
void servidor_atender_cliente(kinetix_socket_t cliente);
void servidor_cerrar(void);
int servidor_enviar(kinetix_socket_t s, const char *msg);
int servidor_recibir(kinetix_socket_t s, char *buf, int tam);

#endif