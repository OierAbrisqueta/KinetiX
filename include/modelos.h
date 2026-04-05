#ifndef KINETIX_MODELOS_H
#define KINETIX_MODELOS_H

typedef struct {
    int id_estacion;
    char nombre[51];
    char direccion[101];
    float coord_x;
    float coord_y;
    int capacidad_max;
    int disponibilidad_actual;
} Estacion;

typedef struct {
    int id_vehiculo;
    char tipo;
    float bateria;
    int id_estacion;
    char estado;
} Vehiculo;

typedef struct {
    int id_usuario;
    char dni[10];
    char nombre[51];
    float saldo;
    char contrasena[51];
} Usuario;

typedef struct {
    int id_alquiler;
    int id_usuario;
    int id_vehiculo;
    int id_estacion_origen;
    int id_estacion_destino;
    char fecha_inicio[20];
    char fecha_fin[20];
    float coste_total;
} Alquiler;

#endif //KINETIX_MODELOS_H