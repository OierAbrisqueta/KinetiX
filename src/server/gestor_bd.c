#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gestor_bd.h"
#include "modelos.h"

static sqlite3 *db = NULL;

/* ============================================================
 * CONEXION
 * ============================================================ */

int conectar_bd(const char *ruta_bd) {
    const char *ruta = (ruta_bd && strlen(ruta_bd) > 0)
                       ? ruta_bd : "data/kinetix.db";

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

int insertar_estacion(Estacion e) {
    if (!db) return SQLITE_MISUSE;
    if (e.capacidad_max <= 0 || e.disponibilidad_actual < 0)
        return SQLITE_CONSTRAINT;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO ESTACION "
        "(id_estacion, nombre, direccion, capacidad_max, disponibilidad_actual) "
        "VALUES (?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT ESTACION: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_int (stmt, 1, e.id_estacion);
    sqlite3_bind_text(stmt, 2, e.nombre,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, e.direccion, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 4, e.capacidad_max);
    sqlite3_bind_int (stmt, 5, e.disponibilidad_actual);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT ESTACION: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    return SQLITE_OK;
}

int insertar_vehiculo(Vehiculo v) {
    if (!db) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO VEHICULO "
        "(id_vehiculo, tipo, bateria, estado, id_estacion) "
        "VALUES (?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT VEHICULO: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    char tipo_s[2]   = { v.tipo,   '\0' };
    char estado_s[2] = { v.estado, '\0' };

    sqlite3_bind_int   (stmt, 1, v.id_vehiculo);
    sqlite3_bind_text  (stmt, 2, tipo_s,   -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, (double)v.bateria);
    sqlite3_bind_text  (stmt, 4, estado_s, -1, SQLITE_TRANSIENT);

    if (v.id_estacion > 0)
        sqlite3_bind_int(stmt, 5, v.id_estacion);
    else
        sqlite3_bind_null(stmt, 5);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT VEHICULO: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    return SQLITE_OK;
}

int insertar_usuario(Usuario u) {
    if (!db) return SQLITE_MISUSE;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO USUARIO "
        "(id_usuario, dni, nombre, contrasena, saldo) "
        "VALUES (?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("[BD] Error preparando INSERT USUARIO: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_int   (stmt, 1, u.id_usuario);
    sqlite3_bind_text  (stmt, 2, u.dni,       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 3, u.nombre,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text  (stmt, 4, u.contrasena,-1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, (double)u.saldo);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        printf("[BD] Error INSERT USUARIO: %s\n", sqlite3_errmsg(db));
        return rc;
    }
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
    if (e.capacidad_max <= 0 || e.disponibilidad_actual < 0)
        return SQLITE_CONSTRAINT;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE ESTACION "
        "SET nombre=?, direccion=?, capacidad_max=?, disponibilidad_actual=? "
        "WHERE id_estacion=?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return rc;

    sqlite3_bind_text(stmt, 1, e.nombre,    -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, e.direccion, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 3, e.capacidad_max);
    sqlite3_bind_int (stmt, 4, e.disponibilidad_actual);
    sqlite3_bind_int (stmt, 5, e.id_estacion);

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

int listar_estaciones(Estacion **lista_out, int *cantidad_out) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    sqlite3_stmt *cnt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM ESTACION;", -1, &cnt, NULL) == SQLITE_OK) {
        if (sqlite3_step(cnt) == SQLITE_ROW) total = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    if (total == 0) return SQLITE_OK;

    Estacion *arr = (Estacion *)malloc(total * sizeof(Estacion));
    if (!arr) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_estacion, nombre, direccion, "
        "capacidad_max, disponibilidad_actual FROM ESTACION;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) { free(arr); return -1; }

    int i = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && i < total) {
        arr[i].id_estacion           = sqlite3_column_int(stmt, 0);
        strncpy(arr[i].nombre,    (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(arr[i].direccion, (const char*)sqlite3_column_text(stmt, 2), 100);
        arr[i].capacidad_max         = sqlite3_column_int(stmt, 3);
        arr[i].disponibilidad_actual = sqlite3_column_int(stmt, 4);
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
    sqlite3_stmt *cnt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM VEHICULO;", -1, &cnt, NULL) == SQLITE_OK) {
        if (sqlite3_step(cnt) == SQLITE_ROW) total = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    if (total == 0) return SQLITE_OK;

    Vehiculo *arr = (Vehiculo *)malloc(total * sizeof(Vehiculo));
    if (!arr) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_vehiculo, tipo, bateria, estado, "
        "COALESCE(id_estacion, 0) FROM VEHICULO;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) { free(arr); return -1; }

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
    sqlite3_stmt *cnt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM USUARIO;", -1, &cnt, NULL) == SQLITE_OK) {
        if (sqlite3_step(cnt) == SQLITE_ROW) total = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    if (total == 0) return SQLITE_OK;

    Usuario *arr = (Usuario *)malloc(total * sizeof(Usuario));
    if (!arr) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT id_usuario, dni, nombre, saldo FROM USUARIO;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) { free(arr); return -1; }

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
    sqlite3_stmt *cnt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM ALQUILER;", -1, &cnt, NULL) == SQLITE_OK) {
        if (sqlite3_step(cnt) == SQLITE_ROW) total = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    if (total == 0) return SQLITE_OK;

    Alquiler *arr = (Alquiler *)malloc(total * sizeof(Alquiler));
    if (!arr) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_alquiler, id_usuario, id_vehiculo, "
        "COALESCE(id_estacion_origen,0), COALESCE(id_estacion_destino,0), "
        "fecha_inicio, COALESCE(fecha_fin,''), coste_total FROM ALQUILER;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) { free(arr); return -1; }

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

int listar_vehiculos_bateria_baja(Vehiculo **lista_out, int *cantidad_out, int umbral) {
    if (!db || !lista_out || !cantidad_out) return -1;
    *lista_out = NULL; *cantidad_out = 0;

    int total = 0;
    sqlite3_stmt *cnt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM VEHICULO;", -1, &cnt, NULL) == SQLITE_OK) {
        if (sqlite3_step(cnt) == SQLITE_ROW) total = sqlite3_column_int(cnt, 0);
        sqlite3_finalize(cnt);
    }
    if (total == 0) return SQLITE_OK;

    Vehiculo *arr = (Vehiculo *)malloc(total * sizeof(Vehiculo));
    if (!arr) return -1;

    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id_vehiculo, tipo, bateria, estado, COALESCE(id_estacion,0) "
        "FROM VEHICULO WHERE bateria < ? ORDER BY bateria ASC;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) { free(arr); return -1; }
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
        "SELECT id_estacion, nombre, direccion, capacidad_max, disponibilidad_actual "
        "FROM ESTACION WHERE id_estacion = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return -1;
    sqlite3_bind_int(stmt, 1, id);
    int rc = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out->id_estacion           = sqlite3_column_int(stmt, 0);
        strncpy(out->nombre,    (const char*)sqlite3_column_text(stmt, 1), 50);
        strncpy(out->direccion, (const char*)sqlite3_column_text(stmt, 2), 100);
        out->capacidad_max         = sqlite3_column_int(stmt, 3);
        out->disponibilidad_actual = sqlite3_column_int(stmt, 4);
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