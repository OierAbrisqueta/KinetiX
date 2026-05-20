#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gestor_config.h"

Config g_config;

static void trim(char *s) {
    if (!s) return;
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\n' ||
                        s[len-1] == '\r' || s[len-1] == '\t'))
        s[--len] = '\0';
    char *inicio = s;
    while (*inicio == ' ' || *inicio == '\t') inicio++;
    if (inicio != s) memmove(s, inicio, strlen(inicio) + 1);
}

static int parsear_linea(const char *linea, char *clave, char *valor) {
    if (linea[0] == '#' || linea[0] == '\n' || linea[0] == '\0') return 0;
    const char *sep = strchr(linea, '=');
    if (!sep) return 0;
    int len = (int)(sep - linea);
    strncpy(clave, linea, len); clave[len] = '\0';
    strncpy(valor, sep + 1, CONFIG_MAX_VALOR - 1);
    valor[CONFIG_MAX_VALOR - 1] = '\0';
    trim(clave); trim(valor);
    return (strlen(clave) > 0);
}

static void aplicar(const char *clave, const char *valor) {
    if      (strcmp(clave, "PUERTO")             == 0) g_config.puerto             = atoi(valor);
    else if (strcmp(clave, "MAX_CONEXIONES")     == 0) g_config.max_conexiones     = atoi(valor);
    else if (strcmp(clave, "IP_SERVIDOR")        == 0) strncpy(g_config.ip_servidor,      valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "ADMIN_USUARIO")      == 0) strncpy(g_config.admin_usuario,    valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "ADMIN_CLAVE")        == 0) strncpy(g_config.admin_clave,      valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "DB_RUTA")            == 0) strncpy(g_config.db_ruta,          valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "LOG_RUTA")           == 0) strncpy(g_config.log_ruta,         valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "REPORTES_RUTA")      == 0) strncpy(g_config.reportes_ruta,    valor, CONFIG_MAX_VALOR-1);
    else if (strcmp(clave, "BATERIA_MINIMA")     == 0) g_config.bateria_minima     = atoi(valor);
    else if (strcmp(clave, "TARIFA_BICI_MIN")    == 0) g_config.tarifa_bici_min    = (float)atof(valor);
    else if (strcmp(clave, "TARIFA_PATINETE_MIN")== 0) g_config.tarifa_patinete_min= (float)atof(valor);
}

int config_cargar(const char *ruta) {
    memset(&g_config, 0, sizeof(Config));
    // Valores por defecto
    g_config.puerto = 8080; g_config.max_conexiones = 10; g_config.bateria_minima = 15;
    g_config.tarifa_bici_min = 0.05f; g_config.tarifa_patinete_min = 0.07f;
    strncpy(g_config.ip_servidor,   "127.0.0.1",        CONFIG_MAX_VALOR-1);
    strncpy(g_config.db_ruta,       "data/KinetiX.sqlite",  CONFIG_MAX_VALOR-1);
    strncpy(g_config.log_ruta,      "data/activity.log",CONFIG_MAX_VALOR-1);
    strncpy(g_config.reportes_ruta, "data/reportes/",   CONFIG_MAX_VALOR-1);

    const char *r = (ruta && strlen(ruta) > 0) ? ruta : CONFIG_RUTA_DEFAULT;
    FILE *f = fopen(r, "r");
    if (!f) { printf("[CONFIG] No se encontro '%s'. Valores por defecto.\n", r); return -1; }

    char linea[512], clave[CONFIG_MAX_VALOR], valor[CONFIG_MAX_VALOR];
    int ok = 0;
    while (fgets(linea, sizeof(linea), f))
        if (parsear_linea(linea, clave, valor)) { aplicar(clave, valor); ok++; }
    fclose(f);
    g_config.cargado = 1;
    printf("[CONFIG] Cargados %d parametros desde '%s'.\n", ok, r);
    return 0;
}

void config_mostrar(void) {
    printf("\n  +------------------------------------------+\n");
    printf("  |     CONFIGURACION ACTUAL DEL SISTEMA     |\n");
    printf("  +------------------------------------------+\n");
    printf("  Puerto         : %d\n",   g_config.puerto);
    printf("  Max conexiones : %d\n",   g_config.max_conexiones);
    printf("  IP servidor    : %s\n",   g_config.ip_servidor);
    printf("  Base de datos  : %s\n",   g_config.db_ruta);
    printf("  Log            : %s\n",   g_config.log_ruta);
    printf("  Bateria minima : %d%%\n", g_config.bateria_minima);
    printf("  Tarifa bici    : %.2f EUR/min\n", g_config.tarifa_bici_min);
    printf("  Tarifa patinete: %.2f EUR/min\n", g_config.tarifa_patinete_min);
    printf("  +------------------------------------------+\n\n");
}