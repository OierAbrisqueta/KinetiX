#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif
#include "gestor_config.h"
#include "gestor_log.h"
#include "gestor_bd.h"
#include "server_socket.h"

/* =============================================================
 *  main_server.c  —  Punto de entrada del servidor remoto
 *
 *  Uso:  KinetiX_RemoteServer [ruta_config]
 *  Por defecto usa data/config.conf
 * ============================================================= */

#ifdef _WIN32
static unsigned __stdcall atender_cliente_thread_win(void *arg) {
    kinetix_socket_t cliente = *((kinetix_socket_t *)arg);
    free(arg);
    servidor_atender_cliente(cliente);
    LOG_I("Cliente desconectado. Esperando nuevo cliente...");
    printf("\n  >> Cliente desconectado. Esperando...\n\n");
    return 0;
}
#else
static void *atender_cliente_thread(void *arg) {
    kinetix_socket_t cliente = *((kinetix_socket_t *)arg);
    free(arg);
    servidor_atender_cliente(cliente);
    LOG_I("Cliente desconectado. Esperando nuevo cliente...");
    printf("\n  >> Cliente desconectado. Esperando...\n\n");
    return NULL;
}
#endif

int main(int argc, char *argv[]) {

    /* ── 1. Cargar configuracion ─────────────────────────── */
    const char *ruta_conf = (argc > 1) ? argv[1] : CONFIG_RUTA_DEFAULT;
    if (config_cargar(ruta_conf) != 0)
        printf("[ARRANQUE] Continuando con valores por defecto.\n");

    /* ── 2. Inicializar log ──────────────────────────────── */
    if (log_inicializar(g_config.log_ruta) != 0)
        printf("[ARRANQUE] Aviso: log no disponible.\n");

    LOG_I("=== KinetiX Servidor Remoto arrancando ===");

    /* ── 3. Conectar base de datos ───────────────────────── */
    if (conectar_bd(g_config.db_ruta) != 0) {
        LOG_E("No se pudo conectar con la BD. Abortando.");
        log_cerrar();
        return EXIT_FAILURE;
    }
    LOG_I("Conexion con la base de datos establecida.");

    /* ── 4. Iniciar socket de escucha ────────────────────── */
    if (servidor_iniciar(g_config.puerto) != 0) {
        LOG_E("No se pudo iniciar el servidor de sockets. Abortando.");
        cerrar_bd();
        log_cerrar();
        return EXIT_FAILURE;
    }

    /* ── 5. Bucle principal ──────────────────────────────── */
    printf("  Servidor listo. Pulsa Ctrl+C para detener.\n\n");

    while (1) {
        kinetix_socket_t cliente = servidor_aceptar_cliente();
        if (cliente == KINETIX_SOCKET_INVALIDO) {
            LOG_A("Error aceptando cliente. Reintentando...");
            continue;
        }
#ifdef _WIN32
        kinetix_socket_t *cliente_ptr = malloc(sizeof(*cliente_ptr));
        if (!cliente_ptr) {
            LOG_E("No se pudo reservar memoria para el cliente.");
            closesocket(cliente);
            continue;
        }
        *cliente_ptr = cliente;

        uintptr_t hilo = _beginthreadex(NULL, 0, atender_cliente_thread_win, cliente_ptr, 0, NULL);
        if (hilo == 0) {
            LOG_E("No se pudo crear el hilo para el cliente.");
            closesocket(cliente);
            free(cliente_ptr);
            continue;
        }
        CloseHandle((HANDLE)hilo);
#else
        kinetix_socket_t *cliente_ptr = malloc(sizeof(*cliente_ptr));
        if (!cliente_ptr) {
            LOG_E("No se pudo reservar memoria para el cliente.");
            closesocket(cliente);
            continue;
        }
        *cliente_ptr = cliente;

        pthread_t hilo;
        int err = pthread_create(&hilo, NULL, atender_cliente_thread, cliente_ptr);
        if (err != 0) {
            LOG_E("No se pudo crear el hilo para el cliente.");
            closesocket(cliente);
            free(cliente_ptr);
            continue;
        }
        pthread_detach(hilo);
#endif
    }

    /* ── 6. Limpieza ─────────────────────────────────────── */
    servidor_cerrar();
    cerrar_bd();
    log_cerrar();
    return EXIT_SUCCESS;
}