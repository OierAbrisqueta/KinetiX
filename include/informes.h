#ifndef KINETIX_INFORMES_H
#define KINETIX_INFORMES_H

typedef struct {
    char tipo;
    int num_alquileres;
    float recaudacion;
} ResumenPorTipo;

typedef struct {
    char fecha[20];
    int num_alquileres;
    float recaudacion;
} ResumenPorDia;

typedef struct {
    int id_vehiculo;
    char tipo;
    int num_alquileres;
    double minutos_totales;
} UsoVehiculo;

#endif //KINETIX_INFORMES_H