#ifndef KINETIX_MODELOS_CLIENTE_H
#define KINETIX_MODELOS_CLIENTE_H

#include <memory>

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


//Bicicleta
class Bicicleta: public Vehiculo {
public:
    Bicicleta(int id, float bateria, int id_estacion, char estado);

    float getTarifaMinuto() const override;
    void  getTipoNombre(char *buf, int tam) const override;
};


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