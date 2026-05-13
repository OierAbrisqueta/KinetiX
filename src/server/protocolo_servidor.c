#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "protocolo_servidor.h"
#include "protocolo.h"
#include "server_socket.h"
#include "gestor_bd.h"
#include "gestor_log.h"
#include "gestor_config.h"
#include "hash_utils.h"
#include "modelos.h"

/* =============================================================
 *  protocolo_servidor.c  —  Handlers de cada comando
 * ============================================================= */

/* ── Utilidades internas ────────────────────────────────────── */

static void resp_simple(kinetix_socket_t s, const char *resp) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%s\n", resp);
    servidor_enviar(s, buf);
}

static int split_campos(char *src, char *campos[], int max) {
    int n = 0;
    char *tok = strtok(src, PROTO_SEP_CAMPO);
    while (tok && n < max) {
        campos[n++] = tok;
        tok = strtok(NULL, PROTO_SEP_CAMPO);
    }
    return n;
}

/* ── Serializacion ──────────────────────────────────────────── */

static void serializar_estacion(const Estacion *e, char *buf, int tam) {
    snprintf(buf, tam, "%d|%s|%s|%.6f|%.6f|%d|%d\n",
             e->id_estacion, e->nombre, e->direccion,
             e->coord_x, e->coord_y,
             e->capacidad_max, e->disponibilidad_actual);
}

static void serializar_vehiculo(const Vehiculo *v, char *buf, int tam) {
    snprintf(buf, tam, "%d|%c|%.2f|%d|%c\n",
             v->id_vehiculo, v->tipo, v->bateria,
             v->id_estacion, v->estado);
}

static void serializar_usuario(const Usuario *u, char *buf, int tam) {
    snprintf(buf, tam, "%d|%s|%s|%.2f\n",
             u->id_usuario, u->dni, u->nombre, u->saldo);
}

static void serializar_alquiler(const Alquiler *a, char *buf, int tam) {
    snprintf(buf, tam, "%d|%d|%d|%d|%d|%s|%s|%.2f\n",
             a->id_alquiler, a->id_usuario, a->id_vehiculo,
             a->id_estacion_origen, a->id_estacion_destino,
             a->fecha_inicio, a->fecha_fin, a->coste_total);
}

/* ── Handlers de sesion ─────────────────────────────────────── */

static void handle_login(kinetix_socket_t s, const char *args) {
    if (!args || strlen(args) == 0) { resp_simple(s, RESP_ERROR); return; }

    char copia[256];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *campos[2];
    if (split_campos(copia, campos, 2) < 2) { resp_simple(s, RESP_ERROR); return; }

    Usuario *lista = NULL;
    int n = 0;
    if (listar_usuarios(&lista, &n) != 0 || n == 0) {
        free(lista);
        resp_simple(s, RESP_ERROR);
        return;
    }

    int encontrado = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(lista[i].dni, campos[0]) == 0) {
            if (verificar_clave(campos[1], lista[i].contrasena)) {
                encontrado = 1;
                char msg[128];
                snprintf(msg, sizeof(msg), "Login de usuario DNI=%s", campos[0]);
                LOG_AC(msg);
            }
            break;
        }
    }
    free(lista);
    resp_simple(s, encontrado ? RESP_OK : RESP_ERROR);
}

/* ── Handlers de estaciones ─────────────────────────────────── */

static void handle_list_estaciones(kinetix_socket_t s) {
    Estacion *lista = NULL;
    int n = 0;
    if (listar_estaciones(&lista, &n) != 0) { resp_simple(s, RESP_ERROR); return; }

    char cabecera[32];
    snprintf(cabecera, sizeof(cabecera), "%d\n", n);
    servidor_enviar(s, cabecera);

    char fila[512];
    for (int i = 0; i < n; i++) {
        serializar_estacion(&lista[i], fila, sizeof(fila));
        servidor_enviar(s, fila);
    }
    free(lista);
}

static void handle_get_estacion(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    Estacion e;
    if (buscar_estacion(atoi(args), &e) != 0) { resp_simple(s, RESP_NOT_FOUND); return; }
    char fila[512];
    serializar_estacion(&e, fila, sizeof(fila));
    servidor_enviar(s, RESP_OK "\n");
    servidor_enviar(s, fila);
}

static void handle_add_estacion(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[512];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[6];
    if (split_campos(copia, c, 6) < 6) { resp_simple(s, RESP_ERROR); return; }

    Estacion e;
    memset(&e, 0, sizeof(e));
    strncpy(e.nombre,     c[0], sizeof(e.nombre)    - 1);
    strncpy(e.direccion,  c[1], sizeof(e.direccion) - 1);
    e.coord_x               = (float)atof(c[2]);
    e.coord_y               = (float)atof(c[3]);
    e.capacidad_max         = atoi(c[4]);
    e.disponibilidad_actual = atoi(c[5]);

    int id_nuevo = 0;
    if (insertar_estacion(e, &id_nuevo) != 0) { resp_simple(s, RESP_ERROR); return; }

    char resp[64];
    snprintf(resp, sizeof(resp), RESP_OK " %d\n", id_nuevo);
    servidor_enviar(s, resp);
}

static void handle_update_estacion(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[512];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[7];
    if (split_campos(copia, c, 7) < 7) { resp_simple(s, RESP_ERROR); return; }

    Estacion e;
    memset(&e, 0, sizeof(e));
    e.id_estacion           = atoi(c[0]);
    strncpy(e.nombre,     c[1], sizeof(e.nombre)    - 1);
    strncpy(e.direccion,  c[2], sizeof(e.direccion) - 1);
    e.coord_x               = (float)atof(c[3]);
    e.coord_y               = (float)atof(c[4]);
    e.capacidad_max         = atoi(c[5]);
    e.disponibilidad_actual = atoi(c[6]);

    resp_simple(s, actualizar_estacion(e) == 0 ? RESP_OK : RESP_ERROR);
}

static void handle_delete_estacion(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    resp_simple(s, borrar_estacion(atoi(args)) == 0 ? RESP_OK : RESP_ERROR);
}

/* ── Handlers de vehiculos ──────────────────────────────────── */

static void handle_list_vehiculos(kinetix_socket_t s) {
    Vehiculo *lista = NULL;
    int n = 0;
    if (listar_vehiculos(&lista, &n) != 0) { resp_simple(s, RESP_ERROR); return; }

    char cabecera[32];
    snprintf(cabecera, sizeof(cabecera), "%d\n", n);
    servidor_enviar(s, cabecera);

    char fila[256];
    for (int i = 0; i < n; i++) {
        serializar_vehiculo(&lista[i], fila, sizeof(fila));
        servidor_enviar(s, fila);
    }
    free(lista);
}

static void handle_get_vehiculo(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    Vehiculo v;
    if (buscar_vehiculo(atoi(args), &v) != 0) { resp_simple(s, RESP_NOT_FOUND); return; }
    char fila[256];
    serializar_vehiculo(&v, fila, sizeof(fila));
    servidor_enviar(s, RESP_OK "\n");
    servidor_enviar(s, fila);
}

static void handle_add_vehiculo(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[256];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[4];
    if (split_campos(copia, c, 4) < 4) { resp_simple(s, RESP_ERROR); return; }

    Vehiculo v;
    memset(&v, 0, sizeof(v));
    v.tipo        = c[0][0];
    v.bateria     = (float)atof(c[1]);
    v.id_estacion = atoi(c[2]);
    v.estado      = c[3][0];

    int id_nuevo = 0;
    if (insertar_vehiculo(v, &id_nuevo) != 0) { resp_simple(s, RESP_ERROR); return; }

    char resp[64];
    snprintf(resp, sizeof(resp), RESP_OK " %d\n", id_nuevo);
    servidor_enviar(s, resp);
}

static void handle_update_vehiculo(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[256];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[5];
    if (split_campos(copia, c, 5) < 5) { resp_simple(s, RESP_ERROR); return; }

    Vehiculo v;
    memset(&v, 0, sizeof(v));
    v.id_vehiculo = atoi(c[0]);
    v.tipo        = c[1][0];
    v.bateria     = (float)atof(c[2]);
    v.id_estacion = atoi(c[3]);
    v.estado      = c[4][0];

    resp_simple(s, actualizar_vehiculo(v) == 0 ? RESP_OK : RESP_ERROR);
}

static void handle_baja_vehiculo(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    resp_simple(s, dar_de_baja_vehiculo(atoi(args)) == 0 ? RESP_OK : RESP_ERROR);
}

/* ── Handlers de usuarios ───────────────────────────────────── */

static void handle_list_usuarios(kinetix_socket_t s) {
    Usuario *lista = NULL;
    int n = 0;
    if (listar_usuarios(&lista, &n) != 0) { resp_simple(s, RESP_ERROR); return; }

    char cabecera[32];
    snprintf(cabecera, sizeof(cabecera), "%d\n", n);
    servidor_enviar(s, cabecera);

    char fila[256];
    for (int i = 0; i < n; i++) {
        serializar_usuario(&lista[i], fila, sizeof(fila));
        servidor_enviar(s, fila);
    }
    free(lista);
}

static void handle_get_usuario(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    Usuario u;
    if (buscar_usuario(atoi(args), &u) != 0) { resp_simple(s, RESP_NOT_FOUND); return; }
    char fila[256];
    serializar_usuario(&u, fila, sizeof(fila));
    servidor_enviar(s, RESP_OK "\n");
    servidor_enviar(s, fila);
}

static void handle_add_usuario(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[512];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[4];
    if (split_campos(copia, c, 4) < 4) { resp_simple(s, RESP_ERROR); return; }

    Usuario u;
    memset(&u, 0, sizeof(u));
    strncpy(u.dni,    c[0], sizeof(u.dni)    - 1);
    strncpy(u.nombre, c[1], sizeof(u.nombre) - 1);
    u.saldo = (float)atof(c[2]);

    if (sha256_hex(c[3], u.contrasena) != 0) { resp_simple(s, RESP_ERROR); return; }

    int id_nuevo = 0;
    if (insertar_usuario(u, &id_nuevo) != 0) { resp_simple(s, RESP_ERROR); return; }

    char resp[64];
    snprintf(resp, sizeof(resp), RESP_OK " %d\n", id_nuevo);
    servidor_enviar(s, resp);
}

static void handle_update_usuario(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[512];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[4];
    if (split_campos(copia, c, 4) < 4) { resp_simple(s, RESP_ERROR); return; }

    Usuario u;
    memset(&u, 0, sizeof(u));
    u.id_usuario = atoi(c[0]);
    strncpy(u.dni,    c[1], sizeof(u.dni)    - 1);
    strncpy(u.nombre, c[2], sizeof(u.nombre) - 1);
    u.saldo = (float)atof(c[3]);

    resp_simple(s, actualizar_usuario(u) == 0 ? RESP_OK : RESP_ERROR);
}

static void handle_baja_usuario(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    resp_simple(s, dar_de_baja_usuario(atoi(args)) == 0 ? RESP_OK : RESP_ERROR);
}

/* ── Handlers de alquileres ─────────────────────────────────── */

static void handle_list_alquileres(kinetix_socket_t s) {
    Alquiler *lista = NULL;
    int n = 0;
    if (listar_alquileres(&lista, &n) != 0) { resp_simple(s, RESP_ERROR); return; }

    char cabecera[32];
    snprintf(cabecera, sizeof(cabecera), "%d\n", n);
    servidor_enviar(s, cabecera);

    char fila[512];
    for (int i = 0; i < n; i++) {
        serializar_alquiler(&lista[i], fila, sizeof(fila));
        servidor_enviar(s, fila);
    }
    free(lista);
}

static void handle_alquilar(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[128];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[3];
    if (split_campos(copia, c, 3) < 3) { resp_simple(s, RESP_ERROR); return; }

    int id_usuario  = atoi(c[0]);
    int id_vehiculo = atoi(c[1]);
    int id_estacion_origen = atoi(c[2]);

    int ok = 0;
    int transaccion_iniciada = 0;
    Vehiculo v;
    Usuario u;
    Alquiler a;

    bd_mutex_lock();
    if (bd_begin_transaccion() != SQLITE_OK) {
        bd_mutex_unlock();
        resp_simple(s, RESP_ERROR);
        return;
    }
    transaccion_iniciada = 1;

    do {
        if (buscar_vehiculo(id_vehiculo, &v) != 0) break;
        if (v.estado != 'D') break;
        if (v.bateria <= (float)g_config.bateria_minima) break;

        if (buscar_usuario(id_usuario, &u) != 0) break;
        if (u.saldo <= 0.0f) break;

        memset(&a, 0, sizeof(a));
        a.id_usuario          = id_usuario;
        a.id_vehiculo         = id_vehiculo;
        a.id_estacion_origen  = id_estacion_origen;
        a.id_estacion_destino = 0;
        a.coste_total         = 0.0f;

        time_t ahora = time(NULL);
        struct tm *tm_info = localtime(&ahora);
        strftime(a.fecha_inicio, sizeof(a.fecha_inicio), "%Y-%m-%d %H:%M:%S", tm_info);
        strcpy(a.fecha_fin, "");

        if (insertar_alquiler(a) != 0) break;

        v.estado = 'R';
        v.id_estacion = 0;
        if (actualizar_vehiculo(v) != 0) break;

        if (bd_commit_transaccion() != SQLITE_OK) break;
        ok = 1;
    } while (0);

    if (!ok && transaccion_iniciada) bd_rollback_transaccion();
    bd_mutex_unlock();

    if (!ok) { resp_simple(s, RESP_ERROR); return; }

    char resp[64];
    snprintf(resp, sizeof(resp), RESP_OK " %d\n", a.id_alquiler);
    servidor_enviar(s, resp);
    LOG_I("Alquiler creado via cliente.");
}

static void handle_devolver(kinetix_socket_t s, const char *args) {
    if (!args) { resp_simple(s, RESP_ERROR); return; }
    char copia[128];
    strncpy(copia, args, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *c[2];
    if (split_campos(copia, c, 2) < 2) { resp_simple(s, RESP_ERROR); return; }

    int id_alquiler         = atoi(c[0]);
    int id_estacion_destino = atoi(c[1]);

    int ok = 0;
    int transaccion_iniciada = 0;
    Alquiler *lista = NULL;
    int n = 0;
    Alquiler al_copia;
    Vehiculo v;

    bd_mutex_lock();
    if (bd_begin_transaccion() != SQLITE_OK) {
        bd_mutex_unlock();
        resp_simple(s, RESP_ERROR);
        return;
    }
    transaccion_iniciada = 1;

    if (listar_alquileres(&lista, &n) != 0 || n == 0) {
        if (transaccion_iniciada) bd_rollback_transaccion();
        bd_mutex_unlock();
        free(lista);
        resp_simple(s, RESP_ERROR);
        return;
    }

    Alquiler *al = NULL;
    for (int i = 0; i < n; i++) {
        if (lista[i].id_alquiler == id_alquiler && strlen(lista[i].fecha_fin) == 0) {
            al = &lista[i];
            break;
        }
    }
    if (!al) {
        if (transaccion_iniciada) bd_rollback_transaccion();
        bd_mutex_unlock();
        free(lista);
        resp_simple(s, RESP_ERROR);
        return;
    }

    al_copia = *al;
    free(lista);

    time_t ahora = time(NULL);
    struct tm *tm_fin = localtime(&ahora);
    strftime(al_copia.fecha_fin, sizeof(al_copia.fecha_fin), "%Y-%m-%d %H:%M:%S", tm_fin);

    struct tm tm_ini = {0};
    sscanf(al_copia.fecha_inicio, "%d-%d-%d %d:%d:%d",
           &tm_ini.tm_year, &tm_ini.tm_mon,  &tm_ini.tm_mday,
           &tm_ini.tm_hour, &tm_ini.tm_min,  &tm_ini.tm_sec);
    tm_ini.tm_year -= 1900;
    tm_ini.tm_mon  -= 1;
    tm_ini.tm_isdst = -1;
    double minutos = difftime(ahora, mktime(&tm_ini)) / 60.0;

    float tarifa = g_config.tarifa_bici_min;
    if (buscar_vehiculo(al_copia.id_vehiculo, &v) == 0 && v.tipo == 'P')
        tarifa = g_config.tarifa_patinete_min;

    al_copia.coste_total         = (float)(minutos * tarifa);
    al_copia.id_estacion_destino = id_estacion_destino;

    if (actualizar_alquiler(al_copia) != 0) {
        ok = 0;
    } else if (buscar_vehiculo(al_copia.id_vehiculo, &v) == 0) {
        v.estado = 'D';
        v.id_estacion = id_estacion_destino;
        ok = (actualizar_vehiculo(v) == 0);
    } else {
        ok = 0;
    }

    if (ok && bd_commit_transaccion() == SQLITE_OK) {
        ok = 1;
    } else {
        ok = 0;
    }

    if (!ok && transaccion_iniciada) bd_rollback_transaccion();
    bd_mutex_unlock();

    if (!ok) { resp_simple(s, RESP_ERROR); return; }

    resp_simple(s, RESP_OK);
    LOG_I("Devolucion registrada via cliente.");
}

static void handle_stat(kinetix_socket_t s, const char *args) {
    if (!args) {
        resp_simple(s, RESP_ERROR);
        return;
    }

    int id_vehiculo = atoi(args);
    float bateria   = 0.0f;
    double minutos  = 0.0;

    if (stat_vehiculo(id_vehiculo, &bateria, &minutos) != 0) {
        resp_simple(s, RESP_NOT_FOUND);
        return;
    }

    // Estimar km según tipo de vehículo
    Vehiculo v;
    double velocidad_media = 15.0;
    if (buscar_vehiculo(id_vehiculo, &v) == 0 && v.tipo == 'P')
        velocidad_media = 20.0;

    double km_estimados = (minutos / 60.0) * velocidad_media;

    char resp[128];
    snprintf(resp, sizeof(resp), RESP_OK " %.1f|%.1f|%.2f\n",
             bateria, minutos, km_estimados);
    servidor_enviar(s, resp);
}

void procesar_comando(kinetix_socket_t cliente, const char *cmd) {
    char copia[PROTO_BUFF_SIZE];
    strncpy(copia, cmd, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';

    char *nombre = copia;
    char *args   = NULL;

    char *espacio = strchr(copia, ' ');
    if (espacio) {
        *espacio = '\0';
        args = espacio + 1;
        char *nl = strchr(args, '\n');
        if (nl) *nl = '\0';
    }

    if (strcmp(nombre, CMD_LOGIN) == 0) handle_login(cliente, args);
    else if (strcmp(nombre, CMD_LIST_ESTACIONES) == 0) handle_list_estaciones(cliente);
    else if (strcmp(nombre, CMD_GET_ESTACION) == 0) handle_get_estacion(cliente, args);
    else if (strcmp(nombre, CMD_ADD_ESTACION) == 0) handle_add_estacion(cliente, args);
    else if (strcmp(nombre, CMD_UPDATE_ESTACION) == 0) handle_update_estacion(cliente, args);
    else if (strcmp(nombre, CMD_DELETE_ESTACION) == 0) handle_delete_estacion(cliente, args);
    else if (strcmp(nombre, CMD_LIST_VEHICULOS) == 0) handle_list_vehiculos(cliente);
    else if (strcmp(nombre, CMD_GET_VEHICULO) == 0) handle_get_vehiculo(cliente, args);
    else if (strcmp(nombre, CMD_ADD_VEHICULO) == 0) handle_add_vehiculo(cliente, args);
    else if (strcmp(nombre, CMD_UPDATE_VEHICULO) == 0) handle_update_vehiculo(cliente, args);
    else if (strcmp(nombre, CMD_BAJA_VEHICULO) == 0) handle_baja_vehiculo(cliente, args);
    else if (strcmp(nombre, CMD_LIST_USUARIOS) == 0) handle_list_usuarios(cliente);
    else if (strcmp(nombre, CMD_GET_USUARIO) == 0) handle_get_usuario(cliente, args);
    else if (strcmp(nombre, CMD_ADD_USUARIO) == 0) handle_add_usuario(cliente, args);
    else if (strcmp(nombre, CMD_UPDATE_USUARIO) == 0) handle_update_usuario(cliente, args);
    else if (strcmp(nombre, CMD_BAJA_USUARIO) == 0) handle_baja_usuario(cliente, args);
    else if (strcmp(nombre, CMD_LIST_ALQUILERES) == 0) handle_list_alquileres(cliente);
    else if (strcmp(nombre, CMD_ALQUILAR) == 0) handle_alquilar(cliente, args);
    else if (strcmp(nombre, CMD_DEVOLVER) == 0) handle_devolver(cliente, args);
    else if (strcmp(nombre, CMD_STAT) == 0) handle_stat(cliente, args);
    else {
        char log_msg[128];
        snprintf(log_msg, sizeof(log_msg), "Comando desconocido: %s", nombre);
        LOG_A(log_msg);
        resp_simple(cliente, RESP_ERROR);
    }
}