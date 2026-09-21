#ifndef KINETIX_MODELOS_CLIENTE_H
#define KINETIX_MODELOS_CLIENTE_H

#include "Vehicle.hpp"

//Estacion
struct Estacion {
    int   id_estacion;
    char  nombre[51];
    char  direccion[101];
    float coord_x;
    float coord_y;
    int   capacidad_max;
    int   disponibilidad_actual;

    float getOcupacion() const;
    void  descripcion(char *buf, int tam) const;

    static Estacion fromString(const char *linea);
};


#endif /* KINETIX_MODELOS_CLIENTE_H */