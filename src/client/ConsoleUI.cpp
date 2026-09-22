#include "ConsoleUI.hpp"

namespace {
    void limpiar_buffer(void) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }
}

namespace ConsoleUI {
    void limpiar() {
        fflush(stdout);
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
    }

    void pausa() {
        printf("\n  ................................................\n");
        printf("\n  Pulse Enter para continuar...");
        fflush(stdout);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    void menu_banner() {
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
}