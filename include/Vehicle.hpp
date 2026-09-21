#ifndef KINETIX_VEHICLE_HPP
#define KINETIX_VEHICLE_HPP

#include <memory>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//Clase vehiculo abstracta
class Vehiculo {
public:
    int id_vehiculo;
    char tipo;
    float bateria;
    int id_estacion;
    char estado;

    Vehiculo(int id, char tipo, float bateria, int id_estacion, char estado);
    virtual ~Vehiculo() = default;

    //Metodos abstractos
    virtual float getTarifaMinuto() const = 0;
    virtual void  getTipoNombre(char *buf, int tam) const = 0;

    //Metodos virtuales
    virtual float calcularCoste(double minutos) const;
    virtual int estaDisponible() const;
    virtual void descripcion(char *buf, int tam) const;

    //Construye Bicicleta o Patinete
    static std::unique_ptr<Vehiculo> fromString(const char *linea);

protected:
    void estadoLegible(char *buf, int tam) const;
};


#endif //KINETIX_VEHICLE_HPP