#ifndef KINETIX_GESTOR_BD_H
#define KINETIX_GESTOR_BD_H

#include <sqlite3.h>
#include "models.h"

int conectar_bd(const char* ruta_bd);
void cerrar_bd();

#endif //KINETIX_GESTOR_BD_H