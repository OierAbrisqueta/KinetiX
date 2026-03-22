#include <stdio.h>
#include <stdlib.h>
#include "gestor_bd.h"

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