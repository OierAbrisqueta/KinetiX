//
// Created by jon.i on 26/03/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "gestor_config.h"
#include "gestor_log.h"
#include "gestor_bd.h"
#include "modelos.h"

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
    printf("\n  Pulse Enter para continuar...");
    /* Limpiar buffer antes de esperar */
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ui_separador(void) {
    printf("  +--------------------------------------------------+\n");
}

int ui_leer_int(const char *prompt, int min, int max) {
    char buf[64];
    int  val;

    printf("  %s [%d-%d]: ", prompt, min, max);
    if (!fgets(buf, sizeof(buf), stdin)) return -1;

    if (sscanf(buf, "%d", &val) != 1) return -1;
    if (val < min || val > max) {
        printf("  Valor fuera de rango.\n");
        return -1;
    }
    return val;
}

void ui_leer_string(const char *prompt, char *buf, int max_len) {
    printf("  %s: ", prompt);
    if (fgets(buf, max_len, stdin)) {
        int len = (int)strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    }
}

float ui_leer_float(const char *prompt, float min, float max) {
    char  buf[64];
    float val;

    printf("  %s [%.2f-%.2f]: ", prompt, min, max);
    if (!fgets(buf, sizeof(buf), stdin)) return min;
    if (sscanf(buf, "%f", &val) != 1) return min;
    if (val < min || val > max) return min;
    return val;
}

/* ============================================================
 * BANNER Y AUTENTICACION
 * ============================================================ */

void menu_banner(void) {
    ui_limpiar();
    printf("\n");
    printf("  ##################################################\n");
    printf("  ##                                              ##\n");
    printf("  ##   K I N E T I X  -  Gestion de Flota        ##\n");
    printf("  ##        Herramienta de Administracion         ##\n");
    printf("  ##                       Servidor Local         ##\n");
    printf("  ##                                              ##\n");
    printf("  ##################################################\n");
    printf("\n");
}

int menu_autenticar(void) {
    char usuario[64];
    char clave[64];
    int  intentos = 3;

    while (intentos > 0) {
        menu_banner();
        printf("  Acceso restringido. Identifiquese.\n\n");

        ui_leer_string("Usuario", usuario, sizeof(usuario));
        /* Ocultar la clave en consola no es portable en C puro,
           se lee normal pero se podria mejorar con termios en Linux */
        ui_leer_string("Clave  ", clave, sizeof(clave));

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
        ui_separador();
        printf("  |         MENU PRINCIPAL - ADMINISTRADOR          |\n");
        ui_separador();
        printf("  |  1. Gestionar Estaciones                         |\n");
        printf("  |  2. Gestionar Flota de Vehiculos                 |\n");
        printf("  |  3. Gestionar Usuarios                           |\n");
        printf("  |  4. Consultar Historico (Logs)                   |\n");
        printf("  |  5. Informes y Exportacion                       |\n");
        printf("  |  6. Configuracion del Sistema                    |\n");
        printf("  |  0. Cerrar sesion y salir                        |\n");
        ui_separador();

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
                printf("\n  Hasta pronto.\n\n");
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
    char buf[16];

    ui_limpiar();
    printf("\n  --- ALTA DE ESTACION ---\n\n");

    /* ID */
    e.id_estacion = ui_leer_int("ID estacion", 1, 9999);
    if (e.id_estacion < 0) { printf("  ID invalido.\n"); ui_pausa(); return; }

    ui_leer_string("Nombre",    e.nombre,    sizeof(e.nombre));
    ui_leer_string("Direccion", e.direccion, sizeof(e.direccion));

    e.capacidad_max = ui_leer_int("Capacidad maxima", 1, 200);
    if (e.capacidad_max < 0) { printf("  Capacidad invalida.\n"); ui_pausa(); return; }

    e.disponibilidad_actual = ui_leer_int("Disponibilidad actual", 0, e.capacidad_max);
    if (e.disponibilidad_actual < 0) e.disponibilidad_actual = e.capacidad_max;

    int res = insertar_estacion(e);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta estacion ID=%d nombre='%s'",
                 e.id_estacion, e.nombre);
        LOG_I(msg);
        printf("\n  Estacion creada correctamente.\n");
    } else {
        char msg[128];
        snprintf(msg, sizeof(msg), "Error al crear estacion ID=%d (cod=%d)",
                 e.id_estacion, res);
        LOG_E(msg);
        printf("\n  Error al crear la estacion (codigo %d).\n", res);
    }
    ui_pausa();
}

static void estacion_baja(void) {
    ui_limpiar();
    printf("\n  --- BAJA DE ESTACION ---\n\n");

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
    printf("\n  --- MODIFICAR ESTACION ---\n\n");

    Estacion e;
    memset(&e, 0, sizeof(Estacion));

    e.id_estacion = ui_leer_int("ID de la estacion a modificar", 1, 9999);
    if (e.id_estacion < 0) { ui_pausa(); return; }

    ui_leer_string("Nuevo nombre",    e.nombre,    sizeof(e.nombre));
    ui_leer_string("Nueva direccion", e.direccion, sizeof(e.direccion));
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
    printf("\n  --- LISTADO DE ESTACIONES ---\n\n");

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

    printf("ID Nombre Direccion Capacidad Max Disponibles\n");

    for (int i = 0; i < n; i++) {
        printf("%d %s %s %d %d\n",
               estaciones[i].id_estacion,
               estaciones[i].nombre,
               estaciones[i].direccion,
               estaciones[i].capacidad_max,
               estaciones[i].disponibilidad_actual);
    }

    printf("Total: %d estaciones.\n", n);
    free(estaciones);
    ui_pausa();
}

void menu_estaciones(void) {
    int opcion;
    do {
        ui_limpiar();
        ui_separador();
        printf("  |           GESTION DE ESTACIONES                 |\n");
        ui_separador();
        printf("  |  1. Alta de estacion                             |\n");
        printf("  |  2. Baja de estacion                             |\n");
        printf("  |  3. Modificar estacion                           |\n");
        printf("  |  4. Listar estaciones                            |\n");
        printf("  |  0. Volver al menu principal                     |\n");
        ui_separador();

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
    char buf[8];

    ui_limpiar();
    printf("\n  --- ALTA DE VEHICULO ---\n\n");

    v.id_vehiculo = ui_leer_int("ID vehiculo", 1, 9999);
    if (v.id_vehiculo < 0) { ui_pausa(); return; }

    printf("  Tipo (B=Bicicleta, P=Patinete): ");
    fgets(buf, sizeof(buf), stdin);
    v.tipo = (buf[0] == 'b') ? 'B' : (buf[0] == 'p' ? 'P' : buf[0]);
    if (v.tipo != 'B' && v.tipo != 'P') {
        printf("  Tipo invalido. Use B o P.\n");
        ui_pausa();
        return;
    }

    float bat = ui_leer_float("Bateria (%)", 0.0f, 100.0f);
    v.bateria = bat;

    printf("  Estado (D=Disponible, R=Reservado, M=Mantenimiento, B=Bloqueado): ");
    fgets(buf, sizeof(buf), stdin);
    v.estado = buf[0];
    if (v.estado != 'D' && v.estado != 'R' && v.estado != 'M' && v.estado != 'B') {
        printf("  Estado invalido.\n");
        ui_pausa();
        return;
    }

    v.id_estacion = ui_leer_int("ID estacion (0 si no tiene)", 0, 9999);
    if (v.id_estacion < 0) v.id_estacion = 0;

    int res = insertar_vehiculo(v);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta vehiculo ID=%d tipo=%c estado=%c",
                 v.id_vehiculo, v.tipo, v.estado);
        LOG_I(msg);
        printf("\n  Vehiculo creado correctamente.\n");
    } else {
        printf("\n  Error al crear el vehiculo (codigo %d).\n", res);
    }
    ui_pausa();
}

static void vehiculo_baja(void) {
    ui_limpiar();
    printf("\n  --- BAJA DE VEHICULO ---\n\n");

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
    printf("\n  --- MODIFICAR VEHICULO ---\n\n");

    Vehiculo v;
    memset(&v, 0, sizeof(Vehiculo));
    char buf[8];

    v.id_vehiculo = ui_leer_int("ID del vehiculo a modificar", 1, 9999);
    if (v.id_vehiculo < 0) { ui_pausa(); return; }

    printf("  Nuevo tipo (B/P): ");
    fgets(buf, sizeof(buf), stdin);
    v.tipo = buf[0];

    v.bateria = ui_leer_float("Nueva bateria (%)", 0.0f, 100.0f);

    printf("  Nuevo estado (D/R/M/B): ");
    fgets(buf, sizeof(buf), stdin);
    v.estado = buf[0];

    v.id_estacion = ui_leer_int("Nuevo ID estacion (0 si ninguna)", 0, 9999);

    int res = actualizar_vehiculo(v);
    if (res == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Modificacion vehiculo ID=%d", v.id_vehiculo);
        LOG_I(msg);
        printf("\n  Vehiculo actualizado correctamente.\n");
    } else {
        printf("\n  Error al actualizar (codigo %d).\n", res);
    }
    ui_pausa();
}

static void vehiculo_listado(void) {
    ui_limpiar();
    printf("\n  --- LISTADO DE VEHICULOS ---\n\n");

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

    printf("ID Tipo Bateria id_estacion estado\n");

    for (int i = 0; i < n; i++) {
        printf("%d %c %f %d %c\n",
               vehiculos[i].id_vehiculo,
               vehiculos[i].tipo,
               vehiculos[i].bateria,
               vehiculos[i].id_estacion,
               vehiculos[i].estado);
    }

    printf("Total: %d vehiculos.\n", n);
    free(vehiculos);
    ui_pausa();
}

void menu_vehiculos(void) {
    int opcion;
    do {
        ui_limpiar();
        ui_separador();
        printf("  |         GESTION DE FLOTA DE VEHICULOS           |\n");
        ui_separador();
        printf("  |  1. Alta de vehiculo                             |\n");
        printf("  |  2. Baja de vehiculo                             |\n");
        printf("  |  3. Modificar vehiculo                           |\n");
        printf("  |  4. Listar vehiculos                              |\n");
        printf("  |  0. Volver al menu principal                     |\n");
        ui_separador();

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

    ui_limpiar();
    printf("\n  --- ALTA DE USUARIO ---\n\n");

    u.id_usuario = ui_leer_int("ID usuario", 1, 99999);
    if (u.id_usuario < 0) { ui_pausa(); return; }

    ui_leer_string("DNI (9 chars)", u.dni,        sizeof(u.dni));
    ui_leer_string("Nombre",        u.nombre,     sizeof(u.nombre));
    ui_leer_string("Contrasena",    u.contrasena, sizeof(u.contrasena));
    u.saldo = ui_leer_float("Saldo inicial (EUR)", 0.0f, 9999.0f);

    int res = insertar_usuario(u);
    if (res == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Alta usuario ID=%d dni='%s'",
                 u.id_usuario, u.dni);
        LOG_I(msg);
        printf("\n  Usuario creado correctamente.\n");
    } else {
        printf("\n  Error al crear el usuario (codigo %d).\n", res);
    }
    ui_pausa();
}

static void usuario_baja(void) {
    ui_limpiar();
    printf("\n  --- BAJA DE USUARIO ---\n\n");

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
    printf("\n  --- MODIFICAR USUARIO ---\n\n");

    Usuario u;
    memset(&u, 0, sizeof(Usuario));

    u.id_usuario = ui_leer_int("ID del usuario a modificar", 1, 99999);
    if (u.id_usuario < 0) { ui_pausa(); return; }

    ui_leer_string("Nuevo DNI",       u.dni,        sizeof(u.dni));
    ui_leer_string("Nuevo nombre",    u.nombre,     sizeof(u.nombre));
    ui_leer_string("Nueva contrasena",u.contrasena, sizeof(u.contrasena));
    u.saldo = ui_leer_float("Nuevo saldo (EUR)", 0.0f, 9999.0f);

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
    printf("\n  --- LISTADO DE USUARIOS ---\n\n");

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

    printf("ID DNI Nombre saldo\n");

    for (int i = 0; i < n; i++) {
        printf("%d %s %s %f\n",
               usuarios[i].id_usuario,
               usuarios[i].dni,
               usuarios[i].nombre,
               usuarios[i].saldo);
    }

    printf("Total: %d usuarios.\n", n);
    free(usuarios);
    ui_pausa();
}

void menu_usuarios(void) {
    int opcion;
    do {
        ui_limpiar();
        ui_separador();
        printf("  |              GESTION DE USUARIOS                |\n");
        ui_separador();
        printf("  |  1. Alta de usuario                              |\n");
        printf("  |  2. Baja de usuario                              |\n");
        printf("  |  3. Modificar usuario                            |\n");
        printf("  |  4. Listar usuarios                              |\n");
        printf("  |  0. Volver al menu principal                     |\n");
        ui_separador();

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
        printf("    No se han encontrado alquileres");
    }

    printf("ID Usuario Vehiculo Est.Origen Est.Destino Inicio Fin Coste");

    for (int i = 0; i < n; i++) {
        char dest[5];
        char fin[21];

        if (alquileres[i].id_estacion_destino > 0) {
            snprintf(dest, sizeof(dest), "%d", alquileres[i].id_estacion_destino);
        } else {
            snprintf(dest, sizeof(dest), "-");
        }

        if (strlen(alquileres[i].fecha_fin) > 0)
            strcpy(fin, alquileres[i].fecha_fin);
        else {
            strcpy(fin, "En curso");
        }

        printf("  %d %d %d %d %s %s %s %f EUR\n",
               alquileres[i].id_alquiler,
               alquileres[i].id_usuario,
               alquileres[i].id_vehiculo,
               alquileres[i].id_estacion_origen,
               dest,
               alquileres[i].fecha_inicio,
               fin,
               alquileres[i].coste_total);
    }
    printf("    Total: %d alquileres", n);
}

void menu_historico(void) {
    int opcion;
    do {
        ui_limpiar();
        ui_separador();
        printf("  |                    HISTORICO                     |\n");
        ui_separador();
        printf("  |  1. Ver todos los alquileres                     |\n");
        printf("  |  2. Filtrar por usuario                          |\n");
        printf("  |  3. Filtrar por fecha (Formato: YYYY/MM/DD)      |\n");
        printf("  |  4. Ver 20 últimos alquileres                    |\n");
        printf("  |  5. Ver 50 últimos alquileres                    |\n");
        printf("  |  0. Volver al menu principal                     |\n");
        ui_separador();

        opcion = ui_leer_int("Selecciona una opcion", 0, 5);

        switch (opcion) {

            case 1: {
                ui_limpiar();
                printf("      TODOS LOS ALQUILERES \n");
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
                printf("      FILTRAR POR USUARIO \n\n");
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
                printf("      FILTRAR POR FECHA \n\n");
                int ano  = ui_leer_int("Año (YYYY)", 2025, 2050);
                int mes   = ui_leer_int("Mes (1-12)", 1, 12);
                int dia   = ui_leer_int("Dia (1-31)", 1, 31);
                if (ano < 0 || mes < 0 || dia < 0) {
                    ui_pausa();
                    break;
                }
                char fecha[20];
                snprintf(fecha, sizeof(fecha), "%d-%d-%d", ano, mes, dia);

                Alquiler *alquieres = NULL;
                int cantidad = 0;
                 buscar_alquiler_por_fecha(fecha, &alquieres, &cantidad);
                imprimir_alquileres(alquieres, cantidad);
                if (alquieres) free(alquieres);
                ui_pausa();
                break;
            }

            case 4: {
                log_mostrar_ultimas(20);
                ui_pausa();
                break;
            }

            case 5: {
                log_mostrar_ultimas(50);
                ui_pausa();
                break;
            }

            case 0: break;
        }
    } while (opcion != 0);
}

/* ============================================================
 * SUBMENU INFORMES (stub — se desarrolla en Paso 7)
 * ============================================================ */

/*
*Informe de Ocupación: Listado de estaciones con el porcentaje de anclajes usados vs.
libres.
Ranking de Uso: Vehículos con más kilómetros o tiempo de uso (útil para mantenimiento).
Resumen Financiero: Recaudación total por día o por tipo de vehículo (Bici vs. Patinete).
Informe de Incidencias: Listado de vehículos con batería baja (<20%) o marcados como
"averiados"
.*/

void menu_informes(void) {
    int opcion;
    do {
        ui_limpiar();
        ui_separador();
        printf("  |                     INFORMES                     |\n");
        ui_separador();
        printf("  |  1. Informe de Ocupacion                         |\n");
        printf("  |  2. Ranking de uso de vehiculos                  |\n");
        printf("  |  3. Resumen Financiero (Dia o Tipo de vehiculo)  |\n");
        printf("  |  4. Informe de Incidencias (Bateria o averiados) |\n");
        printf("  |  0. Volver al menu principal                     |\n");
        ui_separador();

        opcion = ui_leer_int("Seleccione una opcion", 0, 4);

        switch (opcion) {

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
        ui_separador();
        printf("  |         CONFIGURACION DEL SISTEMA               |\n");
        ui_separador();
        printf("  |  1. Ver configuracion actual                     |\n");
        printf("  |  0. Volver                                       |\n");
        ui_separador();

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