#ifndef KINETIX_PROTOCOLO_SERVIDOR_H
#define KINETIX_PROTOCOLO_SERVIDOR_H

#include "server_socket.h"

/*
 * procesar_comando()
 *   Recibe un comando ya leido del socket y lo despacha
 *   al handler correspondiente. Escribe la respuesta en el socket.
 */
void procesar_comando(kinetix_socket_t cliente, const char *cmd);

#endif /* KINETIX_PROTOCOLO_SERVIDOR_H */