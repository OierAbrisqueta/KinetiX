#ifndef KINETIX_PROTOCOLO_SERVIDOR_H
#define KINETIX_PROTOCOLO_SERVIDOR_H

#include "server_socket.h"

void procesar_comando(kinetix_socket_t cliente, const char *cmd);

#endif /* KINETIX_PROTOCOLO_SERVIDOR_H */