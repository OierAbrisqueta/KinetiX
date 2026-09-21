#include "modelos_cliente.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//Estacion
float Estacion::getOcupacion() const {
    if (capacidad_max == 0) return 0;
    float devolver = 100.0f * (float)(capacidad_max - disponibilidad_actual)/(float)capacidad_max;
    return devolver;
}
void  Estacion::descripcion(char *buf, int tam) const {
    snprintf(buf, tam,
             "%d, %s, Libres: %d/%d, Ocupacion: %d%%",
             id_estacion,
             nombre,
             disponibilidad_actual,
             capacidad_max,
             (int)getOcupacion());
}

Estacion Estacion::fromString(const char *linea) {
    Estacion e{};

    //Parseamos campo por campo
    char copia[256];
    strncpy(copia, linea, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *campos[7];
    int n = 0;
    char *item = copia;

    campos[n++] = item;
    while (*item && n < 7) {
        if (*item == '|') {
            *item = '\0';
            campos[n++] = item + 1;
        }
        item++;
    }

    if (n < 7) return e;

    e.id_estacion = atoi(campos[0]);
    strncpy(e.nombre, campos[1], sizeof(e.nombre) - 1);
    strncpy(e.direccion, campos[2], sizeof(e.direccion) - 1);
    e.coord_x = (float)atof(campos[3]);
    e.coord_y = (float)atof(campos[4]);
    e.capacidad_max = atoi(campos[5]);
    e.disponibilidad_actual = atoi(campos[6]);

    return e;
}