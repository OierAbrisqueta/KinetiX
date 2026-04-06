#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gestor_bd.h"
#include "modelos.h"
#include "informes.h"

static sqlite3 *db = NULL;

static int texto_vacio_o_espacios(const char *s) {
    if (!s) return 1;
    while (*s) {
        if (*s != ' ' && *s != '\t' && *s != '\n' && *s != '\r') return 0;
        s++;
    }
    return 1;
}

static int obtener_capacidad_estacion(int id_estacion, int *capacidad_out) {
    if (!db || !capacidad_out) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT capacidad_max FROM ESTACION WHERE id_estacion = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    sqlite3_bind_int(stmt, 1, id_estacion);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        *capacidad_out = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
}

static int contar_vehiculos_en_estacion(int id_estacion, int excluir_id_vehiculo, int *total_out) {
    if (!db || !total_out) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM VEHICULO WHERE id_estacion = ? AND id_vehiculo <> ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    sqlite3_bind_int(stmt, 1, id_estacion);
    sqlite3_bind_int(stmt, 2, excluir_id_vehiculo);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        *total_out = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return SQLITE_OK;
    }

    sqlite3_finalize(stmt);
    return SQLITE_ERROR;
}

static int validar_estacion_con_hueco(int id_estacion, int excluir_id_vehiculo) {
    int capacidad = 0;
    int rc = obtener_capacidad_estacion(id_estacion, &capacidad);
    if (rc != SQLITE_OK) return rc;

    int ocupados = 0;
    rc = contar_vehiculos_en_estacion(id_estacion, excluir_id_vehiculo, &ocupados);
    if (rc != SQLITE_OK) return rc;

    if (ocupados >= capacidad) return SQLITE_FULL;
    return SQLITE_OK;
}

/* ============================================================
 * CONEXION
 * ============================================================ */

int conectar_bd(const char *ruta_bd) {
    const char *ruta = (ruta_bd && strlen(ruta_bd) > 0)
                       ? ruta_bd : "data/kinetix.sqlite";

    int rc = sqlite3_open(ruta, &db);
    if (rc != SQLITE_OK) {
        printf("[BD] Error: no se puede abrir '%s': %s\n",
               ruta, sqlite3_errmsg(db));
        db = NULL;
        return rc;
    }
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    printf("[BD] Conexion establecida con '%s'.\n", ruta);
    return SQLITE_OK;
}

void cerrar_bd(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
        printf("[BD] Conexion cerrada.\n");
    }
}

/* ============================================================
 * INSERCION
 * ============================================================ */

int insertar_estacion(Estacion e, int *id_generado_out) {
    if (!db) return SQLITE_MISUSE;
    if (texto_vacio_o_espacios(e.nombre) ||
        e.capacidad_max <= 0 ||
        e.disponibilidad_actual < 0 ||
        e.disponibilidad_actual > e.capacidad_max)
        return SQLITE_CONSTRAINT;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO ESTACION "
        "(nombre, direccion, coord_x, coord_y, capacidad_max, disponibilidad_actual) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT ESTACION: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    int rc_bind = SQLITE_OK;
    if (sqlite3_bind_text(stmt, 1, e.nombre,    -1, SQLITE_TRANSIENT) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_text(stmt, 2, e.direccion, -1, SQLITE_TRANSIENT) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_double(stmt, 3, (double)e.coord_x) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_double(stmt, 4, (double)e.coord_y) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_int (stmt, 5, e.capacidad_max) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_int (stmt, 6, e.disponibilidad_actual) != SQLITE_OK) rc_bind = SQLITE_ERROR;

    if (rc_bind != SQLITE_OK) {
        printf("[BD] Error bind INSERT ESTACION: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc_bind;
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT ESTACION: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    if (id_generado_out) *id_generado_out = (int)sqlite3_last_insert_rowid(db);
    return SQLITE_OK;
}

int insertar_vehiculo(Vehiculo v, int *id_generado_out) {
    if (!db) return SQLITE_MISUSE;

    if (v.id_estacion > 0) {
        int rc_val = validar_estacion_con_hueco(v.id_estacion, 0);
        if (rc_val != SQLITE_OK) return rc_val;
    }

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO VEHICULO "
        "(tipo, bateria, estado, id_estacion) "
        "VALUES (?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT VEHICULO: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    char tipo_s[2]   = { v.tipo,   '\0' };
    char estado_s[2] = { v.estado, '\0' };

    sqlite3_bind_text  (stmt, 1, tipo_s,   -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, (double)v.bateria);
    sqlite3_bind_text  (stmt, 3, estado_s, -1, SQLITE_TRANSIENT);

    if (v.id_estacion > 0)
        sqlite3_bind_int(stmt, 4, v.id_estacion);
    else
        sqlite3_bind_null(stmt, 4);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT VEHICULO: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    if (id_generado_out) *id_generado_out = (int)sqlite3_last_insert_rowid(db);
    return SQLITE_OK;
}

int insertar_usuario(Usuario u, int *id_generado_out) {
    if (!db) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO USUARIO "
        "(dni, nombre, contrasena, saldo) "
        "VALUES (?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT USUARIO: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text  (stmt, 1, u.dni,       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, u.nombre,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, u.contrasena,-1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, (double)u.saldo);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT USUARIO: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    if (id_generado_out) *id_generado_out = (int)sqlite3_last_insert_rowid(db);
    return SQLITE_OK;
}

int insertar_alquiler(Alquiler a) {
    if (!db) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO ALQUILER "
        "(id_alquiler, id_usuario, id_vehiculo, id_estacion_origen, "
        " id_estacion_destino, fecha_inicio, fecha_fin, coste_total) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT ALQUILER: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_int(stmt, 1, a.id_alquiler);
    sqlite3_bind_int(stmt, 2, a.id_usuario);
    sqlite3_bind_int(stmt, 3, a.id_vehiculo);
    sqlite3_bind_int(stmt, 4, a.id_estacion_origen);

    if (a.id_estacion_destino > 0)
        sqlite3_bind_int(stmt, 5, a.id_estacion_destino);
    else
        sqlite3_bind_null(stmt, 5);

    sqlite3_bind_text(stmt, 6, a.fecha_inicio, -1, SQLITE_TRANSIENT);

    if (strlen(a.fecha_fin) > 0)
        sqlite3_bind_text(stmt, 7, a.fecha_fin, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 7);

    sqlite3_bind_double(stmt, 8, (double)a.coste_total);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT ALQUILER: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    return SQLITE_OK;
}

/* ============================================================
 * BAJA
 * ============================================================ */

static int borrar_por_id(const char *tabla, const char *col_id, int id) {
    if (!db) return SQLITE_MISUSE;
    char sql[128];
    snprintf(sql, sizeof(sql), "DELETE FROM %s WHERE %s = ?;", tabla, col_id);
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return rc;
    if (sqlite3_changes(db) == 0) return SQLITE_NOTFOUND;
    return SQLITE_OK;
}

int borrar_estacion(int id)      { return borrar_por_id("ESTACION", "id_estacion", id); }
int dar_de_baja_vehiculo(int id) { return borrar_por_id("VEHICULO", "id_vehiculo",  id); }
int dar_de_baja_usuario(int id)  { return borrar_por_id("USUARIO",  "id_usuario",   id); }

/* ============================================================
 * ACTUALIZACION
 * ============================================================ */

int actualizar_estacion(Estacion e) {
    if (!db) return SQLITE_MISUSE;
    if (texto_vacio_o_espacios(e.nombre) ||
        e.capacidad_max <= 0 ||
        e.disponibilidad_actual < 0 ||
        e.disponibilidad_actual > e.capacidad_max)
        return SQLITE_CONSTRAINT;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE ESTACION "
        "SET nombre=?, direccion=?, coord_x=?, coord_y=?, capacidad_max=?, disponibilidad_actual=? "
        "WHERE id_estacion=?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    int rc_bind = SQLITE_OK;
    if (sqlite3_bind_text(stmt, 1, e.nombre,    -1, SQLITE_TRANSIENT) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_text(stmt, 2, e.direccion, -1, SQLITE_TRANSIENT) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_double(stmt, 3, (double)e.coord_x) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_double(stmt, 4, (double)e.coord_y) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_int (stmt, 5, e.capacidad_max) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_int (stmt, 6, e.disponibilidad_actual) != SQLITE_OK) rc_bind = SQLITE_ERROR;
    if (sqlite3_bind_int (stmt, 7, e.id_estacion) != SQLITE_OK) rc_bind = SQLITE_ERROR;

    if (rc_bind != SQLITE_OK) {
        printf("[BD] Error bind UPDATE ESTACION: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return rc_bind;
    }

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return rc;
    if (sqlite3_changes(db) == 0) return SQLITE_NOTFOUND;
    return SQLITE_OK;
}

int actualizar_usuario(Usuario u) {
    if (!db) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE USUARIO "
        "SET dni=?, nombre=?, contrasena=?, saldo=? "
        "WHERE id_usuario=?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    sqlite3_bind_text  (stmt, 1, u.dni,       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 2, u.nombre,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, u.contrasena,-1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 4, (double)u.saldo);
    sqlite3_bind_int   (stmt, 5, u.id_usuario);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return rc;
    if (sqlite3_changes(db) == 0) return SQLITE_NOTFOUND;
    return SQLITE_OK;
}

int actualizar_vehiculo(Vehiculo v) {
    if (!db) return SQLITE_MISUSE;

    Vehiculo existente;
    if (buscar_vehiculo(v.id_vehiculo, &existente) != 0) return SQLITE_NOTFOUND;

    if (v.id_estacion > 0) {
        int excluir = (existente.id_estacion == v.id_estacion) ? v.id_vehiculo : 0;
        int rc_val = validar_estacion_con_hueco(v.id_estacion, excluir);
        if (rc_val != SQLITE_OK) return rc_val;
    }

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE VEHICULO "
        "SET tipo=?, bateria=?, estado=?, id_estacion=? "
        "WHERE id_vehiculo=?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    char tipo_s[2]   = { v.tipo,   '\0' };
    char estado_s[2] = { v.estado, '\0' };

    sqlite3_bind_text  (stmt, 1, tipo_s,   -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, (double)v.bateria);
    sqlite3_bind_text  (stmt, 3, estado_s, -1, SQLITE_TRANSIENT);

    if (v.id_estacion > 0)
        sqlite3_bind_int(stmt, 4, v.id_estacion);
    else
        sqlite3_bind_null(stmt, 4);

    sqlite3_bind_int(stmt, 5, v.id_vehiculo);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return rc;
    if (sqlite3_changes(db) == 0) return SQLITE_NOTFOUND;
    return SQLITE_OK;
}

/* ============================================================
 * LECTURA / LISTADO
 * ============================================================ */

static int obtener_total(const char *sql_count, int *total_out) {
    sqlite3_stmt *cnt = NULL;
    int rc = sqlite3_prepare_v2(db, sql_count, -1, &cnt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando COUNT: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_step(cnt);
    if (rc == SQLITE_ROW) {
        *total_out = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
        return SQLITE_OK;
    }

    sqlite3_finalize(cnt);
    printf("[BD] Error ejecutando COUNT: %s\n", sqlite3_errmsg(db));
    return (rc == SQLITE_DONE) ? SQLITE_ERROR : rc;
}

int listar_estaciones(Estacion **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    int rc = obtener_total("SELECT COUNT(*) FROM ESTACION;", &total);
    if (rc != SQLITE_OK) return rc;
    if (total == 0) return SQLITE_OK;

    Estacion *arr = (Estacion *)malloc(total * sizeof(Estacion));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_estacion, nombre, direccion, coord_x, coord_y, "
        "capacidad_max, disponibilidad_actual FROM ESTACION;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando LISTADO ESTACION: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_estacion           = sqlite3_column_int(stmt, 0);
        strncpy(arr[i].nombre,    (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(arr[i].direccion, (const char*)sqlite3_column_text(stmt, 2), 100);
        arr[i].coord_x               = (float)sqlite3_column_double(stmt, 3);
        arr[i].coord_y               = (float)sqlite3_column_double(stmt, 4);
        arr[i].capacidad_max         = sqlite3_column_int(stmt, 5);
        arr[i].disponibilidad_actual = sqlite3_column_int(stmt, 6);
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = arr; *cantidad_out = i;
    return SQLITE_OK;
}

int listar_vehiculos(Vehiculo **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    int rc = obtener_total("SELECT COUNT(*) FROM VEHICULO;", &total);
    if (rc != SQLITE_OK) return rc;
    if (total == 0) return SQLITE_OK;

    Vehiculo *arr = (Vehiculo *)malloc(total * sizeof(Vehiculo));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_vehiculo, tipo, bateria, estado, "
        "COALESCE(id_estacion, 0) FROM VEHICULO;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando LISTADO VEHICULO: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_vehiculo = sqlite3_column_int(stmt, 0);
        arr[i].tipo        = sqlite3_column_text(stmt, 1)[0];
        arr[i].bateria     = (float)sqlite3_column_double(stmt, 2);
        arr[i].estado      = sqlite3_column_text(stmt, 3)[0];
        arr[i].id_estacion = sqlite3_column_int(stmt, 4);
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = arr; *cantidad_out = i;
    return SQLITE_OK;
}

int listar_usuarios(Usuario **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    int rc = obtener_total("SELECT COUNT(*) FROM USUARIO;", &total);
    if (rc != SQLITE_OK) return rc;
    if (total == 0) return SQLITE_OK;

    Usuario *arr = (Usuario *)malloc(total * sizeof(Usuario));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT id_usuario, dni, nombre, saldo FROM USUARIO;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando LISTADO USUARIO: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_usuario = sqlite3_column_int(stmt, 0);
        strncpy(arr[i].dni,    (const char*)sqlite3_column_text(stmt, 1), 9);
        strncpy(arr[i].nombre, (const char*)sqlite3_column_text(stmt, 2), 50);
        arr[i].saldo = (float)sqlite3_column_double(stmt, 3);
        arr[i].contrasena[0] = '\0';
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = arr; *cantidad_out = i;
    return SQLITE_OK;
}

int listar_alquileres(Alquiler **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    int rc = obtener_total("SELECT COUNT(*) FROM ALQUILER;", &total);
    if (rc != SQLITE_OK) return rc;
    if (total == 0) return SQLITE_OK;

    Alquiler *arr = (Alquiler *)malloc(total * sizeof(Alquiler));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_alquiler, id_usuario, id_vehiculo, "
        "COALESCE(id_estacion_origen,0), COALESCE(id_estacion_destino,0), "
        "fecha_inicio, COALESCE(fecha_fin,''), coste_total FROM ALQUILER;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando LISTADO ALQUILER: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_alquiler         = sqlite3_column_int(stmt, 0);
        arr[i].id_usuario          = sqlite3_column_int(stmt, 1);
        arr[i].id_vehiculo         = sqlite3_column_int(stmt, 2);
        arr[i].id_estacion_origen  = sqlite3_column_int(stmt, 3);
        arr[i].id_estacion_destino = sqlite3_column_int(stmt, 4);
        strncpy(arr[i].fecha_inicio, (const char*)sqlite3_column_text(stmt, 5), 19);
        strncpy(arr[i].fecha_fin,    (const char*)sqlite3_column_text(stmt, 6), 19);
        arr[i].coste_total = (float)sqlite3_column_double(stmt, 7);
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = arr; *cantidad_out = i;
    return SQLITE_OK;
}

int listar_ultimos_alquileres(int limite, Alquiler **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL;
    *cantidad_out = 0;
    if (limite <= 0) return SQLITE_CONSTRAINT;

    Alquiler *arr = (Alquiler *)malloc((size_t)limite * sizeof(Alquiler));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_alquiler, id_usuario, id_vehiculo, "
        "id_estacion_origen, COALESCE(id_estacion_destino,0), "
        "fecha_inicio, COALESCE(fecha_fin,''), coste_total "
        "FROM ALQUILER ORDER BY fecha_inicio DESC LIMIT ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando el listado de los ultimos alquileres: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }
    sqlite3_bind_int(stmt, 1, limite);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < limite) {
        arr[i].id_alquiler         = sqlite3_column_int(stmt, 0);
        arr[i].id_usuario          = sqlite3_column_int(stmt, 1);
        arr[i].id_vehiculo         = sqlite3_column_int(stmt, 2);
        arr[i].id_estacion_origen  = sqlite3_column_int(stmt, 3);
        arr[i].id_estacion_destino = sqlite3_column_int(stmt, 4);
        strncpy(arr[i].fecha_inicio, (const char*)sqlite3_column_text(stmt, 5), 19);
        strncpy(arr[i].fecha_fin,    (const char*)sqlite3_column_text(stmt, 6), 19);
        arr[i].coste_total = (float)sqlite3_column_double(stmt, 7);
        i++;
    }
    sqlite3_finalize(stmt);

    if (i == 0) {
        free(arr);
        return SQLITE_OK;
    }

    *lista_out = arr;
    *cantidad_out = i;
    return SQLITE_OK;
}

int listar_vehiculos_bateria_baja(Vehiculo **lista_out, int *cantidad_out, int umbral) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    int rc = obtener_total("SELECT COUNT(*) FROM VEHICULO;", &total);
    if (rc != SQLITE_OK) return rc;
    if (total == 0) return SQLITE_OK;

    Vehiculo *arr = (Vehiculo *)malloc(total * sizeof(Vehiculo));
    if (!arr) return SQLITE_NOMEM;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_vehiculo, tipo, bateria, estado, COALESCE(id_estacion,0) "
        "FROM VEHICULO WHERE bateria < ? ORDER BY bateria ASC;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando LISTADO VEHICULOS BATERIA BAJA: %s\n", sqlite3_errmsg(db));
        free(arr);
        return rc;
    }
    sqlite3_bind_int(stmt, 1, umbral);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_vehiculo = sqlite3_column_int(stmt, 0);
        arr[i].tipo        = sqlite3_column_text(stmt, 1)[0];
        arr[i].bateria     = (float)sqlite3_column_double(stmt, 2);
        arr[i].estado      = sqlite3_column_text(stmt, 3)[0];
        arr[i].id_estacion = sqlite3_column_int(stmt, 4);
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = arr; *cantidad_out = i;
    return SQLITE_OK;
}

int buscar_estacion(int id, Estacion *out) {
    if (!db || !out) return -1;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_estacion, nombre, direccion, coord_x, coord_y, capacidad_max, disponibilidad_actual "
        "FROM ESTACION WHERE id_estacion = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id_estacion           = sqlite3_column_int(stmt, 0);
        strncpy(out->nombre,    (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(out->direccion, (const char*)sqlite3_column_text(stmt, 2), 100);
        out->coord_x               = (float)sqlite3_column_double(stmt, 3);
        out->coord_y               = (float)sqlite3_column_double(stmt, 4);
        out->capacidad_max         = sqlite3_column_int(stmt, 5);
        out->disponibilidad_actual = sqlite3_column_int(stmt, 6);
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}

int buscar_vehiculo(int id, Vehiculo *out) {
    if (!db || !out) return -1;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_vehiculo, tipo, bateria, estado, COALESCE(id_estacion,0) "
        "FROM VEHICULO WHERE id_vehiculo = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id_vehiculo = sqlite3_column_int(stmt, 0);
        out->tipo        = sqlite3_column_text(stmt, 1)[0];
        out->bateria     = (float)sqlite3_column_double(stmt, 2);
        out->estado      = sqlite3_column_text(stmt, 3)[0];
        out->id_estacion = sqlite3_column_int(stmt, 4);
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}

int buscar_usuario(int id, Usuario *out) {
    if (!db || !out) return -1;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_usuario, dni, nombre, saldo FROM USUARIO WHERE id_usuario = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id_usuario = sqlite3_column_int(stmt, 0);
        strncpy(out->dni,    (const char*)sqlite3_column_text(stmt, 1), 9);
        strncpy(out->nombre, (const char*)sqlite3_column_text(stmt, 2), 50);
        out->saldo = (float)sqlite3_column_double(stmt, 3);
        out->contrasena[0] = '\0';
        rc = 0;
    }
    sqlite3_finalize(stmt);
    return rc;
}

int buscar_alquiler_por_usuario(int id_usuario, Alquiler **lista_out, int *n_out) {
    if (!db || !lista_out || !n_out) return -1;
    *lista_out = NULL;
    *n_out = 0;

    int total = 0;
    sqlite3_stmt *count = NULL;
    const char *count_sql = "SELECT COUNT(*) FROM ALQUILER WHERE id_usuario = ?;";
    int rc = sqlite3_prepare_v2(db, count_sql, -1, &count, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(count, 1, id_usuario);
        if (sqlite3_step(count) == SQLITE_ROW) total = sqlite3_column_int(count, 0);
        sqlite3_finalize(count);
    } else {
        return -1;
    }
    if (total == 0) return SQLITE_OK;

    Alquiler *alquileres = (Alquiler *)malloc(total * sizeof(Alquiler));
    if (!alquileres) {
        printf("Error: No se ha podido reservar en la memoria\n");
        return -1;
    };

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_alquiler, id_usuario, id_vehiculo, "
        "id_estacion_origen, COALESCE(id_estacion_destino,0), "
        "fecha_inicio, COALESCE(fecha_fin,''), coste_total "
        "FROM ALQUILER WHERE id_usuario = ? ORDER BY fecha_inicio DESC;";

    int rc_2 = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc_2 != SQLITE_OK) {
        free(alquileres);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, id_usuario);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        alquileres[i].id_alquiler = sqlite3_column_int(stmt, 0);
        alquileres[i].id_usuario = sqlite3_column_int(stmt, 1);
        alquileres[i].id_vehiculo = sqlite3_column_int(stmt, 2);
        alquileres[i].id_estacion_origen = sqlite3_column_int(stmt, 3);
        alquileres[i].id_estacion_destino = sqlite3_column_int(stmt, 4);
        strncpy(alquileres[i].fecha_inicio, (const char*)sqlite3_column_text(stmt, 5), 19);
        strncpy(alquileres[i].fecha_fin, (const char*)sqlite3_column_text(stmt, 6), 19);
        alquileres[i].coste_total = (float)sqlite3_column_double(stmt, 7);
        i++;
    }

    sqlite3_finalize(stmt);
    *lista_out = alquileres;
    *n_out = i;
    return SQLITE_OK;
}

int buscar_alquiler_por_fecha(const char *fecha, Alquiler **lista_out, int *n_out) {
    if (!db || !lista_out || !n_out) return -1;
    *lista_out = NULL;
    *n_out = 0;

    int total = 0;
    sqlite3_stmt *count = NULL;
    const char *count_sql = "SELECT COUNT(*) FROM ALQUILER WHERE DATE(fecha_inicio) = ?;";
    int rc = sqlite3_prepare_v2(db, count_sql, -1, &count, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(count, 1, fecha, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(count) == SQLITE_ROW) total = sqlite3_column_int(count, 0);
        sqlite3_finalize(count);
    } else {
        return -1;
    }
    if (total == 0) return SQLITE_OK;

    Alquiler *alquileres = (Alquiler *)malloc(total * sizeof(Alquiler));
    if (!alquileres) {
        printf("Error: No se ha podido reservar en la memoria\n");
        return -1;
    };

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_alquiler, id_usuario, id_vehiculo, "
        "id_estacion_origen, COALESCE(id_estacion_destino,0), "
        "fecha_inicio, COALESCE(fecha_fin,''), coste_total "
        "FROM ALQUILER WHERE DATE(fecha_inicio) = ? ORDER BY fecha_inicio DESC;";

    int rc_2 = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc_2 != SQLITE_OK) {
        free(alquileres);
        return -1;
    }
    sqlite3_bind_text(stmt, 1, fecha, -1, SQLITE_TRANSIENT);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        alquileres[i].id_alquiler = sqlite3_column_int(stmt, 0);
        alquileres[i].id_usuario = sqlite3_column_int(stmt, 1);
        alquileres[i].id_vehiculo = sqlite3_column_int(stmt, 2);
        alquileres[i].id_estacion_origen = sqlite3_column_int(stmt, 3);
        alquileres[i].id_estacion_destino = sqlite3_column_int(stmt, 4);
        strncpy(alquileres[i].fecha_inicio, (const char*)sqlite3_column_text(stmt, 5), 19);
        strncpy(alquileres[i].fecha_fin, (const char*)sqlite3_column_text(stmt, 6), 19);
        alquileres[i].coste_total = (float)sqlite3_column_double(stmt, 7);
        i++;
    }

    sqlite3_finalize(stmt);
    *lista_out = alquileres;
    *n_out = i;
    return SQLITE_OK;
}

//INFORMES
int informe_recaudacion_por_tipo(ResumenPorTipo out[2]) {
    if (!db) return -1;

    out[0].tipo = 'B';
    out[0].num_alquileres = 0;
    out[0].recaudacion = 0;

    out[1].tipo = 'P';
    out[1].num_alquileres = 0;
    out[1].recaudacion = 0;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT V.tipo, COUNT(*), SUM(A.coste_total) "
        "FROM ALQUILER A "
        "JOIN VEHICULO V ON A.id_vehiculo = V.id_vehiculo "
        "WHERE A.fecha_fin IS NOT NULL "
        "GROUP BY V.tipo;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char tipo = sqlite3_column_text(stmt, 0)[0];
        int  idx = (tipo == 'B') ? 0 : 1;
        out[idx].num_alquileres = sqlite3_column_int(stmt, 1);
        out[idx].recaudacion = (float)sqlite3_column_double(stmt, 2);
    }
    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

int informe_recaudacion_por_dia(ResumenPorDia **lista_out, int *n_out) {
    if (!db || !lista_out || !n_out) return -1;
    *lista_out = NULL;
    *n_out = 0;

    int total = 0;
    sqlite3_stmt *count = NULL;
    const char *count_sql = "SELECT COUNT(DISTINCT DATE(fecha_inicio)) FROM ALQUILER "
                            "WHERE fecha_fin IS NOT NULL;";
    int rc = sqlite3_prepare_v2(db, count_sql, -1, &count, NULL);
    if (rc == SQLITE_OK) {
        if (sqlite3_step(count) == SQLITE_ROW) total = sqlite3_column_int(count, 0);
        sqlite3_finalize(count);
    } else {
        return -1;
    }
    if (total == 0) return SQLITE_OK;

    ResumenPorDia *array = (ResumenPorDia *)malloc(total * sizeof(ResumenPorDia));
    if (!array) {
        printf("Error: No se ha podido reservar en la memoria\n");
        return -1;
    };

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT DATE(fecha_inicio), COUNT(*), SUM(coste_total) "
        "FROM ALQUILER "
        "WHERE fecha_fin IS NOT NULL "
        "GROUP BY DATE(fecha_inicio) "
        "ORDER BY DATE(fecha_inicio) DESC;";

    int rc_2 = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc_2 != SQLITE_OK) {
        free(array);
        return -1;
    }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        const char* fecha = (const char*)sqlite3_column_text(stmt, 0);

        if (fecha != NULL) {
            strncpy(array[i].fecha, fecha, 10);
            array[i].fecha[10] = '\0';
        } else {
            array[i].fecha[0] = '\0';
        }

        array[i].num_alquileres = sqlite3_column_int(stmt, 1);
        array[i].recaudacion = (float)sqlite3_column_double(stmt, 2);

        i++;
    }

    sqlite3_finalize(stmt);
    *lista_out = array;
    *n_out = i;
    return SQLITE_OK;
}

int ranking_uso_vehiculo(UsoVehiculo **lista_out, int *n_out, int top_n) {
    if (!db || !lista_out || !n_out) return -1;
    *lista_out = NULL;
    *n_out = 0;

    int mostrar = (top_n > 0) ? top_n : 10;

    UsoVehiculo *array = (UsoVehiculo *)malloc(mostrar * sizeof(UsoVehiculo));
    if (!array) {
        printf("Error: No se ha podido reservar en la memoria\n");
        return -1;
    };

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT A.id_vehiculo, V.tipo, COUNT(*), "
        "SUM((strftime('%s', A.fecha_fin) - strftime('%s', A.fecha_inicio)) / 60.0) "
        "FROM ALQUILER A "
        "JOIN VEHICULO V ON A.id_vehiculo = V.id_vehiculo "
        "WHERE A.fecha_fin IS NOT NULL "
        "GROUP BY A.id_vehiculo "
        "ORDER BY COUNT(*) DESC "
        "LIMIT ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(array);
        return -1;
    };
    sqlite3_bind_int(stmt, 1, mostrar);

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < mostrar) {
        array[i].id_vehiculo = sqlite3_column_int(stmt, 0);
        array[i].tipo = sqlite3_column_text(stmt, 1)[0];
        array[i].num_alquileres = sqlite3_column_int(stmt, 2);
        array[i].minutos_totales = sqlite3_column_double(stmt, 3);
        i++;
    }
    sqlite3_finalize(stmt);
    *lista_out = array;
    *n_out = i;
    return SQLITE_OK;
}