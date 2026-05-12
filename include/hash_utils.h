#ifndef KINETIX_HASH_UTILS_H
#define KINETIX_HASH_UTILS_H

/* =============================================================
 *  hash_utils.h  —  Hashing de contraseñas con SHA-256 portable
 * ============================================================= */

/* Longitud de un hash SHA-256 en hex: 64 caracteres + '\0' */
#define HASH_SHA256_HEX_LEN 65

/*
 * sha256_hex()
 *   Calcula el SHA-256 de 'input' y escribe el resultado
 *   como string hexadecimal en 'out' (debe tener >= 65 bytes).
 *   Devuelve 0 en éxito, -1 en error.
 */
int sha256_hex(const char *input, char out[HASH_SHA256_HEX_LEN]);

/*
 * verificar_clave()
 *   Compara 'clave_plana' con 'hash_almacenado' (hex SHA-256).
 *   Devuelve 1 si coinciden, 0 si no.
 */
int verificar_clave(const char *clave_plana, const char *hash_almacenado);

#endif /* KINETIX_HASH_UTILS_H */