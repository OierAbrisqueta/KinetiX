#ifndef KINETIX_PROTOCOLO_H
#define KINETIX_PROTOCOLO_H

/* =============================================================
 *  protocolo.h  —  Contrato de comunicacion cliente/servidor
 *  Incluir en AMBOS proyectos (servidor C y cliente C++)
 * ============================================================= */

/* ── Tamaños de buffer ─────────────────────────────────────── */
#define PROTO_BUFF_SIZE     4096
#define PROTO_MAX_CAMPOS    16

/* ── Separadores ────────────────────────────────────────────── */
#define PROTO_SEP_CAMPO     "|"
#define PROTO_SEP_FILA      "\n"
#define PROTO_SEP_CAMPO_C   '|'
#define PROTO_SEP_FILA_C    '\n'

/* ── Comandos (cliente → servidor) ─────────────────────────── */
#define CMD_LOGIN               "LOGIN"
#define CMD_EXIT                "EXIT"

#define CMD_LIST_ESTACIONES     "LIST_ESTACIONES"
#define CMD_GET_ESTACION        "GET_ESTACION"
#define CMD_ADD_ESTACION        "ADD_ESTACION"
#define CMD_UPDATE_ESTACION     "UPDATE_ESTACION"
#define CMD_DELETE_ESTACION     "DELETE_ESTACION"

#define CMD_LIST_VEHICULOS      "LIST_VEHICULOS"
#define CMD_GET_VEHICULO        "GET_VEHICULO"
#define CMD_ADD_VEHICULO        "ADD_VEHICULO"
#define CMD_UPDATE_VEHICULO     "UPDATE_VEHICULO"
#define CMD_BAJA_VEHICULO       "BAJA_VEHICULO"

#define CMD_LIST_USUARIOS       "LIST_USUARIOS"
#define CMD_GET_USUARIO         "GET_USUARIO"
#define CMD_ADD_USUARIO         "ADD_USUARIO"
#define CMD_UPDATE_USUARIO      "UPDATE_USUARIO"
#define CMD_BAJA_USUARIO        "BAJA_USUARIO"

#define CMD_LIST_ALQUILERES     "LIST_ALQUILERES"
#define CMD_ALQUILAR            "ALQUILAR"
#define CMD_DEVOLVER            "DEVOLVER"

#define CMD_STAT  "STAT"

/* ── Respuestas (servidor → cliente) ────────────────────────── */
#define RESP_OK                 "OK"
#define RESP_ERROR              "ERROR"
#define RESP_BYE                "BYE"
#define RESP_NOT_FOUND          "NOT_FOUND"

/*
 * =============================================================
 *  ESPECIFICACION DEL PROTOCOLO
 *  Formato general: "<COMANDO> [arg]\n"
 *  El servidor siempre termina su respuesta con '\n'
 * =============================================================
 *
 *  LOGIN <dni>|<contrasena>
 *      OK  /  ERROR
 *
 *  EXIT
 *      BYE
 *
 *  LIST_ESTACIONES
 *      <n>\n  +  n x "<id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>\n"
 *
 *  GET_ESTACION <id>
 *      OK\n<id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>\n  /  NOT_FOUND
 *
 *  ADD_ESTACION <nombre>|<dir>|<cx>|<cy>|<cap>|<disp>
 *      OK <id_generado>  /  ERROR
 *
 *  UPDATE_ESTACION <id>|<nombre>|<dir>|<cx>|<cy>|<cap>|<disp>
 *      OK  /  ERROR
 *
 *  DELETE_ESTACION <id>
 *      OK  /  ERROR
 *
 *  LIST_VEHICULOS
 *      <n>\n  +  n x "<id>|<tipo>|<bateria>|<id_estacion>|<estado>\n"
 *
 *  GET_VEHICULO <id>
 *      OK\n<id>|<tipo>|<bateria>|<id_estacion>|<estado>\n  /  NOT_FOUND
 *
 *  ADD_VEHICULO <tipo>|<bateria>|<id_estacion>|<estado>
 *      OK <id_generado>  /  ERROR
 *
 *  UPDATE_VEHICULO <id>|<tipo>|<bateria>|<id_estacion>|<estado>
 *      OK  /  ERROR
 *
 *  BAJA_VEHICULO <id>
 *      OK  /  ERROR
 *
 *  LIST_USUARIOS
 *      <n>\n  +  n x "<id>|<dni>|<nombre>|<saldo>\n"  (sin contrasena)
 *
 *  GET_USUARIO <id>
 *      OK\n<id>|<dni>|<nombre>|<saldo>\n  /  NOT_FOUND
 *
 *  ADD_USUARIO <dni>|<nombre>|<saldo>|<contrasena>
 *      OK <id_generado>  /  ERROR
 *
 *  UPDATE_USUARIO <id>|<dni>|<nombre>|<saldo>
 *      OK  /  ERROR
 *
 *  BAJA_USUARIO <id>
 *      OK  /  ERROR
 *
 *  LIST_ALQUILERES
 *      <n>\n  +  n x "<id>|<id_u>|<id_v>|<est_orig>|<est_dest>|<f_ini>|<f_fin>|<coste>\n"
 *
 *  ALQUILAR <id_usuario>|<id_vehiculo>|<id_estacion_origen>
 *      OK <id_alquiler>  /  ERROR
 *
 *  DEVOLVER <id_alquiler>|<id_estacion_destino>
 *      OK  /  ERROR
 * ============================================================= */

#endif /* KINETIX_PROTOCOLO_H */