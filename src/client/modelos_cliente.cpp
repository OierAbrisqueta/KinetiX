#include "modelos_cliente.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//Vehiculo
Vehiculo::Vehiculo(int id, char tipo, float bateria, int id_estacion, char estado) {
    this->id_vehiculo = id;
    this->tipo = tipo;
    this->bateria = bateria;
    this->id_estacion = id_estacion;
    this->estado = estado;
}

virtual float Vehiculo::calcularCoste(double minutos) const {
    return minutos * getTarifaMinuto();
}

virtual int Vehiculo::estaDisponible() const {
    return this->estado == 'D' && this->bateria > 15;
}
virtual void Vehiculo::descripcion(char *buf, int tam) const {
    char tipo_nombre[16];
    char estado_nombre[16];
    getTipoNombre(tipo_nombre, sizeof(tipo_nombre));
    estadoLegible(estado_nombre, sizeof(estado_nombre));

    snprintf(buf, tam,
             "%d, %s, Bateria: %d%%, Estado: %s, Estacion: %d",
             id_vehiculo,
             tipo_nombre,
             (int)bateria,
             estado_nombre,
             id_estacion);
}

//IAG: Método generado con inteligencia artificial
static std::unique_ptr<Vehiculo> Vehiculo::fromString(const char *linea) {
    int id = 0;
    char tipo[2] = {0};
    float bateria = 0.0f;
    int id_estacion = 0;
    char estado[2] = {0};

    int campos = sscanf(linea, "%d|%1[BP]|%f|%d|%1[DRMB]",
                        &id, tipo, &bateria, &id_estacion, estado);

    if (campos != 5) return nullptr;

    if (tipo[0] == 'B')
        return std::make_unique<Bicicleta>(id, bateria, id_estacion, estado[0]);
    if (tipo[0] == 'P')
        return std::make_unique<Patinete> (id, bateria, id_estacion, estado[0]);

    return nullptr;
}

void Vehiculo::estadoLegible(char *buf, int tam) const {
    switch (estado) {
        case 'D': strncpy(buf, "Disponible", tam); break;
        case 'R': strncpy(buf, "Reservado", tam); break;
        case 'M': strncpy(buf, "Mantenimiento", tam); break;
        case 'B': strncpy(buf, "Baja", tam); break;
        default: strncpy(buf, "Desconocido", tam); break;
    }
    buf[tam - 1] = '\0';
}