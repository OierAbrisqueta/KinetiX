#include "Estacion.hpp"

Estacion::Estacion(int id_estacion, char* nombre, char* direccion,
        float coord_x, float coord_y, int capacidad_maxima, int disponibilidad_actual)
    : id_estacion(id_estacion),
    nombre(nombre),
    direccion(direccion),
    coord_x(coord_x),
    coord_y(coord_y),
    capacidad_max(capacidad_maxima),
    disponibilidad_actual(disponibilidad_actual) {

}

int Estacion::getIdEstacion() const {
    return id_estacion;
}

float Estacion::getOcupacion() const {
    if (capacidad_max == 0) return 0;
    float devolver = 100.0f * (float)(capacidad_max - disponibilidad_actual)/(float)capacidad_max;
    return devolver;
}

const std::string &Estacion::getNombre() const {
    return nombre;
}

const std::string& Estacion::getDireccion() const {
    return direccion;
}

float Estacion::getCoordX() const {
    return coord_x;
}

float Estacion::getCoordY() const {
    return coord_y;
}

int Estacion::getCapacidadMaxima() const {
    return capacidad_max;
}

int Estacion::getDisponibilidadActual() const {
    return disponibilidad_actual;
}

void  Estacion::descripcion(char *buf, int tam) const {
    std::cout << id_estacion << ", " << nombre << ", Libres: " <<
        disponibilidad_actual << ", Ocupacion: " << getOcupacion();
}

std::optional<Estacion> Estacion::fromString(const char *linea) {
    if (linea == nullptr) return std::nullopt;

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

    if (n < 7) return std::nullopt;

    return Estacion(
        atoi(campos[0]),
        campos[1],
        campos[2],
        (float)atof(campos[3]),
        (float)atof(campos[4]),
        atoi(campos[5]),
        atoi(campos[6])
    );
}