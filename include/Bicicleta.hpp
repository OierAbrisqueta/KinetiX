#ifndef KINETIX_BICICLETA_HPP
#define KINETIX_BICICLETA_HPP

#include "Vehicle.hpp"

class Bicicleta: public Vehiculo {
public:
    Bicicleta(int id, float bateria, int id_estacion, char estado);

    float getTarifaMinuto() const override;
    void  getTipoNombre(char *buf, int tam) const override;
};

#endif //KINETIX_BICICLETA_HPP