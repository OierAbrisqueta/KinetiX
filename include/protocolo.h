//
// Created by jon.i on 07/05/2026.
//

#ifndef KINETIX_PROTOCOLO_H
#define KINETIX_PROTOCOLO_H

#ifndef KINETIX_PROTOCOLO_H
#define KINETIX_PROTOCOLO_H

/* =============================================================
 *  protocolo.h  —  Contrato de comunicacion cliente/servidor
 *  Incluir en AMBOS proyectos (servidor C y cliente C++)
 * ============================================================= */

/* ── Tamaños de buffer ─────────────────────────────────────── */
#define PROTO_BUFF_SIZE     4096
#define PROTO_MAX_CAMPOS    16

/* ── Separadores de serialización ──────────────────────────── */
#define PROTO_SEP_CAMPO     "|"
#define PROTO_SEP_FILA      "\n"
#define PROTO_SEP_CAMPO_C   '|'
#define PROTO_SEP_FILA_C    '\n'

/* ── Comandos (cliente → servidor) ─────────────────────────── */

/* Sesión */
#define CMD_LOGIN               "LOGIN"
#define CMD_EXIT                "EXIT"

/* Estaciones */
#define CMD_LIST_ESTACIONES     "LIST_ESTACIONES"
#define CMD_GET_ESTACION        "GET_ESTACION"
#define CMD_ADD_ESTACION        "ADD_ESTACION"
#define CMD_UPDATE_ESTACION     "UPDATE_ESTACION"
#define CMD_DELETE_ESTACION     "DELETE_ESTACION"

/* Vehículos */
#define CMD_LIST_VEHICULOS      "LIST_VEHICULOS"
#define CMD_GET_VEHICULO        "GET_VEHICULO"
#define CMD_ADD_VEHICULO        "ADD_VEHICULO"
#define CMD_UPDATE_VEHICULO     "UPDATE_VEHICULO"
#define CMD_BAJA_VEHICULO       "BAJA_VEHICULO"

/* Usuarios */
#define CMD_LIST_USUARIOS       "LIST_USUARIOS"
#define CMD_GET_USUARIO         "GET_USUARIO"
#define CMD_ADD_USUARIO         "ADD_USUARIO"
#define CMD_UPDATE_USUARIO      "UPDATE_USUARIO"
#define CMD_BAJA_USUARIO        "BAJA_USUARIO"

/* Alquileres */
#define CMD_LIST_ALQUILERES     "LIST_ALQUILERES"
#define CMD_ALQUILAR            "ALQUILAR"
#define CMD_DEVOLVER            "DEVOLVER"

/* ── Respuestas (servidor → cliente) ────────────────────────── */
#define RESP_OK                 "OK"
#define RESP_ERROR              "ERROR"
#define RESP_BYE                "BYE"
#define RESP_NOT_FOUND          "NOT_FOUND"

/* =============================================================
 *  ESPECIFICACION DEL PROTOCOLO
 *  Formato general: "<COMANDO> [arg]\n"
 *  El servidor siempre termina su respuesta con '\n'
 * =============================================================
 *
 *  LOGIN <dni>|<contrasena>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  EXIT
 *      → "BYE\n"
 *
 *  -- ESTACIONES --
 *
 *  LIST_ESTACIONES
 *      → "<n>\n"
 *        "<id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>\n"  (repetido n veces)
 *
 *  GET_ESTACION <id>
 *      → "OK\n<id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>\n"
 *      → "NOT_FOUND\n"
 *
 *  ADD_ESTACION <nombre>|<dir>|<cx>|<cy>|<cap>|<disp>
 *      → "OK <id_generado>\n"
 *      → "ERROR\n"
 *
 *  UPDATE_ESTACION <id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  DELETE_ESTACION <id>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  -- VEHICULOS --
 *
 *  LIST_VEHICULOS
 *      → "<n>\n"
 *        "<id>|<tipo>|<bateria>|<id_estacion>|<estado>\n"  (repetido n veces)
 *
 *  GET_VEHICULO <id>
 *      → "OK\n<id>|<tipo>|<bateria>|<id_estacion>|<estado>\n"
 *      → "NOT_FOUND\n"
 *
 *  ADD_VEHICULO <tipo>|<bateria>|<id_estacion>|<estado>
 *      → "OK <id_generado>\n"
 *      → "ERROR\n"
 *
 *  UPDATE_VEHICULO <id>|<tipo>|<bateria>|<id_estacion>|<estado>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  BAJA_VEHICULO <id>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  -- USUARIOS --
 *
 *  LIST_USUARIOS
 *      → "<n>\n"
 *        "<id>|<dni>|<nombre>|<saldo>\n"  (repetido n veces, SIN contrasena)
 *
 *  GET_USUARIO <id>
 *      → "OK\n<id>|<dni>|<nombre>|<saldo>\n"
 *      → "NOT_FOUND\n"
 *
 *  ADD_USUARIO <dni>|<nombre>|<saldo>|<contrasena>
 *      → "OK <id_generado>\n"
 *      → "ERROR\n"
 *
 *  UPDATE_USUARIO <id>|<dni>|<nombre>|<saldo>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  BAJA_USUARIO <id>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 *  -- ALQUILERES --
 *
 *  LIST_ALQUILERES
 *      → "<n>\n"
 *        "<id>|<id_usuario>|<id_vehiculo>|<id_est_orig>|<id_est_dest>|<f_inicio>|<f_fin>|<coste>\n"
 *
 *  ALQUILAR <id_usuario>|<id_vehiculo>|<id_estacion_origen>
 *      → "OK <id_alquiler>\n"
 *      → "ERROR\n"
 *
 *  DEVOLVER <id_alquiler>|<id_estacion_destino>
 *      → "OK\n"
 *      → "ERROR\n"
 *
 * ============================================================= */

#endif /* KINETIX_PROTOCOLO_H */

#endif //KINETIX_PROTOCOLO_H