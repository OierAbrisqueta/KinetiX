#include "Bicicleta.hpp"

Bicicleta::Bicicleta(int id, float bateria, int id_estacion, char estado)
    : Vehiculo(id, 'B', bateria, id_estacion, estado) {}

float Bicicleta::getTarifaMinuto() const {
    return 0.05f;
}

void  Bicicleta::getTipoNombre(char *buf, int tam) const {
    strncpy(buf, "Bicicleta", tam);
    buf[tam - 1] = '\0';
}