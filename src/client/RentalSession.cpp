#include "RentalSession.hpp"

RentalSession::RentalSession(): id_usuario(0), saldo(0.0f), alquiler_activo(0),
    vehiculo_activo(0), tipo_vehiculo_activo('\0') {}

int RentalSession::menu_autenticar() {
    char dni[32], clave[64];
    int intentos = 3;

    while (intentos > 0) {
        ConsoleUI::limpiar();
        ConsoleUI::menu_banner();
        printf("  Acceso restringido. Identifiquese.\n\n");

        ConsoleUI::ui_leer_string("DNI", dni, sizeof(dni));
        ConsoleUI::ui_leer_string("Contrasena", clave, sizeof(clave));

        // Enviamos el login al servidor
        char comando[256];
        snprintf(comando, sizeof(comando), CMD_LOGIN " %s|%s", dni, clave);

        char resp[64];
        net_cmd(comando, resp, sizeof(resp));

        if (strcmp(resp, RESP_OK) == 0) {
            // Login correcto: buscamos los datos del usuario para guardarlos
            net_enviar(CMD_LIST_USUARIOS "\n");
            char buf[32];
            net_recibir_linea(buf, sizeof(buf));
            int n = atoi(buf);

            for (int i = 0; i < n; i++) {
                char linea[PROTO_BUFF_SIZE];
                net_recibir_linea(linea, sizeof(linea));
                int id; char udni[32], unombre[64]; float saldo;
                if (sscanf(linea, "%d|%31[^|]|%63[^|]|%f", &id, udni, unombre, &saldo) == 4) {
                    if (strcmp(udni, dni) == 0) {
                        this->id_usuario = id;
                        this->dni = udni;
                        this->nombre = unombre;
                        this->saldo = saldo;
                    }
                }
            }

            printf("\n  Bienvenido, %s.\n", this->nombre.c_str());
            ConsoleUI::pausa();
            sincronizar_estado_inicial();
            return 1;
        }

        intentos--;
        printf("\n  Credenciales incorrectas. Intentos restantes: %d\n", intentos);
        if (intentos > 0) ConsoleUI::pausa();
    }

    printf("\n  Demasiados intentos fallidos. El programa se cerrara.\n\n");
    return 0;
}

void RentalSession::menu_principal() {
    int opcion;
    do {
        ConsoleUI::limpiar();
        refrescar_saldo();
        ConsoleUI::menu_banner();
        printf("  Bienvenido, %s  |  Saldo: %.2f EUR\n", this->nombre.c_str(), this->saldo);
        if (this->alquiler_activo)
            printf("  Alquiler activo: ID %d\n", this->alquiler_activo);
        printf("\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Ver estaciones\n");
        printf("    [ 2 ]  Ver vehiculos disponibles\n");
        printf("    [ 3 ]  Alquilar vehiculo\n");
        printf("    [ 4 ]  Devolver vehiculo\n");
        printf("    [ 5 ]  Mis alquileres\n");
        printf("    [ 6 ]  Consultar saldo y estado\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Cerrar sesion\n");
        printf("\n");

        opcion = ConsoleUI::ui_leer_int("Seleccione opcion", 0, 6);

        switch (opcion) {
            case 1: ver_estaciones(); break;
            case 2: ver_vehiculos_disponibles(); break;
            case 3: alquilar(); break;
            case 4: devolver(); break;
            case 5: mis_alquileres(); break;
            case 6: consultar_estado(); break;
            case 0:
                printf("\n  ................................................\n");
                printf("  Hasta pronto, %s!\n\n", this->nombre.c_str());
                break;
            default:
                printf("  Opcion no valida.\n");
                ConsoleUI::pausa();
                break;
        }
    } while (opcion != 0);
}

void RentalSession::refrescar_saldo() {
    if (this->id_usuario <= 0) return;

    char comando[64];
    snprintf(comando, sizeof(comando), CMD_GET_USUARIO " %d\n", this->id_usuario);
    net_enviar(comando);

    char resp[64];
    net_recibir_linea(resp, sizeof(resp));
    if (strcmp(resp, RESP_OK) != 0) return;

    char linea[PROTO_BUFF_SIZE];
    net_recibir_linea(linea, sizeof(linea));

    int id = 0;
    char dni[32];
    char nombre[64];
    float saldo = 0.0f;
    if (sscanf(linea, "%d|%31[^|]|%63[^|]|%f", &id, dni, nombre, &saldo) == 4 && id == this->id_usuario) {
        this->saldo = saldo;
    }
}

void RentalSession::ver_estaciones() {
    ConsoleUI::limpiar();
    printf("\n  --- Listado de estaciones ---\n");
    printf("  ................................................\n\n");

    cache.cache_cargar();
    const auto& estaciones = cache.getEstaciones();

    if (estaciones.empty()) {
        printf("  No hay estaciones registradas.\n");
        ConsoleUI::pausa();
        return;
    }

    printf("  %-6s  %-25s  %-10s  %s\n", "ID", "Nombre", "Libres", "Ocupacion");
    printf("  ------  -------------------------  ----------  ---------\n");

    for (int i = 0; i < (int)estaciones.size(); i++) {
        const Estacion &e = estaciones[i];
        char libres[16];

        snprintf(libres, sizeof(libres), "%d/%d",
                 e.getDisponibilidadActual(), e.getCapacidadMaxima());

        printf("  %-6d  %-25s  %-10s  %d%%\n",
               e.getIdEstacion(), e.getNombre().c_str(), libres, (int)e.getOcupacion());
    }

    printf("  ................................................\n");
    printf("  Total: %d estaciones.\n", (int)estaciones.size());
    ConsoleUI::pausa();
}

void RentalSession::ver_vehiculos_disponibles() {
    ConsoleUI::limpiar();
    printf("\n  --- Vehiculos disponibles ---\n");
    printf("  ................................................\n\n");

    cache.cache_asegurar();
    const auto& vehiculos = cache.getVehiculos();

    if (vehiculos.empty()) {
        printf("  No hay vehículos registrados.\n");
        ConsoleUI::pausa();
        return;
    }

    printf("  %-6s  %-12s  %-10s  %-10s  %s\n",
           "ID", "Tipo", "Bateria", "Estacion", "Tarifa/min");
    printf("  ------  ------------  ----------  ----------  ----------\n");

    int mostrados = 0;

    for (const auto &par : vehiculos) {
        const auto &v = par.second;
        if (v->estaDisponible()) {
            char tipo_nombre[16];
            v->getTipoNombre(tipo_nombre, sizeof(tipo_nombre));
            printf("  %-6d  %-12s  %-9.1f%%  %-10d  %.2f EUR\n",
                   v->id_vehiculo, tipo_nombre, v->bateria,
                   v->id_estacion, v->getTarifaMinuto());
            mostrados++;
        }
    }

    if (mostrados == 0)
        printf("  No hay vehiculos disponibles en este momento.\n");

    printf("  ................................................\n");
    printf("  Total disponibles: %d vehiculos.\n", mostrados);
    ConsoleUI::pausa();
}

void RentalSession::alquilar() {
    ConsoleUI::limpiar();
    printf("\n  --- Alquilar vehiculo ---\n");
    printf("  ................................................\n\n");

    if (this->alquiler_activo != 0) {
        printf("  Ya tienes un alquiler en curso (ID: %d).\n", this->alquiler_activo);
        printf("  Devuelve el vehiculo actual antes de alquilar otro.\n");
        ConsoleUI::pausa();
        return;
    }

    if (this->saldo <= 0.0f) {
        printf("  Saldo insuficiente (%.2f EUR).\n", this->saldo);
        printf("  Recarga tu saldo para poder alquilar.\n");
        ConsoleUI::pausa();
        return;
    }

    cache.cache_asegurar();
    const auto& estaciones = cache.getEstaciones();
    const auto& vehiculos = cache.getVehiculos();

    if (estaciones.empty()) {
        printf("  No hay estaciones disponibles.\n");
        ConsoleUI::pausa();
        return;
    }

    printf("  %-6s  %-26s  %s\n", "ID", "Nombre", "Disponibles");
    printf("  ------  --------------------------  -----------\n");

    int max_id_est = 1;

    for (int i = 0; i < (int)estaciones.size(); i++) {
        const Estacion &e = estaciones[i];

        printf("  %-6d  %-26s  %d/%d\n",
               e.getIdEstacion(), e.getNombre().c_str(),
               e.getDisponibilidadActual(), e.getCapacidadMaxima());

        if (e.getIdEstacion() > max_id_est) max_id_est = e.getIdEstacion();
    }

    int id_estacion = -1;
    while (id_estacion == -1) {
        printf("\n");
        int elegido = ConsoleUI::ui_leer_int("Selecciona una estacion (ID)", 1, max_id_est);
        for (int i = 0; i < estaciones.size(); i++) {
            const Estacion &e = estaciones[i];
            if (e.getIdEstacion() == elegido) {
                id_estacion = elegido;
                break;
            }
        }
        if (id_estacion == -1)
            printf("  Error: ese ID no corresponde a ninguna estacion de la lista.\n");
    }

    //Mostrar vehiculos de esa estacion
    ConsoleUI::limpiar();
    printf("\n  --- Vehiculos disponibles en estacion %d ---\n", id_estacion);
    printf("  ................................................\n\n");

    printf("  %-6s  %-12s  %-10s  %s\n", "ID", "Tipo", "Bateria", "Tarifa/min");
    printf("  ------  ------------  ----------  ----------\n");

    int max_id_veh = 1;
    int disponibles = 0;
    for (const auto &par : vehiculos) {
        const auto &v = par.second;
        if (v->id_estacion == id_estacion && v->estaDisponible()) {
            char tipo_nombre[16];
            v->getTipoNombre(tipo_nombre, sizeof(tipo_nombre));
            printf("  %-6d  %-12s  %-9.1f%%  %.2f EUR\n",
                   v->id_vehiculo, tipo_nombre,
                   v->bateria, v->getTarifaMinuto());
            if (v->id_vehiculo > max_id_veh) max_id_veh = v->id_vehiculo;
            disponibles++;
        }
    }

    if (disponibles == 0) {
        printf("\n  No hay vehiculos disponibles en esta estacion.\n");
        ConsoleUI::pausa();
        return;
    }

    //Elegir vehiculo y confirmar
    printf("\n");

    int id_vehiculo = -1;
    while (id_vehiculo == -1) {
        int elegido = ConsoleUI::ui_leer_int("ID del vehiculo a alquilar", 1, max_id_veh);
        auto it = vehiculos.find(elegido);
        if (it != vehiculos.end()
            && it->second->id_estacion == id_estacion
            && it->second->estaDisponible()) {
            id_vehiculo = elegido;
            } else {
                printf("  Error: ese ID no corresponde a ningun vehiculo disponible en esta estacion.\n");
            }
    }

    char comando[128];
    snprintf(comando, sizeof(comando), CMD_ALQUILAR " %d|%d|%d",
             this->id_usuario, id_vehiculo, id_estacion);

    char resp[64];
    net_cmd(comando, resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, strlen(RESP_OK)) == 0) {
        int id_alquiler = 0;
        sscanf(resp, "OK %d", &id_alquiler);
        this->alquiler_activo = id_alquiler;
        this->vehiculo_activo = id_vehiculo;
        this->tipo_vehiculo_activo = vehiculos.at(id_vehiculo)->tipo;

        cache.cache_invalidar();

        printf("\n  Alquiler iniciado correctamente.\n");
        printf("  ID de alquiler: %d\n", id_alquiler);
        printf("  Buen viaje!\n");
    } else {
        printf("\n  Error al iniciar el alquiler.\n");
        printf("  Comprueba que el ID del vehiculo sea correcto.\n");
    }
    ConsoleUI::pausa();
}

void RentalSession::devolver() {
    ConsoleUI::limpiar();
    printf("\n  --- Devolver vehiculo ---\n");
    printf("  ................................................\n\n");

    if (this->alquiler_activo == 0) {
        printf("  No tienes ningun alquiler en curso.\n");
        ConsoleUI::pausa();
        return;
    }

    printf("  Alquiler activo  : ID %d\n", this->alquiler_activo);
    printf("  Vehiculo en uso  : ID %d\n\n", this-vehiculo_activo);

    cache.cache_asegurar();
    const auto& estaciones = cache.getEstaciones();

    printf("  %-6s  %-26s  %s\n", "ID", "Nombre", "Libres");
    printf("  ------  --------------------------  -------\n");
    for (int i = 0; i < estaciones.size(); i++) {
        const Estacion &e = estaciones[i];
        printf("  %-6d  %-26s  %d/%d\n",
               e.getIdEstacion(), e.getNombre().c_str(),
               e.getDisponibilidadActual(), e.getCapacidadMaxima());
    }
    printf("\n");

    int id_estacion = ConsoleUI::ui_leer_int("ID de la estacion de destino", 1, 99999);

    // Confirmacion antes de devolver
    printf("\n  Confirmar devolucion en estacion %d (s/n): ", id_estacion);
    char conf[8];
    fgets(conf, sizeof(conf), stdin);
    if (conf[0] != 's' && conf[0] != 'S') {
        printf("  Operacion cancelada.\n");
        ConsoleUI::pausa();
        return;
    }

    char comando[128];
    snprintf(comando, sizeof(comando), CMD_DEVOLVER " %d|%d",
             this->alquiler_activo, id_estacion);

    char resp[64];
    net_cmd(comando, resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, strlen(RESP_OK)) == 0) {
        printf("\n  Vehiculo devuelto correctamente.\n");
        this->alquiler_activo = 0;
        this->vehiculo_activo = 0;
        this->tipo_vehiculo_activo = '\0';

        cache.cache_invalidar();
        refrescar_saldo();
    } else {
        printf("\n  Error al devolver el vehiculo. Intentalo de nuevo.\n");
    }
    ConsoleUI::pausa();
}

void RentalSession::mis_alquileres() {
    ConsoleUI::limpiar();
    printf("\n  --- Mis alquileres ---\n");
    printf("  ................................................\n\n");

    net_enviar(CMD_LIST_ALQUILERES "\n");

    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    printf("  %-6s  %-8s  %-19s  %-19s  %s\n",
           "ID", "Vehic.", "Inicio", "Fin", "Coste");
    printf("  ------  --------  -------------------  -------------------  --------\n");

    int total = 0;
    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        int id_al, id_u, id_v, est_o, est_d;
        char f_ini[32] = {0}, f_fin[32] = {0};
        float coste = 0.0f;
        sscanf(linea, "%d|%d|%d|%d|%d|%[^|]|%[^|]|%f",
               &id_al, &id_u, &id_v, &est_o, &est_d, f_ini, f_fin, &coste);

        if (id_u == this->id_usuario) {
            const char *fin = (strlen(f_fin) == 0 || strcmp(f_fin, "-") == 0)
                              ? "En curso" : f_fin;
            printf("  %-6d  %-8d  %-19s  %-19s  %.2f EUR\n",
                   id_al, id_v, f_ini, fin, coste);
            total++;
        }
    }

    if (total == 0) printf("  No tienes alquileres registrados.\n");
    printf("  ................................................\n");
    printf("  Total: %d alquileres.\n", total);
    ConsoleUI::pausa();
}

void RentalSession::sincronizar_estado_inicial() {
    net_enviar(CMD_LIST_ALQUILERES "\n");
    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        int id_al, id_u, id_v, est_o, est_d;
        char f_ini[32] = {0}, f_fin[32] = {0};
        float coste;
        sscanf(linea, "%d|%d|%d|%d|%d|%[^|]|%[^|]|%f",
               &id_al, &id_u, &id_v, &est_o, &est_d, f_ini, f_fin, &coste);

        if (id_u == this->id_usuario &&
            (strlen(f_fin) == 0 || strcmp(f_fin, "-") == 0)) {
            this->alquiler_activo = id_al;
            this->vehiculo_activo = id_v;

            // Recuperar tipo del vehiculo desde el servidor
            char cmd[64], resp[PROTO_BUFF_SIZE];
            snprintf(cmd, sizeof(cmd), CMD_GET_VEHICULO " %d\n", id_v);
            net_enviar(cmd);
            net_recibir_linea(resp, sizeof(resp));
            if (strcmp(resp, RESP_OK) == 0) {
                char vdata[PROTO_BUFF_SIZE];
                net_recibir_linea(vdata, sizeof(vdata));
                char tipo_s[2] = {0};
                int vid, vest, vbat_dummy;
                char vestado_s[2] = {0};
                sscanf(vdata, "%d|%1[BP]|%*f|%d|%1[DRMB]",
                       &vid, tipo_s, &vest, vestado_s);
                if (tipo_s[0] != '\0')
                    this->tipo_vehiculo_activo = tipo_s[0];
            }
            }
    }
}

void RentalSession::consultar_estado() {
    ConsoleUI::limpiar();
    printf("\n  --- Estado de tu cuenta ---\n");
    printf("  ................................................\n\n");

    refrescar_saldo();
    printf("  Saldo actual     : %.2f EUR\n", this->saldo);

    if (this->alquiler_activo == 0) {
        printf("  Alquiler activo  : Ninguno\n");
        ConsoleUI::pausa();
        return;
    }

    printf("  Alquiler activo  : ID %d\n", this->alquiler_activo);
    printf("  Vehiculo en uso  : ID %d\n\n", this->vehiculo_activo);

    char cmd[64], resp[256];
    snprintf(cmd, sizeof(cmd), CMD_STAT " %d\n", this->vehiculo_activo);
    net_enviar(cmd);
    net_recibir_linea(resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, 2) == 0) {
        float bateria = 0;
        double minutos = 0, km = 0;
        sscanf(resp + 3, "%f|%lf|%lf", &bateria, &minutos, &km);
        float tarifa = (this->tipo_vehiculo_activo == 'P') ? 0.07f : 0.05f;
        float coste = (float)(minutos * tarifa);
        printf("  Bateria          : %.1f%%\n", bateria);
        printf("  Duracion         : %.0f min\n", minutos);
        printf("  Km estimados     : %.1f km\n", km);
        printf("  Coste acumulado  : %.2f EUR\n", coste);
    }
    ConsoleUI::pausa();
}