//
// Created by jon.i on 26/03/2026.
//

#ifndef KINETIX_GESTOR_LOG_H
#define KINETIX_GESTOR_LOG_H

#define LOG_INFO   "INFO"
#define LOG_AVISO  "AVISO"
#define LOG_ERROR  "ERROR"
#define LOG_ACCESO "ACCESO"

int  log_inicializar(const char *ruta);
void log_cerrar(void);
void log_escribir(const char *nivel, const char *mensaje);
void log_mostrar_ultimas(int n);

#define LOG_I(msg)  log_escribir(LOG_INFO,   (msg))
#define LOG_A(msg)  log_escribir(LOG_AVISO,  (msg))
#define LOG_E(msg)  log_escribir(LOG_ERROR,  (msg))
#define LOG_AC(msg) log_escribir(LOG_ACCESO, (msg))

#endif