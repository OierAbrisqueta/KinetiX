//
// Created by jon.i on 26/03/2026.
//

#ifndef KINETIX_MENU_H
#define KINETIX_MENU_H

void menu_banner(void);
int  menu_autenticar(void);
void menu_principal(void);
void menu_estaciones(void);
void menu_vehiculos(void);
void menu_usuarios(void);
void menu_historico(void);
void menu_configuracion(void);
void menu_informes(void);

void  ui_limpiar(void);
void  ui_pausa(void);
void  ui_separador(void);
int   ui_leer_int(const char *prompt, int min, int max);
void  ui_leer_string(const char *prompt, char *buf, int max_len);
float ui_leer_float(const char *prompt, float min, float max);

#endif