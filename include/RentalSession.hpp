#ifndef KINETIX_RENTALSESSION_HPP
#define KINETIX_RENTALSESSION_HPP

#include <string>
#include "CacheManager.hpp"
#include "protocolo.h"
#include "ConsoleUI.hpp"
#include "Network.hpp"

class RentalSession {
public:
    RentalSession();
    ~RentalSession() = default;

    int menu_autenticar();
    void menu_principal();

private:
    int id_usuario;
    std::string nombre;
    std::string dni;
    float saldo;

    int alquiler_activo;
    int vehiculo_activo;
    char tipo_vehiculo_activo;

    CacheManager cache;

    void refrescar_saldo();
    void ver_estaciones();
    void ver_vehiculos_disponibles();
    void alquilar();
    void devolver();
    void mis_alquileres();
    void sincronizar_estado_inicial();
    void consultar_estado();

};

#endif //KINETIX_RENTALSESSION_HPP