#ifndef KINETIX_MODELOS_CLIENTE_H
#define KINETIX_MODELOS_CLIENTE_H

#include "Vehicle.hpp"

//Patinete
class Patinete: public Vehiculo {
public:
    Patinete(int id, float bateria, int id_estacion, char estado);

    float getTarifaMinuto() const override;
    void  getTipoNombre(char *buf, int tam) const override;
};


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