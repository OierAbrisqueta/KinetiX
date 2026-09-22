#ifndef KINETIX_CONSOLEUI_HPP
#define KINETIX_CONSOLEUI_HPP

#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace ConsoleUI {
    void limpiar();
    void pausa();
    void menu_banner();

    int ui_leer_int(const char *msg, int min, int max);
    void ui_leer_string(const char *msg, char *buf, int max);
}

#endif //KINETIX_CONSOLEUI_HPP