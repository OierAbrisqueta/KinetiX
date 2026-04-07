#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "menu.h"

#include <time.h>

#include "gestor_config.h"
#include "gestor_log.h"
#include "gestor_bd.h"
#include "modelos.h"
#include "informes.h"

/* ============================================================
 * menu.c - Logica de navegacion y menus del administrador
 * ============================================================ */

/* ============================================================
 * UTILIDADES DE UI
 * ============================================================ */

void ui_limpiar(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void ui_pausa(void) {
    printf("\n  ................................................\n");
    printf("\n  Pulse Enter para continuar...");
    /* Limpiar buffer antes de esperar */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ui_separador(void) {
    printf("  +--------------------------------------------------+\n");
}

static void limpiar_buffer_teclado(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

static int esta_vacio(const char *s) {
    if (!s) return 1;

    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') return 0;
    }

    return 1;
}

static void mayusculas(char *s) {
    if (!s) return;
    for (int i = 0; s[i] != '\0'; i++) {
        s[i] = (char)toupper((unsigned char)s[i]);
    }
}

static void ui_leer_string_no_vacio(const char *mensaje, char *buf, int max) {
    do {
        ui_leer_string(mensaje, buf, max);
        if (esta_vacio(buf)) {
            printf("  Error: No puedes dejarlo vacio.\n");
        }
    } while (esta_vacio(buf));
}

static void leer_dni(char *buf, int max) {
    while (1) {
        ui_leer_string("DNI (9 chars)", buf, max);

        if (esta_vacio(buf)) {
            printf("  Error: El DNI no puede estar vacio.\n");
            continue;
        }

        int len = (int)strlen(buf);
        if (len != 9) {
            printf("  Error: El DNI debe tener exactamente 9 caracteres.\n");
            continue;
        }

        int valido = 1;
        for (int i = 0; i < 8; i++) {
            if (!isdigit((unsigned char)buf[i])) {
                valido = 0;
                break;
            }
        }

        if (!isalpha((unsigned char)buf[8])) valido = 0;

        if (!valido) {
            printf("  Error: El formato del dni es incorrecto.\n");
            continue;
        }

        mayusculas(buf);
        return;
    }
}

static void leer_nie(char *buf, int max) {
    while (1) {
        ui_leer_string("NIE ", buf, max);

        if (esta_vacio(buf)) {
            printf("  Error: El NIE no puede estar vacio.\n");
            continue;
        }

        int len = (int)strlen(buf);
        if (len != 9) {
            printf("  Error: El NIE debe tener exactamente 9 caracteres.\n");
            continue;
        }

        mayusculas(buf);

        //El primer caracter debe de ser x, y o z
        if (buf[0] != 'X' && buf[0] != 'Y' && buf[0] != 'Z') {
            printf("  Error: El primer caracter debe de ser x, y o z.\n");
            continue;
        }

        int valido = 1;
        for (int i = 1; i < 8; i++) {
            if (!isdigit((unsigned char)buf[i])) {
                valido = 0;
                break;
            }
        }

        if (!isalpha((unsigned char)buf[8])) valido = 0;

        if (!valido) {
            printf("  Error: El formato del NIE es incorrecto.\n");
            continue;
        }

        return;
    }
}

static void dni_o_nie(char *buf, int max) {
    char documento;

    //Se pregunta que tipo de documento tiene el usuario
    if (ui_leer_opcion_char("Tipo de documento (D: DNI o N: NIE): ", "DN", &documento)) {
        //En función del tipo de documento se redirige a la funcion correspondiente
        if (documento == 'D') {
            leer_dni(buf, max);
        } else if (documento == 'N') {
            leer_nie(buf, max);
        }
    }
}

int ui_leer_opcion_char(const char *prompt, const char *validos, char *out) {
    char buf[64];

    while (1) {
        printf("  %s", prompt);

        if (!fgets(buf, sizeof(buf), stdin)) {
            return 0;
        }

        if (strchr(buf, '\n') == NULL) limpiar_buffer_teclado();

        if (esta_vacio(buf)) {
            printf("  Error: No puedes dejarlo vacio.\n");
            continue;
        }

        char c = (char)toupper((unsigned char)buf[0]);
        if (!strchr(validos, c)) {
            printf("  Error: Opcion invalida. Valores permitidos: %s\n", validos);
            continue;
        }

        *out = c;
        return 1;
    }
}

static int ui_leer_float_en_rango(const char *mensaje, float min, float max, float *valor_out) {
    char buf[64];
    float valor;
    char extra;

    while (1) {
        printf("  %s [%.2f - %.2f]: ", mensaje, min, max);

        //En caso de error de lectura se devuelve 0
        if (!fgets(buf, sizeof(buf), stdin)) {
            return 0;
        }

        /*Si la cadena es más larga que el buffer, limpiamos el resto
        Para comprobarlo se busca si el salto de linea está dentro del texto*/
        if (strchr(buf, '\n') == NULL) limpiar_buffer_teclado();

        //Se verifica si el administrador ha escrito contenido
        if (esta_vacio(buf)) {
            printf("  Error: No puedes dejarlo vacio.\n");

            //Se vuelve a preguntar
            continue;
        }

        /*Se utiliza sscanf para comprobar los elementos escritos (busca un numérico y luego un char),
         *Debería unicamente que haber uno numérico, en el hipotetico caso de que lea un char antes del numerico
         * devolvera 0*/
        if (sscanf(buf, " %f %c", &valor, &extra) != 1) {
            printf("  Error: Debes introducir un numero valido sin letras.\n");
            continue;
        }

        //Se verifica el rango
        if (valor < min || valor > max) {
            printf("  Error: El numero debe estar entre %.2f y %.2f.\n", min, max);
            continue;
        }

        *valor_out = valor;
        return 1;
    }
}

int ui_leer_int(const char *mensaje, int min, int max) {
    char buf[64];
    int  valor;
    char extra;

    while (1) {
        printf("  %s [%d-%d]: ", mensaje, min, max);

        //En caso de error se devuelve 0
        if (!fgets(buf, sizeof(buf), stdin)) {
            return 0;
        }

        /*Si la cadena es más larga que el buffer, limpiamos el resto
        Para comprobarlo se busca si el salto de linea está dentro del texto*/
        if (strchr(buf, '\n') == NULL) limpiar_buffer_teclado();

        //Se verifica si el administrador ha escrito contenido
        if (esta_vacio(buf)) {
            printf("  Error: No puedes dejarlo vacio.\n");

            //Se vuelve a preguntar
            continue;
        }

        /*Se utiliza sscanf para comprobar los elementos escritos (busca un entero y luego un char),
         *Debería unicamente que haber uno entero, en el hipotetico caso de que lea un char antes del numerico
         * devolvera 0. En este caso tambien comprueba que sea un entero*/
        if (sscanf(buf, " %d %c", &valor, &extra) != 1) {
            printf("  Error: Debes introducir un entero valido sin letras.\n");
            continue;
        }

        //Se verifica el rango
        if (valor < min || valor > max) {
            printf("  Error: El numero debe estar entre %d y %d.\n", min, max);
            continue;
        }

        return valor;
    }
}

void ui_leer_string(const char *prompt, char *buf, int max_len) {
    printf("  %s:  ", prompt);
    if (fgets(buf, max_len, stdin)) {
        int len = (int)strlen(buf);
        if (len > 0 && buf[len-1] == '\n') {
            buf[len-1] = '\0';
        } else {
            limpiar_buffer_teclado();
        }
    } else if (max_len > 0) {
        buf[0] = '\0';
    }
}

/* ============================================================
 * BANNER Y AUTENTICACION
 * ============================================================ */

void menu_banner(void) {
    printf("\n");
    printf("  ================================================\n");
    printf("\n");
    printf("    K I N E T I X\n");
    printf("\n");
    printf("    Gestion de Flota  .  Herramienta de Administracion\n");
    printf("\n");
    printf("  ================================================\n");
    printf("\n");
}

int menu_autenticar(void) {
    char usuario[64];
    char clave[64];
    int  intentos = 3;

    while (intentos > 0) {
        ui_limpiar();
        menu_banner();
        printf("  Acceso restringido. Identifiquese.\n\n");

        ui_leer_string("Usuario", usuario, sizeof(usuario));
        /* Ocultar la clave en consola no es portable en C puro,
           se lee normal pero se podria mejorar con termios en Linux */
        ui_leer_string("Clave", clave, sizeof(clave));

        if (strcmp(usuario, g_config.admin_usuario) == 0 &&
            strcmp(clave,   g_config.admin_clave)   == 0) {

            char msg[128];
            snprintf(msg, sizeof(msg), "Login correcto: usuario '%s'", usuario);
            LOG_AC(msg);
            printf("\n  Bienvenido, %s.\n", usuario);
            ui_pausa();
            return 1;
        }

        intentos--;
        char msg[128];
        snprintf(msg, sizeof(msg),
                 "Intento de login fallido: usuario '%s' (%d intentos restantes)",
                 usuario, intentos);
        LOG_A(msg);
        printf("\n  Credenciales incorrectas. Intentos restantes: %d\n", intentos);
        if (intentos > 0) ui_pausa();
    }

    LOG_E("Acceso bloqueado por demasiados intentos fallidos.");
    printf("\n  Demasiados intentos. El programa se cerrara.\n\n");
    return 0;
}

/* ============================================================
 * MENU PRINCIPAL
 * ============================================================ */

void menu_principal(void) {
    int opcion;

    do {
        ui_limpiar();
        menu_banner();
        printf("    Menu Principal\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Gestionar Estaciones\n");
        printf("    [ 2 ]  Gestionar Flota de Vehiculos\n");
        printf("    [ 3 ]  Gestionar Usuarios\n");
        printf("    [ 4 ]  Historico de Alquileres\n");
        printf("    [ 5 ]  Informes y Exportacion\n");
        printf("    [ 6 ]  Configuracion del Sistema\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Cerrar sesion\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 6);

        switch (opcion) {
            case 1: menu_estaciones();     break;
            case 2: menu_vehiculos();      break;
            case 3: menu_usuarios();       break;
            case 4: menu_historico();      break;
            case 5: menu_informes();       break;
            case 6: menu_configuracion();  break;
            case 0:
                LOG_AC("Sesion cerrada por el administrador.");
                printf("\n  ................................................\n");
                printf("  Hasta pronto.\n\n");
                break;
            default:
                printf("  Opcion no valida.\n");
                ui_pausa();
                break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU ESTACIONES
 * ============================================================ */

static void estacion_alta(void) {
    Estacion e;
    memset(&e, 0, sizeof(Estacion));
    int id_generado = 0;

    ui_limpiar();
    printf("\n  --- Alta de estacion ---\n");
    printf("  ................................................\n\n");

    do {
        ui_leer_string("Nombre", e.nombre, sizeof(e.nombre));
        if (esta_vacio(e.nombre)) {
            printf("  El nombre no puede estar vacio.\n");
        }
    } while (esta_vacio(e.nombre));

    ui_leer_string("Direccion", e.direccion, sizeof(e.direccion));

    float coord_x;
    if (!ui_leer_float_en_rango("Coordenada X", -100000.0f, 100000.0f, &coord_x)) {
        printf("  Error desconocido al insertar la coordenada.\n");
        ui_pausa();
        return;
    }

    e.coord_x = coord_x;

    float coord_y;
    if (!ui_leer_float_en_rango("Coordenada Y", -100000.0f, 100000.0f, &coord_y)) {
        printf("  Error desconocido al insertar la coordenada.\n");
        ui_pausa();
        return;
    }

    e.coord_y = coord_y;

    e.capacidad_max = ui_leer_int("Capacidad maxima", 1, 50);
    if (e.capacidad_max < 0) { printf("  Capacidad invalida.\n"); ui_pausa(); return; }

    e.disponibilidad_actual = ui_leer_int("Disponibilidad actual", 0, e.capacidad_max);
    if (e.disponibilidad_actual < 0) e.disponibilidad_actual = e.capacidad_max;

    int res = insertar_estacion(e, &id_generado);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta estacion ID=%d nombre='%s'",
                 id_generado, e.nombre);
        LOG_I(msg);
        printf("\n  Estacion creada correctamente. ID asignado: %d\n", id_generado);
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Error al crear estacion (cod=%d)", res);
        LOG_E(msg);
        printf("\n  Error al crear la estacion (codigo %d).\n", res);
    }
    ui_pausa();
}

static void estacion_baja(void) {
    ui_limpiar();
    printf("\n  --- Baja de estacion ---\n");
    printf("  ................................................\n\n");
    int id = ui_leer_int("ID de la estacion a eliminar", 1, 9999);
    if (id < 0) { ui_pausa(); return; }

    printf("\n  Confirmar eliminacion de estacion %d (s/n): ", id);
    char conf[8];
    fgets(conf, sizeof(conf), stdin);

    if (conf[0] != 's' && conf[0] != 'S') {
        printf("  Operacion cancelada.\n");
        ui_pausa();
        return;
    }

    int res = borrar_estacion(id);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Baja estacion ID=%d", id);
        LOG_I(msg);
        printf("  Estacion eliminada correctamente.\n");
    } else {
        printf("  No se encontro la estacion con ID %d.\n", id);
    }
    ui_pausa();
}

static void estacion_modificar(void) {
    ui_limpiar();
    printf("\n  --- Modificar estacion ---\n");
    printf("  ................................................\n\n");

    Estacion e;
    memset(&e, 0, sizeof(Estacion));

    e.id_estacion = ui_leer_int("ID de la estacion a modificar", 1, 9999);
    if (e.id_estacion < 0) { ui_pausa(); return; }

    do {
        ui_leer_string("Nuevo nombre", e.nombre, sizeof(e.nombre));
        if (esta_vacio(e.nombre)) {
            printf("  El nombre no puede estar vacio.\n");
        }
    } while (esta_vacio(e.nombre));

    ui_leer_string("Nueva direccion", e.direccion, sizeof(e.direccion));

    float coord_x;
    if (!ui_leer_float_en_rango("Coordenada X", -100000.0f, 100000.0f, &coord_x)) {
        printf("  Error desconocido al insertar la coordenada.\n");
        ui_pausa();
        return;
    }

    e.coord_x = coord_x;

    float coord_y;
    if (!ui_leer_float_en_rango("Coordenada Y", -100000.0f, 100000.0f, &coord_y)) {
        printf("  Error desconocido al insertar la coordenada.\n");
        ui_pausa();
        return;
    }

    e.coord_y = coord_y;

    e.capacidad_max         = ui_leer_int("Nueva capacidad maxima", 1, 200);
    e.disponibilidad_actual = ui_leer_int("Nueva disponibilidad", 0, e.capacidad_max > 0 ? e.capacidad_max : 200);

    int res = actualizar_estacion(e);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Modificacion estacion ID=%d", e.id_estacion);
        LOG_I(msg);
        printf("\n  Estacion actualizada correctamente.\n");
    } else {
        printf("\n  Error al actualizar (codigo %d).\n", res);
    }
    ui_pausa();
}

static void estacion_listado(void) {
    ui_limpiar();

    Estacion *estaciones = NULL;
    int n = 0;

    int rc = listar_estaciones(&estaciones, &n);
    if (rc != SQLITE_OK) {
        printf("Error al listar estaciones, error: %d.\n", rc);
        if (estaciones) free(estaciones);
        ui_pausa();
        return;
    }

    if (n == 0) {
        printf("No existen estaciones\n");
        if (estaciones) free(estaciones);
        ui_pausa();
        return;
    }

    printf("\n  --- Listado de estaciones ---\n");
    printf("  ................................................\n\n");
    printf("  %-6s  %-20s  %-20s  %-8s  %-8s  %-5s  %s\n",
           "ID", "Nombre", "Direccion", "CoordX", "CoordY", "Cap.", "Disp.");
    printf("  ------  --------------------  --------------------  --------  --------  -----  -----\n");

    for (int i = 0; i < n; i++) {
        printf("  %-6d  %-20s  %-20s  %-8.2f  %-8.2f  %-5d  %d\n",
               estaciones[i].id_estacion,
               estaciones[i].nombre,
               estaciones[i].direccion,
               estaciones[i].coord_x,
               estaciones[i].coord_y,
               estaciones[i].capacidad_max,
               estaciones[i].disponibilidad_actual);
    }

    printf("  ................................................\n");
    printf("  Total: %d estaciones.\n", n);
    free(estaciones);
    ui_pausa();
}

void menu_estaciones(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Estaciones\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Alta de estacion\n");
        printf("    [ 2 ]  Baja de estacion\n");
        printf("    [ 3 ]  Modificar estacion\n");
        printf("    [ 4 ]  Listar estaciones\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 4);

        switch (opcion) {
            case 1: estacion_alta(); break;
            case 2: estacion_baja(); break;
            case 3: estacion_modificar(); break;
            case 4: estacion_listado(); break;
            case 0: break;
            default: printf("  Opcion no valida.\n"); ui_pausa(); break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU VEHICULOS
 * ============================================================ */

static void vehiculo_alta(void) {
    Vehiculo v;
    memset(&v, 0, sizeof(Vehiculo));
    int id_generado = 0;

    ui_limpiar();
    printf("\n  --- Alta de vehiculo ---\n");
    printf("  ................................................\n\n");

    if (!ui_leer_opcion_char("Tipo (B=Bicicleta, P=Patinete): ", "BP", &v.tipo)) {
        printf("  Error al leer el tipo de vehiculo.\n");
        ui_pausa();
        return;
    }

    float bat;
    if (!ui_leer_float_en_rango("Bateria (%)", 0.0f, 100.0f, &bat)) {
        printf("  Error desconocido al insertar la bateria.\n");
        ui_pausa();
        return;
    }

    v.bateria = bat;

    if (!ui_leer_opcion_char("Estado (D=Disponible, R=Reservado, M=Mantenimiento, B=Bloqueado): ", "DRMB", &v.estado)) {
        printf("  Error al leer el estado del vehiculo.\n");
        ui_pausa();
        return;
    }

    v.id_estacion = ui_leer_int("ID estacion (0 si no tiene)", 0, 9999);

    int res = insertar_vehiculo(v, &id_generado);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta vehiculo ID=%d tipo=%c estado=%c",
                 id_generado, v.tipo, v.estado);
        LOG_I(msg);
        printf("\n  Vehiculo creado correctamente. ID asignado: %d\n", id_generado);
    } else if (res == SQLITE_NOTFOUND) {
        printf("\n  Error: La estacion no existe.\n");
    } else if (res == SQLITE_FULL) {
        printf("\n  Error: La estacion esta llena.\n");
    } else {
        printf("\n  Error al crear el vehiculo (codigo %d).\n", res);
    }
    ui_pausa();
}

static void vehiculo_baja(void) {
    ui_limpiar();
    printf("\n  --- Baja de vehiculo ---\n");
    printf("  ................................................\n\n");

    int id = ui_leer_int("ID del vehiculo a dar de baja", 1, 9999);
    if (id < 0) { ui_pausa(); return; }

    int res = dar_de_baja_vehiculo(id);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Baja vehiculo ID=%d", id);
        LOG_I(msg);
        printf("  Vehiculo dado de baja correctamente.\n");
    } else {
        printf("  No se encontro vehiculo con ID %d.\n", id);
    }
    ui_pausa();
}

static void vehiculo_modificar(void) {
    ui_limpiar();
    printf("\n  --- Modificar vehiculo ---\n");
    printf("  ................................................\n\n");

    Vehiculo v;
    memset(&v, 0, sizeof(Vehiculo));

    v.id_vehiculo = ui_leer_int("ID del vehiculo a modificar", 1, 9999);
    if (v.id_vehiculo < 0) { ui_pausa(); return; }

    if (!ui_leer_opcion_char("Nuevo tipo (B/P): ", "BP", &v.tipo)) {
        printf("  Error al leer el tipo de vehiculo.\n");
        ui_pausa();
        return;
    }

    float bat;
    if (!ui_leer_float_en_rango("Bateria (%)", 0.0f, 100.0f, &bat)) {
        printf("  Error desconocido al insertar la bateria.\n");
        ui_pausa();
        return;
    }

    v.bateria = bat;

    if (!ui_leer_opcion_char("Nuevo estado (D/R/M/B): ", "DRMB", &v.estado)) {
        printf("  Error al leer el estado del vehiculo.\n");
        ui_pausa();
        return;
    }

    v.id_estacion = ui_leer_int("Nuevo ID estacion (0 si ninguna)", 0, 9999);

    int res = actualizar_vehiculo(v);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Modificacion vehiculo ID=%d", v.id_vehiculo);
        LOG_I(msg);
        printf("\n  Vehiculo actualizado correctamente.\n");
    } else if (res == SQLITE_NOTFOUND) {
        printf("\n  Error: Vehiculo o estacion no encontrados.\n");
    } else if (res == SQLITE_FULL) {
        printf("\n  Error: La estacion indicada esta llena (sin huecos).\n");
    } else {
        printf("\n  Error al actualizar (codigo %d).\n", res);
    }
    ui_pausa();
}

static void vehiculo_listado(void) {
    ui_limpiar();

    Vehiculo *vehiculos = NULL;
    int n = 0;

    int rc = listar_vehiculos(&vehiculos, &n);
    if (rc != SQLITE_OK) {
        printf("Error al listar vehiculos, error: %d.\n", rc);
        if (vehiculos) free(vehiculos);
        ui_pausa();
        return;
    }

    if (n == 0) {
        printf("No existen vehiculos\n");
        if (vehiculos) free(vehiculos);
        ui_pausa();
        return;
    }

    printf("\n  --- Listado de vehiculos ---\n");
    printf("  ................................................\n\n");
    printf("  %-6s  %-10s  %-10s  %-12s  %s\n",
           "ID", "Tipo", "Bateria", "Estacion", "Estado");
    printf("  ------  ----------  ----------  ------------  ------\n");

    for (int i = 0; i < n; i++) {
        const char *tipo = (vehiculos[i].tipo == 'B') ? "Bicicleta" : "Patinete";
        printf("  %-6d  %-10s  %-9.1f%%  %-12d  %c\n",
               vehiculos[i].id_vehiculo,
               tipo,
               vehiculos[i].bateria,
               vehiculos[i].id_estacion,
               vehiculos[i].estado);
    }

    printf("  ................................................\n");
    printf("  Total: %d vehiculos.\n", n);
    free(vehiculos);
    ui_pausa();
}

void menu_vehiculos(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Flota de Vehiculos\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Alta de vehiculo\n");
        printf("    [ 2 ]  Baja de vehiculo\n");
        printf("    [ 3 ]  Modificar vehiculo\n");
        printf("    [ 4 ]  Listar vehiculos\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 4);

        switch (opcion) {
            case 1: vehiculo_alta(); break;
            case 2: vehiculo_baja(); break;
            case 3: vehiculo_modificar(); break;
            case 4: vehiculo_listado(); break;
            case 0: break;
            default: printf("  Opcion no valida.\n"); ui_pausa(); break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU USUARIOS
 * ============================================================ */

static void usuario_alta(void) {
    Usuario u;
    memset(&u, 0, sizeof(Usuario));
    int id_generado = 0;

    ui_limpiar();
    printf("\n  --- Alta de usuario ---\n");
    printf("  ................................................\n\n");

    dni_o_nie(u.dni, sizeof(u.dni));
    ui_leer_string_no_vacio("Nombre", u.nombre, sizeof(u.nombre));
    ui_leer_string_no_vacio("Contrasena", u.contrasena, sizeof(u.contrasena));

    float saldo;
    if (!ui_leer_float_en_rango("Saldo inicial (EUR)", 0.0f, 9999.0f, &saldo)) {
        printf("  Error desconocido al insertar el saldo.\n");
        ui_pausa();
        return;
    }

    u.saldo = saldo;

    int res = insertar_usuario(u, &id_generado);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta usuario ID=%d dni='%s'",
                 id_generado, u.dni);
        LOG_I(msg);
        printf("\n  Usuario creado correctamente. ID asignado: %d\n", id_generado);
    } else {
        printf("\n  Error al crear el usuario (codigo %d).\n", res);
    }
    ui_pausa();
}

static void usuario_baja(void) {
    ui_limpiar();
    printf("\n  --- Baja de usuario ---\n");
    printf("  ................................................\n\n");

    int id = ui_leer_int("ID del usuario a eliminar", 1, 99999);
    if (id < 0) { ui_pausa(); return; }

    int res = dar_de_baja_usuario(id);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Baja usuario ID=%d", id);
        LOG_I(msg);
        printf("  Usuario eliminado correctamente.\n");
    } else {
        printf("  No se encontro usuario con ID %d.\n", id);
    }
    ui_pausa();
}

static void usuario_modificar(void) {
    ui_limpiar();
    printf("\n  --- Modificar usuario ---\n");
    printf("  ................................................\n\n");

    Usuario u;
    memset(&u, 0, sizeof(Usuario));

    u.id_usuario = ui_leer_int("ID del usuario a modificar", 1, 99999);
    if (u.id_usuario < 0) { ui_pausa(); return; }

    dni_o_nie(u.dni, sizeof(u.dni));
    ui_leer_string_no_vacio("Nuevo nombre", u.nombre, sizeof(u.nombre));
    ui_leer_string_no_vacio("Nueva contrasena", u.contrasena, sizeof(u.contrasena));

    float saldo;
    if (!ui_leer_float_en_rango("Saldo inicial (EUR)", 0.0f, 9999.0f, &saldo)) {
        printf("  Error desconocido al insertar el saldo.\n");
        ui_pausa();
        return;
    }

    u.saldo = saldo;

    int res = actualizar_usuario(u);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Modificacion usuario ID=%d", u.id_usuario);
        LOG_I(msg);
        printf("\n  Usuario actualizado correctamente.\n");
    } else {
        printf("\n  Error al actualizar (codigo %d).\n", res);
    }
    ui_pausa();
}

static void usuario_listado(void) {
    ui_limpiar();

    Usuario *usuarios = NULL;
    int n = 0;

    int rc = listar_usuarios(&usuarios, &n);
    if (rc != SQLITE_OK) {
        printf("Error al listar usuarios, error: %d.\n", rc);
        if (usuarios) free(usuarios);
        ui_pausa();
        return;
    }

    if (n == 0) {
        printf("No existen usuarios\n");
        if (usuarios) free(usuarios);
        ui_pausa();
        return;
    }

    printf("\n  --- Listado de usuarios ---\n");
    printf("  ................................................\n\n");
    printf("  %-6s  %-12s  %-20s  %s\n", "ID", "DNI", "Nombre", "Saldo");
    printf("  ------  ------------  --------------------  --------\n");

    for (int i = 0; i < n; i++) {
        printf("  %-6d  %-12s  %-20s  %.2f\n",
               usuarios[i].id_usuario,
               usuarios[i].dni,
               usuarios[i].nombre,
               usuarios[i].saldo);
    }

    printf("  ................................................\n");
    printf("  Total: %d usuarios.\n", n);
    free(usuarios);
    ui_pausa();
}

void menu_usuarios(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Usuarios\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Alta de usuario\n");
        printf("    [ 2 ]  Baja de usuario\n");
        printf("    [ 3 ]  Modificar usuario\n");
        printf("    [ 4 ]  Listar usuarios\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 4);

        switch (opcion) {
            case 1: usuario_alta(); break;
            case 2: usuario_baja(); break;
            case 3: usuario_modificar(); break;
            case 4: usuario_listado(); break;
            case 0: break;
            default: printf("  Opcion no valida.\n"); ui_pausa(); break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU HISTORICO / LOGS
 * ============================================================ */

static void imprimir_alquileres(Alquiler *alquileres, int n) {
    if (n == 0) {
        printf("  No se han encontrado alquileres.\n");
        return;
    }

    printf("  ................................................\n\n");
    printf("  %-6s  %-8s  %-8s  %-10s  %-10s  %-19s  %-19s  %s\n",
           "ID", "Usuario", "Vehiculo", "Est.Orig.", "Est.Dest.", "Inicio", "Fin", "Coste");
    printf("  ------  --------  --------  ----------  ----------  -------------------  -------------------  --------\n");

    for (int i = 0; i < n; i++) {
        char dest[8];
        char fin[21];

        if (alquileres[i].id_estacion_destino > 0)
            snprintf(dest, sizeof(dest), "%d", alquileres[i].id_estacion_destino);
        else
            snprintf(dest, sizeof(dest), "-");

        if (strlen(alquileres[i].fecha_fin) > 0)
            strcpy(fin, alquileres[i].fecha_fin);
        else
            strcpy(fin, "En curso");

        printf("  %-6d  %-8d  %-8d  %-10d  %-10s  %-19s  %-19s  %.2f EUR\n",
               alquileres[i].id_alquiler,
               alquileres[i].id_usuario,
               alquileres[i].id_vehiculo,
               alquileres[i].id_estacion_origen,
               dest,
               alquileres[i].fecha_inicio,
               fin,
               alquileres[i].coste_total);
    }

    printf("  ................................................\n");
    printf("  Total: %d alquileres.\n", n);
}

void menu_historico(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Historico de Alquileres\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Ver todos los alquileres\n");
        printf("    [ 2 ]  Filtrar por usuario\n");
        printf("    [ 3 ]  Filtrar por fecha\n");
        printf("    [ 4 ]  Ver 20 ultimos alquileres\n");
        printf("    [ 5 ]  Ver 50 ultimos alquileres\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Selecciona una opcion", 0, 5);

        switch (opcion) {

            case 1: {
                ui_limpiar();
                printf("\n  --- Todos los alquileres ---\n");
                printf("  ................................................\n\n");
                Alquiler *alquieres = NULL;
                int cantidad = 0;
                listar_alquileres(&alquieres, &cantidad);
                imprimir_alquileres(alquieres, cantidad);
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 2: {
                ui_limpiar();
                printf("\n  --- Filtrar por usuario ---\n");
                printf("  ................................................\n\n");
                int id = ui_leer_int("ID de usuario", 1, 99999);
                if (id < 0) {
                    ui_pausa();
                    break;
                }

                Alquiler *alquieres = NULL;
                int cantidad = 0;
                buscar_alquiler_por_usuario(id, &alquieres, &cantidad);
                imprimir_alquileres(alquieres, cantidad);
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 3: {
                ui_limpiar();
                printf("\n  --- Filtrar por fecha ---\n");
                printf("  ................................................\n\n");
                int ano  = ui_leer_int("Ano (YYYY)", 2025, 2050);
                int mes   = ui_leer_int("Mes (1-12)", 1, 12);
                int dia   = ui_leer_int("Dia (1-31)", 1, 31);
                if (ano < 0 || mes < 0 || dia < 0) {
                    ui_pausa();
                    break;
                }
                char fecha[20];
                snprintf(fecha, sizeof(fecha), "%04d-%02d-%02d", ano, mes, dia);

                Alquiler *alquieres = NULL;
                int cantidad = 0;
                 buscar_alquiler_por_fecha(fecha, &alquieres, &cantidad);
                imprimir_alquileres(alquieres, cantidad);
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 4: {
                ui_limpiar();
                printf("\n  --- Ultimos 20 alquileres ---\n");
                printf("  ................................................\n\n");
                Alquiler *alquieres = NULL;
                int cantidad = 0;
                int rc = listar_ultimos_alquileres(20, &alquieres, &cantidad);
                if (rc != SQLITE_OK) {
                    printf("  Error al recuperar los ultimos alquileres. Error: %d\n", rc);
                } else {
                    imprimir_alquileres(alquieres, cantidad);
                }
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 5: {
                ui_limpiar();
                printf("\n  --- Ultimos 50 alquileres ---\n");
                printf("  ................................................\n\n");
                Alquiler *alquieres = NULL;
                int cantidad = 0;
                int rc = listar_ultimos_alquileres(50, &alquieres, &cantidad);
                if (rc != SQLITE_OK) {
                    printf("  Error al recuperar los ultimos alquileres. Error: %d\n", rc);
                } else {
                    imprimir_alquileres(alquieres, cantidad);
                }
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 0: break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU INFORMES
 * ============================================================ */

static int asegurar_directorio(const char *ruta) {
    if (!ruta || ruta[0] == '\0') return 0;

    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "if not exist \"%s\" mkdir \"%s\" >NUL 2>&1", ruta, ruta);
#else
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\" >/dev/null 2>&1", ruta);
#endif

    return (system(cmd) == 0);
}

static void generar_nombre_informe(const char *base_ruta, const char *tipo, const char *formato, char *resultado, int max) {
    //Se consigue el tiempo real
    time_t ahora;
    time(&ahora);
    struct tm *t = localtime(&ahora);

    //Se formatea la fecha
    char fecha[11];
    strftime(fecha, sizeof(fecha), "%Y%m%d", t);

    //Se crea el texto final
    if (!base_ruta || base_ruta[0] == '\0') base_ruta = "data/reportes";

    int len = (int)strlen(base_ruta);
    int tiene_sep = (len > 0 && (base_ruta[len - 1] == '/' || base_ruta[len - 1] == '\\'));
    snprintf(resultado, max, "%s%s%s_%s.%s",
             base_ruta,
             tiene_sep ? "" : "/",
             tipo,
             fecha,
             formato);
}

static FILE *abrir_fichero_informe(const char *tipo, const char *formato, char *ruta, int max_ruta) {
    const char *rutas_candidatas[3];
    int n = 0;

    if (g_config.reportes_ruta[0] != '\0') rutas_candidatas[n++] = g_config.reportes_ruta;
    rutas_candidatas[n++] = "data/reportes";
    rutas_candidatas[n++] = "../data/reportes";

    for (int i = 0; i < n; i++) {
        if (!asegurar_directorio(rutas_candidatas[i])) continue;
        generar_nombre_informe(rutas_candidatas[i], tipo, formato, ruta, max_ruta);

        FILE *f = fopen(ruta, "w");
        if (f) return f;
    }

    ruta[0] = '\0';
    return NULL;
}

static void informe_ocupacion(void) {
    ui_limpiar();
    printf("\n  --- Informe de ocupacion por estacion ---\n");
    printf("  ................................................\n\n");

    Estacion *estaciones = NULL;
    int cantidad = 0;
    int rc = listar_estaciones(&estaciones, &cantidad);

    if (rc != SQLITE_OK) {
        printf("  Error al recuperar estaciones. Error: %d\n", rc);
        ui_pausa();
        return;
    }

    if (cantidad <= 0 || estaciones == NULL) {
        printf("  No hay estaciones registradas");
        ui_pausa();
        return;
    }

    printf("  %-6s  %-20s  %-13s  %-11s  %s\n",
       "ID", "Nombre", "Cap. Max", "Disponibles", "Ocupacion");
    printf("  ------  --------------------  -------------  -----------  --------------------\n");

    for (int i = 0; i < cantidad; i++) {
        int capacidad = estaciones[i].capacidad_max;
        int disponibles = estaciones[i].disponibilidad_actual;

        if (capacidad < 0) capacidad = 0;
        if (disponibles < 0) disponibles = 0;
        if (disponibles > capacidad) disponibles = capacidad;

        float porcentaje = (capacidad > 0)
                            ? ((float)disponibles * 100.0f / (float)capacidad) : 0.0f;
        if (porcentaje < 0.0f) porcentaje = 0.0f;
        if (porcentaje > 100.0f) porcentaje = 100.0f;

        char barra[21];
        int llenos = (int)(porcentaje / 5.0f);
        for (int j = 0; j < 20; j++)
            barra[j] = (j < llenos) ? '#' : '.';
        barra[20] = '\0';

        printf("  %-6d  %-20s  %-13d  %-11d  %.1f%% [%s]\n",
               estaciones[i].id_estacion,
               estaciones[i].nombre,
               estaciones[i].capacidad_max,
               estaciones[i].disponibilidad_actual,
               porcentaje, barra);
    }

    printf("  ................................................\n");
    free(estaciones);
    ui_pausa();
}

static void informe_ranking_uso(void) {
    ui_limpiar();
    printf("\n  --- Ranking de uso de vehiculos ---\n");
    printf("  ................................................\n\n");

    UsoVehiculo *lista = NULL;
    int cantidad = 0;

    int rc = ranking_uso_vehiculo(&lista, &cantidad, 10);

    if (rc != SQLITE_OK) {
        printf("  Error al generar el ranking. Error: %d\n", rc);
        ui_pausa();
        return;
    }

    if (cantidad <= 0 || lista == NULL) {
        printf("  No hay datos de alquileres finalizados.\n");
        if (lista) free(lista);
        ui_pausa();
        return;
    }

    printf("  %-5s  %-8s  %-10s  %-13s  %s\n",
               "Pos.", "ID Veh.", "Tipo", "Alquileres", "Min. totales");
    printf("  -----  --------  ----------  -------------  ------------\n");

    for (int i = 0; i < cantidad; i++) {
        const char *tipo = (lista[i].tipo == 'B') ? "Bicicleta" : "Patinete";
        printf("  %-5d  %-8d  %-10s  %-13d  %.1f min\n",
               i + 1,
               lista[i].id_vehiculo,
               tipo,
               lista[i].num_alquileres,
               lista[i].minutos_totales);
    }

    printf("  ................................................\n");
    free(lista);
    ui_pausa();
}

static void informe_financiero(void) {
    ui_limpiar();
    printf("\n  --- Informe financiero ---\n");
    printf("  ................................................\n\n");

    //Por tipo de vehiculo
    ResumenPorTipo por_tipo[2];
    int rc_tipo = informe_recaudacion_por_tipo(por_tipo);
    if (rc_tipo != SQLITE_OK) {
        printf("  Error al calcular la recaudacion por tipo. Error: %d\n", rc_tipo);
    } else {
        printf("  Recaudacion por tipo de vehiculo:\n\n");
        printf("  %-12s  %-14s  %s\n", "Tipo", "Alquileres", "Recaudacion");
        printf("  ------------  --------------  ------------\n");

        const char *nombres[2] = {"Bicicleta", "Patinete"};
        float total = 0.0f;
        for (int i = 0; i < 2; i++) {
            printf("  %-12s  %-14d  %.2f EUR\n",
                   nombres[i], por_tipo[i].num_alquileres, por_tipo[i].recaudacion);
            total += por_tipo[i].recaudacion;
        }
        printf("  ............  ..............  ............\n");
        printf("  %-12s  %-14s  %.2f EUR\n\n", "Total", "", total);
    }

    //Por dia
    ResumenPorDia *por_dia = NULL;
    int cantidad = 0;
    int rc_dia = informe_recaudacion_por_dia(&por_dia, &cantidad);

    if (rc_dia != SQLITE_OK) {
        printf("  Error al calcular la recaudacion por dia. Error: %d\n", rc_dia);
        if (por_dia) free(por_dia);
        ui_pausa();
        return;
    }

    if (cantidad <= 0 || por_dia == NULL) {
        printf("  No hay datos diarios para mostrar.\n");
        if (por_dia) free(por_dia);
        ui_pausa();
        return;
    }

    printf("  Recaudacion por dia:\n\n");
    printf("  %-12s  %-14s  %s\n", "Fecha", "Alquileres", "Recaudacion");
    printf("  ------------  --------------  ------------\n");

    for (int i = 0; i < cantidad; i++) {
        printf("  %-12s  %-14d  %.2f EUR\n",
               por_dia[i].fecha, por_dia[i].num_alquileres, por_dia[i].recaudacion);
    }

    //Exportar CSV
    printf("\n  Exportar a CSV? (s/n): ");
    char conf[4];
    fgets(conf, sizeof(conf), stdin);
    if (conf[0] == 's' || conf[0] == 'S') {
        char ruta[128];
        FILE *f = abrir_fichero_informe("report_financiero", "csv", ruta, sizeof(ruta));
        if (f) {
            int seguir = 1;
            if (fprintf(f, "Fecha,Num_Alquileres,Recaudacion_EUR\n") < 0) seguir = 0;
            for (int i = 0; i < cantidad && seguir; i++)
                if (fprintf(f, "%s,%d,%.2f\n",
                        por_dia[i].fecha, por_dia[i].num_alquileres, por_dia[i].recaudacion) < 0) seguir = 0;
            if (fclose(f) != 0) seguir = 0;

            if (seguir) {
                printf("  Exportado a: %s\n", ruta);
                LOG_I(ruta);
            } else {
                printf("  Error escribiendo el CSV.\n");
            }
        } else {
            printf("  Error al crear el fichero.\n");
        }
    }

    free(por_dia);
    ui_pausa();
}

static void informe_incidencias(void) {
    ui_limpiar();
    printf("\n  --- Informe de incidencias ---\n");
    printf("  ................................................\n\n");

    Vehiculo *lista = NULL;
    int cantidad = 0;
    int umbral = g_config.bateria_minima > 0 ? g_config.bateria_minima : 20;

    if (listar_vehiculos_bateria_baja(&lista, &cantidad, umbral) != 0 || cantidad == 0) {
        printf("  No hay vehiculos con bateria baja ni incidencias.\n");
        ui_pausa();
        return;
    }

    printf("  Vehiculos con bateria < %d%% o estado critico:\n\n", umbral);
    printf("  %-6s  %-10s  %-10s  %-15s  %s\n",
           "ID", "Tipo", "Bateria", "Estado", "Estacion");
    printf("  ------  ----------  ----------  ---------------  --------\n");

    for (int i = 0; i < cantidad; i++) {
        const char *tipo = (lista[i].tipo == 'B') ? "Bicicleta" : "Patinete";
        const char *estado;
        switch (lista[i].estado) {
            case 'D': estado = "Disponible"; break;
            case 'R': estado = "Rentado";    break;
            case 'M': estado = "Mantenimiento";      break;
            case 'B': estado = "Bloqueado";  break;
            default:  estado = "?";          break;
        }
        char est_str[8];
        if (lista[i].id_estacion > 0)
            snprintf(est_str, sizeof(est_str), "%d", lista[i].id_estacion);
        else
            snprintf(est_str, sizeof(est_str), "-");

        printf("  %-6d  %-10s  %-9.1f%%  %-15s  %s\n",
               lista[i].id_vehiculo, tipo,
               lista[i].bateria, estado, est_str);
    }

    /* Exportar TXT */
    printf("  ................................................\n");
    printf("\n  Exportar a TXT? (s/n): ");
    char conf[4];
    fgets(conf, sizeof(conf), stdin);
    if (conf[0] == 's' || conf[0] == 'S') {
        char ruta[128];
        FILE *f = abrir_fichero_informe("informe_incidencias", "txt", ruta, sizeof(ruta));
        if (f) {
            int seguir = 1;
            if (fprintf(f, "INFORME DE INCIDENCIAS - Bateria < %d%%\n", umbral) < 0) seguir = 0;
            for (int i = 0; i < cantidad && seguir; i++) {
                const char *tipo = (lista[i].tipo == 'B') ? "Bici" : "Patinete";
                char est_str[8];
                if (lista[i].id_estacion > 0)
                    snprintf(est_str, sizeof(est_str), "%d", lista[i].id_estacion);
                else
                    snprintf(est_str, sizeof(est_str), "-");
                if (fprintf(f, "%d %s %.1f %c %s\n",
                        lista[i].id_vehiculo, tipo,
                        lista[i].bateria, lista[i].estado, est_str) < 0) seguir = 0;
            }
            if (fclose(f) != 0) seguir = 0;

            if (seguir) {
                printf("  Exportado a: %s\n", ruta);
                LOG_I(ruta);
            } else {
                printf("  Error escribiendo el fichero.\n");
            }
        } else {
            printf("  Error al crear el fichero.\n");
        }
    }

    free(lista);
    ui_pausa();
}

void menu_informes(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Informes y Exportacion\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Informe de ocupacion\n");
        printf("    [ 2 ]  Ranking de uso de vehiculos\n");
        printf("    [ 3 ]  Resumen financiero\n");
        printf("    [ 4 ]  Informe de incidencias\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione una opcion", 0, 4);

        switch (opcion) {
            case 1: informe_ocupacion(); break;
            case 2: informe_ranking_uso(); break;
            case 3: informe_financiero(); break;
            case 4: informe_incidencias(); break;
            case 0: break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU CONFIGURACION
 * ============================================================ */

void menu_configuracion(void) {
    int opcion;
    do {
        ui_limpiar();
        menu_banner();
        printf("    Configuracion del Sistema\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Ver configuracion actual\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Volver al menu principal\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 1);

        switch (opcion) {
            case 1:
                config_mostrar();
                ui_pausa();
                break;
            case 0:
                break;
        }
    } while (opcion != 0);
}