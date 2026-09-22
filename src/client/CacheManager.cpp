#include "CacheManager.hpp"
#include "Network.hpp"

void CacheManager::cache_cargar() {
    estaciones.clear();
    vehiculos.clear();

    // Estaciones
    net_enviar(CMD_LIST_ESTACIONES "\n");
    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n_est = atoi(buf);

    for (int i = 0; i < n_est; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        auto estOp = Estacion::fromString(linea);

        if (estOp.has_value()) {
            estaciones.push_back(estOp.value());
        }
    }

    // Vehiculos
    net_enviar(CMD_LIST_VEHICULOS "\n");
    net_recibir_linea(buf, sizeof(buf));
    int n_veh = atoi(buf);

    for (int i = 0; i < n_veh; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));
        auto v = Vehiculo::fromString(linea);
        if (v) {
            int id = v->id_vehiculo;
            vehiculos[id] = std::move(v);
        }
    }

    valida = true;
}

void CacheManager::cache_invalidar() {
    valida = false;
}

void CacheManager::cache_asegurar() {
    if (!valida) cache_cargar();
}

const std::vector<Estacion>& CacheManager::getEstaciones() const {
    return estaciones;
}

const std::unordered_map<int, std::unique_ptr<Vehiculo>>& CacheManager::getVehiculos() const {
    return vehiculos;
}