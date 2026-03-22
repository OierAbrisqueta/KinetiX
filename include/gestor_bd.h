#ifndef KINETIX_GESTOR_BD_H
#define KINETIX_GESTOR_BD_H

#include <sqlite3.h>
#include "modelos.h"

int conectar_bd(const char* ruta_bd);
void cerrar_bd();
int insertar_estacion(Estacion nueva_estacion);
int insertar_vehiculo(Vehiculo nuevo_vehiculo);
int insertar_usuario(Usuario nuevo_usuario);
int insertar_alquiler(Alquiler nuevo_alquiler);

#endif //KINETIX_GESTOR_BD_H