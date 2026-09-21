#ifndef KINETIX_ESTACION_HPP
#define KINETIX_ESTACION_HPP

#include <string>
#include <iostream>
#include <String.h>
#include <optional>

class Estacion {
public:
    Estacion(int id_estacion, char* nombre, char* direccion,
        float coord_x, float coord_y, int capacidad_maxima, int disponibilidad_actual);
    virtual ~Estacion() = default;

    int getIdEstacion() const;
    float getOcupacion() const;
    void descripcion(char *buf, int tam) const;
    const std::string& getNombre() const;
    const std::string& getDireccion() const;
    float getCoordX() const;
    float getCoordY() const;
    int getCapacidadMaxima() const;
    int getDisponibilidadActual() const;

    static std::optional<Estacion> fromString(const char *linea);

private:
    int id_estacion;
    std::string nombre;
    std::string direccion;
    float coord_x;
    float coord_y;
    int capacidad_max;
    int disponibilidad_actual;
};

#endif //KINETIX_ESTACION_HPP