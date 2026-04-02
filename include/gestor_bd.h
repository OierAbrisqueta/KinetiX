#ifndef KINETIX_GESTOR_BD_H
#define KINETIX_GESTOR_BD_H

#include <sqlite3.h>
#include "modelos.h"

int  conectar_bd(const char *ruta_bd);
void cerrar_bd(void);

int insertar_estacion(Estacion e);
int insertar_vehiculo(Vehiculo v);
int insertar_usuario(Usuario u);
int insertar_alquiler(Alquiler a);

int borrar_estacion(int id);
int dar_de_baja_vehiculo(int id);
int dar_de_baja_usuario(int id);

int actualizar_estacion(Estacion e);
int actualizar_usuario(Usuario u);
int actualizar_vehiculo(Vehiculo v);

int listar_estaciones(Estacion **lista_out, int *cantidad_out);
int listar_vehiculos(Vehiculo  **lista_out, int *cantidad_out);
int listar_usuarios(Usuario   **lista_out, int *cantidad_out);
int listar_alquileres(Alquiler **lista_out, int *cantidad_out);
int listar_vehiculos_bateria_baja(Vehiculo **lista_out, int *cantidad_out, int umbral);

int buscar_estacion(int id, Estacion *out);
int buscar_vehiculo(int id, Vehiculo *out);
int buscar_usuario(int id,  Usuario  *out);
int buscar_alquiler_por_usuario(int id_usuario, Alquiler **lista_out, int *n_out);
int buscar_alquiler_por_fecha(char *fecha, Alquiler **lista_out, int *n_out);

#endif