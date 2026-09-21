#include "Patinete.hpp"

Patinete::Patinete(int id, float bateria, int id_estacion, char estado)
    : Vehiculo(id, 'P', bateria, id_estacion, estado) {}

float Patinete::getTarifaMinuto() const {
    return 0.07f;
}

void  Patinete::getTipoNombre(char *buf, int tam) const {
    strncpy(buf, "Patinete", tam);
    buf[tam - 1] = '\0';
}