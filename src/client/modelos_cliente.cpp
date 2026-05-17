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

float Vehiculo::calcularCoste(double minutos) const {
    return minutos * getTarifaMinuto();
}

int Vehiculo::estaDisponible() const {
    return this->estado == 'D' && this->bateria > 15;
}
void Vehiculo::descripcion(char *buf, int tam) const {
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

//IAG: Metodo generado con inteligencia artificial
std::unique_ptr<Vehiculo> Vehiculo::fromString(const char *linea) {
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

//bicicleta
Bicicleta::Bicicleta(int id, float bateria, int id_estacion, char estado)
    : Vehiculo(id, 'B', bateria, id_estacion, estado) {}

float Bicicleta::getTarifaMinuto() const {
    return 0.05f;
}

void  Bicicleta::getTipoNombre(char *buf, int tam) const {
    strncpy(buf, "Bicicleta", tam);
    buf[tam - 1] = '\0';
}

//Patinete
Patinete::Patinete(int id, float bateria, int id_estacion, char estado)
    : Vehiculo(id, 'P', bateria, id_estacion, estado) {}

float Patinete::getTarifaMinuto() const {
    return 0.07f;
}

void  Patinete::getTipoNombre(char *buf, int tam) const {
    strncpy(buf, "Patinete", tam);
    buf[tam - 1] = '\0';
}

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

static Estacion fromString(const char *linea) {
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