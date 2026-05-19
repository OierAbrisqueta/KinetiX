#include "modelos_cliente.h"
#include "protocolo.h"
#include "gestor_config.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
static SOCKET g_sock = INVALID_SOCKET;
#else
using SocketHandle = int;
static const SocketHandle INVALID_SOCKET = -1;
static SocketHandle g_sock = INVALID_SOCKET;
#endif

static int   g_id_usuario      = 0;
static char  g_nombre[64]      = {0};
static char  g_dni[32]         = {0};
static float g_saldo           = 0.0f;
static int   g_alquiler_activo = 0;  // 0 = sin alquiler en curso
static int   g_vehiculo_activo = 0;
static char  g_tipo_vehiculo_activo = '\0';

void net_enviar(const char *msg);
void net_recibir_linea(char *buf, int tam);

//Sincroniza el saldo del usuario con el servidor.
static void refrescar_saldo(void) {
    if (g_id_usuario <= 0) return;

    char comando[64];
    snprintf(comando, sizeof(comando), CMD_GET_USUARIO " %d\n", g_id_usuario);
    net_enviar(comando);

    char resp[64];
    net_recibir_linea(resp, sizeof(resp));
    if (strcmp(resp, RESP_OK) != 0) return;

    char linea[PROTO_BUFF_SIZE];
    net_recibir_linea(linea, sizeof(linea));

    int id = 0;
    char dni[32];
    char nombre[64];
    float saldo = 0.0f;
    if (sscanf(linea, "%d|%31[^|]|%63[^|]|%f", &id, dni, nombre, &saldo) == 4 && id == g_id_usuario) {
        g_saldo = saldo;
    }
}

// Conecta al servidor. Devuelve 0 si ok, -1 si error.
int net_conectar(const char *ip, int puerto) {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sock == INVALID_SOCKET) return -1;

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port   = htons(puerto);
    inet_pton(AF_INET, ip, &srv.sin_addr);

    if (connect(g_sock, (struct sockaddr *)&srv, sizeof(srv)) != 0) {
#ifdef _WIN32
        closesocket(g_sock);
#else
        close(g_sock);
#endif
        return -1;
    }
    return 0;
}

// Envia un mensaje al servidor
void net_enviar(const char *msg) {
    send(g_sock, msg, strlen(msg), 0);
}

// Recibe una linea del servidor (hasta '\n')
void net_recibir_linea(char *buf, int tam) {
    int i = 0;
    while (i < tam - 1) {
        char c;
        if (recv(g_sock, &c, 1, 0) <= 0) break;
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = '\0';
    // Quitar el salto de linea del final
    if (i > 0 && buf[i-1] == '\n') buf[i-1] = '\0';
}

// Envia un comando y guarda la primera linea de respuesta en buf
void net_cmd(const char *comando, char *buf, int tam) {
    char msg[PROTO_BUFF_SIZE];
    snprintf(msg, sizeof(msg), "%s\n", comando);
    net_enviar(msg);
    net_recibir_linea(buf, tam);
}


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
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void menu_banner(void) {
    printf("\n");
    printf("  ================================================\n");
    printf("\n");
    printf("    K I N E T I X\n");
    printf("\n");
    printf("    Gestion de Flota  .  Cliente Remoto\n");
    printf("\n");
    printf("  ================================================\n");
    printf("\n");
}

// Limpia el buffer de teclado
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Lee un entero en un rango. Valida que sea un numero y este en rango.
int ui_leer_int(const char *msg, int min, int max) {
    char buf[64];
    int  valor;
    char extra;

    while (1) {
        printf("  %s [%d-%d]: ", msg, min, max);

        if (!fgets(buf, sizeof(buf), stdin)) continue;

        // Si no cabe en el buffer, limpiamos el resto
        if (strchr(buf, '\n') == NULL) limpiar_buffer();

        // Verificar que sea un entero sin caracteres extra
        if (sscanf(buf, " %d %c", &valor, &extra) != 1) {
            printf("  Error: Debes introducir un entero valido sin letras.\n");
            continue;
        }

        if (valor < min || valor > max) {
            printf("  Error: El numero debe estar entre %d y %d.\n", min, max);
            continue;
        }

        return valor;
    }
}

// Lee una cadena de texto
void ui_leer_string(const char *msg, char *buf, int max) {
    printf("  %s:  ", msg);
    if (fgets(buf, max, stdin)) {
        int len = (int)strlen(buf);
        if (len > 0 && buf[len-1] == '\n') {
            buf[len-1] = '\0';
        } else {
            limpiar_buffer();
        }
    } else if (max > 0) {
        buf[0] = '\0';
    }
}


// Muestra el listado de todas las estaciones
static void ver_estaciones(void) {
    ui_limpiar();
    printf("\n  --- Listado de estaciones ---\n");
    printf("  ................................................\n\n");

    // Pedimos la lista al servidor
    net_enviar(CMD_LIST_ESTACIONES "\n");

    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    if (n == 0) {
        printf("  No hay estaciones registradas.\n");
        ui_pausa();
        return;
    }

    printf("  %-6s  %-25s  %-10s  %s\n", "ID", "Nombre", "Libres", "Ocupacion");
    printf("  ------  -------------------------  ----------  ---------\n");

    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        Estacion e = Estacion::fromString(linea);

        char libres[16];
        snprintf(libres, sizeof(libres), "%d/%d",
                 e.disponibilidad_actual, e.capacidad_max);

        printf("  %-6d  %-25s  %-10s  %d%%\n",
               e.id_estacion, e.nombre, libres, (int)e.getOcupacion());
    }

    printf("  ................................................\n");
    printf("  Total: %d estaciones.\n", n);
    ui_pausa();
}

// Muestra los vehiculos disponibles para alquilar
static void ver_vehiculos_disponibles(void) {
    ui_limpiar();
    printf("\n  --- Vehiculos disponibles ---\n");
    printf("  ................................................\n\n");

    net_enviar(CMD_LIST_VEHICULOS "\n");

    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    printf("  %-6s  %-12s  %-10s  %-10s  %s\n",
           "ID", "Tipo", "Bateria", "Estacion", "Tarifa/min");
    printf("  ------  ------------  ----------  ----------  ----------\n");

    int mostrados = 0;
    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        // Parsear: id|tipo|bateria|id_estacion|estado
        int id, id_est;
        char tipo[2] = {0}, estado[2] = {0};
        float bateria;
        sscanf(linea, "%d|%1s|%f|%d|%1s", &id, tipo, &bateria, &id_est, estado);

        // Solo mostramos los disponibles y con bateria suficiente
        if (estado[0] == 'D' && bateria > 15) {
            const char *nombre_tipo = (tipo[0] == 'B') ? "Bicicleta" : "Patinete";
            float tarifa = (tipo[0] == 'B') ? 0.05f : 0.07f;
            printf("  %-6d  %-12s  %-9.1f%%  %-10d  %.2f EUR\n",
                   id, nombre_tipo, bateria, id_est, tarifa);
            mostrados++;
        }
    }

    if (mostrados == 0)
        printf("  No hay vehiculos disponibles en este momento.\n");

    printf("  ................................................\n");
    printf("  Total disponibles: %d vehiculos.\n", mostrados);
    ui_pausa();
}

// Permite al usuario alquilar un vehiculo
static void alquilar(void) {
    ui_limpiar();
    printf("\n  --- Alquilar vehiculo ---\n");
    printf("  ................................................\n\n");

    if (g_alquiler_activo != 0) {
        printf("  Ya tienes un alquiler en curso (ID: %d).\n", g_alquiler_activo);
        printf("  Devuelve el vehiculo actual antes de alquilar otro.\n");
        ui_pausa();
        return;
    }

    if (g_saldo <= 0.0f) {
        printf("  Saldo insuficiente (%.2f EUR).\n", g_saldo);
        printf("  Recarga tu saldo para poder alquilar.\n");
        ui_pausa();
        return;
    }

    // --- PASO 1: Mostrar estaciones ---
    net_enviar(CMD_LIST_ESTACIONES "\n");
    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n_est = atoi(buf);

    printf("  %-6s  %-26s  %s\n", "ID", "Nombre", "Disponibles");
    printf("  ------  --------------------------  -----------\n");

    int ids_est[512];
    int n_ids_est = 0;
    int max_id_est = 1;

    for (int i = 0; i < n_est; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        int id_e, cap, disp;
        char nombre[51] = {0}, dir[101] = {0};
        float cx, cy;
        sscanf(linea, "%d|%50[^|]|%100[^|]|%f|%f|%d|%d",
               &id_e, nombre, dir, &cx, &cy, &cap, &disp);

        printf("  %-6d  %-26s  %d/%d\n", id_e, nombre, disp, cap);

        if (n_ids_est < 512) ids_est[n_ids_est++] = id_e;
        if (id_e > max_id_est) max_id_est = id_e;
    }

    int id_estacion = -1;
    while (id_estacion == -1) {
        printf("\n");
        int elegido = ui_leer_int("Selecciona una estacion (ID)", 1, max_id_est);
        for (int i = 0; i < n_ids_est; i++) {
            if (ids_est[i] == elegido) { id_estacion = elegido; break; }
        }
        if (id_estacion == -1)
            printf("  Error: ese ID no corresponde a ninguna estacion de la lista.\n");
    }

    // --- PASO 2: Mostrar vehiculos de esa estacion ---
    ui_limpiar();
    printf("\n  --- Vehiculos disponibles en estacion %d ---\n", id_estacion);
    printf("  ................................................\n\n");

    net_enviar(CMD_LIST_VEHICULOS "\n");
    char buf2[32];
    net_recibir_linea(buf2, sizeof(buf2));
    int n_veh = atoi(buf2);

    printf("  %-6s  %-12s  %-10s  %s\n", "ID", "Tipo", "Bateria", "Tarifa/min");
    printf("  ------  ------------  ----------  ----------\n");

    int ids_veh[512];
    int n_ids_veh = 0;
    int max_id_veh = 1;
    char tipos_veh[512] = {0};

    int disponibles = 0;
    for (int i = 0; i < n_veh; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        int id, id_est;
        char tipo[2] = {0}, estado[2] = {0};
        float bateria;
        sscanf(linea, "%d|%1s|%f|%d|%1s", &id, tipo, &bateria, &id_est, estado);

        if (id_est == id_estacion && estado[0] == 'D' && bateria > 15) {
            const char *nombre_tipo = (tipo[0] == 'B') ? "Bicicleta" : "Patinete";
            float tarifa = (tipo[0] == 'B') ? 0.05f : 0.07f;
            printf("  %-6d  %-12s  %-9.1f%%  %.2f EUR\n",
                   id, nombre_tipo, bateria, tarifa);

            if (n_ids_veh < 512) {
                ids_veh[n_ids_veh] = id;
                tipos_veh[n_ids_veh] = tipo[0];
                n_ids_veh++;
            }
            if (id > max_id_veh) max_id_veh = id;
            disponibles++;
        }
    }

    if (disponibles == 0) {
        printf("\n  No hay vehiculos disponibles en esta estacion.\n");
        ui_pausa();

    }

    // --- PASO 3: Elegir vehiculo y confirmar ---
    printf("\n");

    int id_vehiculo = -1;
    char tipo_seleccionado = '\0';
    while (id_vehiculo == -1) {
        int elegido = ui_leer_int("ID del vehiculo a alquilar", 1, max_id_veh);
        for (int i = 0; i < n_ids_veh; i++) {
            if (ids_veh[i] == elegido) {
                id_vehiculo = elegido;
                tipo_seleccionado = tipos_veh[i];
                break;
            }
        }
        if (id_vehiculo == -1)
            printf("  Error: ese ID no corresponde a ningun vehiculo disponible en esta estacion.\n");
    }

    char comando[128];
    snprintf(comando, sizeof(comando), CMD_ALQUILAR " %d|%d|%d",
             g_id_usuario, id_vehiculo, id_estacion);

    char resp[64];
    net_cmd(comando, resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, strlen(RESP_OK)) == 0) {
        int id_alquiler = 0;
        sscanf(resp, "OK %d", &id_alquiler);
        g_alquiler_activo = id_alquiler;
        g_vehiculo_activo = id_vehiculo;
        g_tipo_vehiculo_activo = tipo_seleccionado;
        printf("\n  Alquiler iniciado correctamente.\n");
        printf("  ID de alquiler: %d\n", id_alquiler);
        printf("  Buen viaje!\n");
    } else {
        printf("\n  Error al iniciar el alquiler.\n");
        printf("  Comprueba que el ID del vehiculo sea correcto.\n");
    }
    ui_pausa();
}

// Permite al usuario devolver el vehiculo que tiene alquilado
static void devolver(void) {
    ui_limpiar();
    printf("\n  --- Devolver vehiculo ---\n");
    printf("  ................................................\n\n");

    if (g_alquiler_activo == 0) {
        printf("  No tienes ningun alquiler en curso.\n");
        ui_pausa();
        return;
    }

    printf("  Alquiler activo  : ID %d\n", g_alquiler_activo);
    printf("  Vehiculo en uso  : ID %d\n\n", g_vehiculo_activo);

    net_enviar(CMD_LIST_ESTACIONES "\n");
    char buf_est[32];
    net_recibir_linea(buf_est, sizeof(buf_est));
    int n_est = atoi(buf_est);
    printf("  %-6s  %-26s  %s\n", "ID", "Nombre", "Libres");
    printf("  ------  --------------------------  -------\n");
    for (int i = 0; i < n_est; i++) {
        char linea_est[PROTO_BUFF_SIZE];
        net_recibir_linea(linea_est, sizeof(linea_est));
        Estacion e = Estacion::fromString(linea_est);
        printf("  %-6d  %-26s  %d/%d\n",
               e.id_estacion, e.nombre,
               e.disponibilidad_actual, e.capacidad_max);
    }
    printf("\n");

    int id_estacion = ui_leer_int("ID de la estacion de destino", 1, 99999);

    // Confirmacion antes de devolver
    printf("\n  Confirmar devolucion en estacion %d (s/n): ", id_estacion);
    char conf[8];
    fgets(conf, sizeof(conf), stdin);
    if (conf[0] != 's' && conf[0] != 'S') {
        printf("  Operacion cancelada.\n");
        ui_pausa();
        return;
    }

    char comando[128];
    snprintf(comando, sizeof(comando), CMD_DEVOLVER " %d|%d",
             g_alquiler_activo, id_estacion);

    char resp[64];
    net_cmd(comando, resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, strlen(RESP_OK)) == 0) {
        printf("\n  Vehiculo devuelto correctamente.\n");
        g_alquiler_activo = 0;
        g_vehiculo_activo = 0;
        g_tipo_vehiculo_activo = '\0';
        refrescar_saldo();
    } else {
        printf("\n  Error al devolver el vehiculo. Intentalo de nuevo.\n");
    }
    ui_pausa();
}

// Muestra el historial de alquileres del usuario actual
static void mis_alquileres(void) {
    ui_limpiar();
    printf("\n  --- Mis alquileres ---\n");
    printf("  ................................................\n\n");

    net_enviar(CMD_LIST_ALQUILERES "\n");

    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    printf("  %-6s  %-8s  %-19s  %-19s  %s\n",
           "ID", "Vehic.", "Inicio", "Fin", "Coste");
    printf("  ------  --------  -------------------  -------------------  --------\n");

    int total = 0;
    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        // Parsear: id|id_u|id_v|est_orig|est_dest|f_ini|f_fin|coste
        int id_al, id_u, id_v, est_o, est_d;
        char f_ini[32], f_fin[32];
        float coste;
        sscanf(linea, "%d|%d|%d|%d|%d|%[^|]|%[^|]|%f",
               &id_al, &id_u, &id_v, &est_o, &est_d, f_ini, f_fin, &coste);

        // Filtramos solo los del usuario actual
        if (id_u == g_id_usuario) {
            const char *fin = (strlen(f_fin) == 0 || strcmp(f_fin, "-") == 0)
                              ? "En curso" : f_fin;
            printf("  %-6d  %-8d  %-19s  %-19s  %.2f EUR\n",
                   id_al, id_v, f_ini, fin, coste);
            total++;
        }
    }

    if (total == 0) printf("  No tienes alquileres registrados.\n");
    printf("  ................................................\n");
    printf("  Total: %d alquileres.\n", total);
    ui_pausa();
}

// Comprueba al arrancar si el usuario ya tiene un alquiler activo
static void sincronizar_estado_inicial(void) {
    net_enviar(CMD_LIST_ALQUILERES "\n");
    char buf[32];
    net_recibir_linea(buf, sizeof(buf));
    int n = atoi(buf);

    for (int i = 0; i < n; i++) {
        char linea[PROTO_BUFF_SIZE];
        net_recibir_linea(linea, sizeof(linea));

        int id_al, id_u, id_v, est_o, est_d;
        char f_ini[32] = {0}, f_fin[32] = {0};
        float coste;
        sscanf(linea, "%d|%d|%d|%d|%d|%[^|]|%[^|]|%f",
               &id_al, &id_u, &id_v, &est_o, &est_d, f_ini, f_fin, &coste);

        if (id_u == g_id_usuario &&
            (strlen(f_fin) == 0 || strcmp(f_fin, "-") == 0)) {
            g_alquiler_activo = id_al;
            g_vehiculo_activo = id_v;
            }
    }
}

// Muestra saldo actualizado y estado del vehiculo activo
static void consultar_estado(void) {
    ui_limpiar();
    printf("\n  --- Estado de tu cuenta ---\n");
    printf("  ................................................\n\n");

    refrescar_saldo();
    printf("  Saldo actual     : %.2f EUR\n", g_saldo);

    if (g_alquiler_activo == 0) {
        printf("  Alquiler activo  : Ninguno\n");
        ui_pausa();
        return;
    }

    printf("  Alquiler activo  : ID %d\n", g_alquiler_activo);
    printf("  Vehiculo en uso  : ID %d\n\n", g_vehiculo_activo);

    char cmd[64], resp[256];
    snprintf(cmd, sizeof(cmd), CMD_STAT " %d\n", g_vehiculo_activo);
    net_enviar(cmd);
    net_recibir_linea(resp, sizeof(resp));

    if (strncmp(resp, RESP_OK, 2) == 0) {
        float bateria = 0;
        double minutos = 0, km = 0;
        sscanf(resp + 3, "%f|%lf|%lf", &bateria, &minutos, &km);
        float tarifa = (g_tipo_vehiculo_activo == 'P') ? 0.07f : 0.05f;
        float coste = (float)(minutos * tarifa);
        printf("  Bateria          : %.1f%%\n", bateria);
        printf("  Duracion         : %.0f min\n", minutos);
        printf("  Km estimados     : %.1f km\n", km);
        printf("  Coste acumulado  : %.2f EUR\n", coste);
    }
    ui_pausa();
}

int menu_autenticar(void) {
    char dni[32], clave[64];
    int intentos = 3;

    while (intentos > 0) {
        ui_limpiar();
        menu_banner();
        printf("  Acceso restringido. Identifiquese.\n\n");

        ui_leer_string("DNI", dni, sizeof(dni));
        ui_leer_string("Contrasena", clave, sizeof(clave));

        // Enviamos el login al servidor
        char comando[256];
        snprintf(comando, sizeof(comando), CMD_LOGIN " %s|%s", dni, clave);

        char resp[64];
        net_cmd(comando, resp, sizeof(resp));

        if (strcmp(resp, RESP_OK) == 0) {
            // Login correcto: buscamos los datos del usuario para guardarlos
            net_enviar(CMD_LIST_USUARIOS "\n");
            char buf[32];
            net_recibir_linea(buf, sizeof(buf));
            int n = atoi(buf);

            for (int i = 0; i < n; i++) {
                char linea[PROTO_BUFF_SIZE];
                net_recibir_linea(linea, sizeof(linea));
                int id; char udni[32], unombre[64]; float saldo;
                if (sscanf(linea, "%d|%31[^|]|%63[^|]|%f", &id, udni, unombre, &saldo) == 4) {
                    if (strcmp(udni, dni) == 0) {
                        g_id_usuario = id;
                        strncpy(g_dni,    udni,    sizeof(g_dni)    - 1);
                        strncpy(g_nombre, unombre, sizeof(g_nombre) - 1);
                        g_saldo = saldo;
                    }
                }
            }

            printf("\n  Bienvenido, %s.\n", g_nombre);
            ui_pausa();
            sincronizar_estado_inicial();
            return 1;
        }

        intentos--;
        printf("\n  Credenciales incorrectas. Intentos restantes: %d\n", intentos);
        if (intentos > 0) ui_pausa();
    }

    printf("\n  Demasiados intentos fallidos. El programa se cerrara.\n\n");
    return 0;
}

void menu_principal(void) {
    int opcion;
    do {
        refrescar_saldo();
        ui_limpiar();
        menu_banner();
        printf("  Bienvenido, %s  |  Saldo: %.2f EUR\n", g_nombre, g_saldo);
        if (g_alquiler_activo)
            printf("  Alquiler activo: ID %d\n", g_alquiler_activo);
        printf("\n");
        printf("  ................................................\n");
        printf("    [ 1 ]  Ver estaciones\n");
        printf("    [ 2 ]  Ver vehiculos disponibles\n");
        printf("    [ 3 ]  Alquilar vehiculo\n");
        printf("    [ 4 ]  Devolver vehiculo\n");
        printf("    [ 5 ]  Mis alquileres\n");
        printf("    [ 6 ]  Consultar saldo y estado\n");
        printf("  ................................................\n");
        printf("    [ 0 ]  Cerrar sesion\n");
        printf("\n");

        opcion = ui_leer_int("Seleccione opcion", 0, 6);

        switch (opcion) {
            case 1: ver_estaciones();          break;
            case 2: ver_vehiculos_disponibles(); break;
            case 3: alquilar();                break;
            case 4: devolver();                break;
            case 5: mis_alquileres();          break;
            case 6: consultar_estado();          break;
            case 0:
                printf("\n  ................................................\n");
                printf("  Hasta pronto, %s!\n\n", g_nombre);
                break;
            default:
                printf("  Opcion no valida.\n");
                ui_pausa();
                break;
        }
    } while (opcion != 0);
}

int main(void) {
    printf("\n  Conectando al servidor...\n");

    config_cargar("data/config.conf");
    if (net_conectar(g_config.ip_servidor, g_config.puerto) != 0) {
        printf("  Error: no se pudo conectar al servidor.\n");
        printf("  Asegurate de que KinetiX_Server esta en ejecucion.\n\n");
        return 1;
    }

    printf("  Conexion establecida.\n\n");

    if (menu_autenticar()) {
        menu_principal();

        // Enviamos EXIT al cerrar sesion
        char resp[32];
        net_cmd(CMD_EXIT, resp, sizeof(resp));
    }

#ifdef _WIN32
    closesocket(g_sock);
    WSACleanup();
#else
    close(g_sock);
#endif
    return 0;
}
