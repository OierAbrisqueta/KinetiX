//
// Created by jon.i on 26/03/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gestor_log.h"

static FILE *log_file = NULL;
static char  log_ruta[512] = "data/activity.log";

static void timestamp(char *buf, int max) {
    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);
    strftime(buf, max, "%Y-%m-%d %H:%M:%S", t);
}

int log_inicializar(const char *ruta) {
    if (ruta && strlen(ruta) > 0) strncpy(log_ruta, ruta, sizeof(log_ruta)-1);
    log_file = fopen(log_ruta, "a");
    if (!log_file) { printf("[LOG] Error abriendo '%s'.\n", log_ruta); return -1; }
    char ts[32]; timestamp(ts, sizeof(ts));
    fprintf(log_file, "[%s] [INFO] ======= Sesion iniciada =======\n", ts);
    fflush(log_file);
    printf("[LOG] Log activo en '%s'.\n", log_ruta);
    return 0;
}

void log_cerrar(void) {
    if (log_file) {
        char ts[32]; timestamp(ts, sizeof(ts));
        fprintf(log_file, "[%s] [INFO] ======= Sesion cerrada =======\n\n", ts);
        fclose(log_file); log_file = NULL;
    }
}

void log_escribir(const char *nivel, const char *mensaje) {
    char ts[32]; timestamp(ts, sizeof(ts));
    if (log_file) { fprintf(log_file, "[%s] [%-6s] %s\n", ts, nivel, mensaje); fflush(log_file); }
    if (strcmp(nivel, LOG_ERROR) == 0 || strcmp(nivel, LOG_AVISO) == 0)
        printf("[%s] %s\n", nivel, mensaje);
}

void log_mostrar_ultimas(int n) {
    if (n <= 0) n = 20;
    FILE *f = fopen(log_ruta, "r");
    if (!f) { printf("  No se pudo leer el log.\n"); return; }
    char **lines = (char**)malloc(n * sizeof(char*));
    for (int i = 0; i < n; i++) { lines[i] = (char*)malloc(512); lines[i][0] = '\0'; }
    int idx = 0, total = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) { strncpy(lines[idx % n], buf, 511); idx++; total++; }
    fclose(f);
    int mostrar = total < n ? total : n;
    int inicio  = total < n ? 0 : (idx % n);
    printf("\n  --- Ultimas %d entradas ---\n", mostrar);
    for (int i = 0; i < mostrar; i++) {
        int pos = (inicio + i) % n;
        int len = (int)strlen(lines[pos]);
        if (len > 0 && lines[pos][len-1] == '\n') lines[pos][len-1] = '\0';
        printf("  %s\n", lines[pos]);
    }
    printf("  ---------------------------\n\n");
    for (int i = 0; i < n; i++) free(lines[i]);
    free(lines);
}