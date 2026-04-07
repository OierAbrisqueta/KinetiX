#include <stdio.h>
#include <stdlib.h>
#include "gestor_config.h"
#include "gestor_log.h"
#include "gestor_bd.h"
#include "menu.h"

int main(int argc, char *argv[]) {

    const char *ruta_conf = (argc > 1) ? argv[1] : CONFIG_RUTA_DEFAULT;
    if (config_cargar(ruta_conf) != 0)
        printf("[ARRANQUE] Continuando con valores por defecto.\n");

    if (log_inicializar(g_config.log_ruta) != 0)
        printf("[ARRANQUE] Aviso: log no disponible.\n");

    if (conectar_bd(g_config.db_ruta) != 0) {
        LOG_E("No se pudo conectar con la BD. Abortando.");
        log_cerrar();
        return EXIT_FAILURE;
    }
    LOG_I("Conexion con la base de datos establecida.");

    if (!menu_autenticar()) {
        cerrar_bd();
        log_cerrar();
        return EXIT_FAILURE;
    }

    menu_principal();

    cerrar_bd();
    log_cerrar();
    return EXIT_SUCCESS;
}