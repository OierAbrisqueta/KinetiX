#ifndef KINETIX_GESTOR_CONFIG_H
#define KINETIX_GESTOR_CONFIG_H

#define CONFIG_MAX_VALOR  256
#define CONFIG_RUTA_DEFAULT "data/config.conf"

typedef struct {
    int    puerto;
    int    max_conexiones;
    char   ip_servidor[CONFIG_MAX_VALOR];
    char   admin_usuario[CONFIG_MAX_VALOR];
    char   admin_clave[CONFIG_MAX_VALOR];
    char   db_ruta[CONFIG_MAX_VALOR];
    char   log_ruta[CONFIG_MAX_VALOR];
    char   reportes_ruta[CONFIG_MAX_VALOR];
    int    bateria_minima;
    float  tarifa_bici_min;
    float  tarifa_patinete_min;
    int    cargado;
} Config;

extern Config g_config;

int  config_cargar(const char *ruta);
void config_mostrar(void);

#endif