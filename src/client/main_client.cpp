#include "protocolo.h"
#include "gestor_config.h"
#include "CacheManager.hpp"
#include "Network.hpp"
#include "RentalSession.hpp"

int main(void) {
    RentalSession rentalSession;
    printf("\n  Conectando al servidor...\n");

    config_cargar("data/config.conf");
    if (net_conectar(g_config.ip_servidor, g_config.puerto) != 0) {
        printf("  Error: no se pudo conectar al servidor.\n");
        printf("  Asegurate de que KinetiX_Server esta en ejecucion.\n\n");
        return 1;
    }

    printf("  Conexion establecida.\n\n");

    if (rentalSession.menu_autenticar()) {
        CacheManager cache;

        rentalSession.menu_principal();

        // Enviamos EXIT al cerrar sesion
        char resp[32];
        net_cmd(CMD_EXIT, resp, sizeof(resp));
    }

    net_desconectar();

    return 0;
}
