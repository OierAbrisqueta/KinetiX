#ifndef KINETIX_CACHE_MANAGER_CPP_HPP
#define KINETIX_CACHE_MANAGER_CPP_HPP

#include <vector>
#include <unordered_map>
#include "Estacion.hpp"
#include "Vehicle.hpp"

class CacheManager {
public:
    CacheManager(): valida(false){}

    void cache_cargar();
    void cache_invalidar();
    void cache_asegurar();

    const std::vector<Estacion>& getEstaciones() const;
    const std::unordered_map<int, std::unique_ptr<Vehiculo>>& getVehiculos() const;

private:
    std::vector<Estacion> estaciones;
    std::unordered_map<int, std::unique_ptr<Vehiculo>> vehiculos;
    bool valida = false;
};

#endif //KINETIX_CACHE_MANAGER_CPP_HPP