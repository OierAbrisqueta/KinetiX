#ifndef KINETIX_PATINETE_HPP
#define KINETIX_PATINETE_HPP

#include "Vehicle.hpp"

class Patinete: public Vehiculo {
public:
    Patinete(int id, float bateria, int id_estacion, char estado);

    float getTarifaMinuto() const override;
    void  getTipoNombre(char *buf, int tam) const override;
};

#endif //KINETIX_PATINETE_HPP