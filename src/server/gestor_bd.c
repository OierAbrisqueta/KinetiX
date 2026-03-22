#include <stdio.h>
#include <stdlib.h>
#include "gestor_bd.h"
#include "modelos.h"

sqlite3 *db;

int conectar_bd(const char* ruta_bd) {
    int result = sqlite3_open("test.sqlite", &db);

    if (result != SQLITE_OK) {
        printf("Error: No se puede abrir la base de datos\n");
        return result;
    } else {
        printf("Se ha establecido la conexión con la base de datos\n");
        return result;
    }
};

void cerrar_bd() {
    if (db != NULL) {
        sqlite3_close(db);
        printf("Conexión con la base de datos cerrada\n");
    }
}

int insertar_estacion(Estacion nueva_estacion) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "INSERT INTO ESTACION "
        "(id_estacion, nombre, direccion, capacidad_max, disponibilidad_actual) "
        "VALUES (?, ?, ?, ?, ?);";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, nueva_estacion.id_estacion);
    sqlite3_bind_text(stmt, 2, nueva_estacion.nombre, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, nueva_estacion.direccion, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, nueva_estacion.capacidad_max);
    sqlite3_bind_int(stmt, 5, nueva_estacion.disponibilidad_actual);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error insertando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    printf("Se ha insertado correctamente\n");

    return SQLITE_OK;
}

int insertar_vehiculo(Vehiculo nuevo_vehiculo) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "INSERT INTO VEHICULO "
        "(id_vehiculo, tipo, bateria, estado, id_estacion) "
        "VALUES (?, ?, ?, ?, ?);";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    char tipo_str[2] = { nuevo_vehiculo.tipo, '\0' };
    char estado_str[2] = { nuevo_vehiculo.estado, '\0' };

    sqlite3_bind_int(stmt, 1, nuevo_vehiculo.id_vehiculo);
    sqlite3_bind_text(stmt, 2, tipo_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, (double)nuevo_vehiculo.bateria);
    sqlite3_bind_text(stmt, 4, estado_str, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, nuevo_vehiculo.id_estacion);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error insertando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    printf("Se ha insertado correctamente\n");

    return SQLITE_OK;
}

int insertar_usuario(Usuario nuevo_usuario) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "INSERT INTO USUARIO "
        "(id_usuario, dni, nombre, contrasena, saldo) "
        "VALUES (?, ?, ?, ?, ?);";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, nuevo_usuario.id_usuario);
    sqlite3_bind_text(stmt, 2, nuevo_usuario.dni, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, nuevo_usuario.nombre, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, nuevo_usuario.contrasena, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, nuevo_usuario.saldo);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error insertando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    printf("Se ha insertado correctamente\n");

    return SQLITE_OK;
}

int insertar_alquiler(Alquiler nuevo_alquiler) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;

    const char *sql =
        "INSERT INTO ALQUILER "
        "(id_alquiler, id_usuario, id_vehiculo, id_estacion_origen, id_estacion_destino, fecha_inicio, fecha_fin, coste_total) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, nuevo_alquiler.id_alquiler);
    sqlite3_bind_int(stmt, 2, nuevo_alquiler.id_usuario);
    sqlite3_bind_int(stmt, 3, nuevo_alquiler.id_vehiculo);
    sqlite3_bind_int(stmt, 4, nuevo_alquiler.id_estacion_origen);

    if (nuevo_alquiler.id_estacion_destino > 0) {
        sqlite3_bind_int(stmt, 5, nuevo_alquiler.id_estacion_destino);
    } else {
        sqlite3_bind_null(stmt, 5);
    }

    sqlite3_bind_text(stmt, 6, nuevo_alquiler.fecha_inicio, -1, SQLITE_TRANSIENT);

    if (nuevo_alquiler.fecha_fin != '\0') {
        sqlite3_bind_text(stmt, 7, nuevo_alquiler.fecha_fin, -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt, 7);
    }

    sqlite3_bind_double(stmt, 8, (double)nuevo_alquiler.coste_total);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error insertando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    printf("Se ha insertado correctamente\n");

    return SQLITE_OK;
}

int borrar_estacion(int id_estacion) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM ESTACION WHERE id_estacion = ?;";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, id_estacion);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error eliminando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    int filas = sqlite3_changes(db);

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    if (filas == 0) {
        printf("No existe ninguna estación con ese id");
        return SQLITE_NOTFOUND;
    }

    printf("Fila borrada correctamente\n");

    return SQLITE_OK;
}

int dar_de_baja_vehiculo(int id_vehiculo) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM VEHICULO WHERE id_vehiculo = ?;";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, id_vehiculo);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error eliminando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    int filas = sqlite3_changes(db);

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    if (filas == 0) {
        printf("No existe ningún vehículo con ese id");
        return SQLITE_NOTFOUND;
    }

    printf("Fila borrada correctamente\n");

    return SQLITE_OK;
}

int dar_de_baja_usuario(int id_usuario) {
    if (db == NULL) {
        return SQLITE_MISUSE;
    }

    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM USUARIO WHERE id_usuario = ?;";

    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (result != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_int(stmt, 1, id_usuario);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        printf("Error eliminando datos: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return result;
    }

    int filas = sqlite3_changes(db);

    result = sqlite3_finalize(stmt);
    if (result != SQLITE_OK) {
        printf("Error finalizando statement: %s\n", sqlite3_errmsg(db));
        return result;
    }

    if (filas == 0) {
        printf("No existe ningún usuario con ese id");
        return SQLITE_NOTFOUND;
    }

    printf("Fila borrada correctamente\n");

    return SQLITE_OK;
}