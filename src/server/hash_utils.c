#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <wincrypt.h>
#include "hash_utils.h"

/* =============================================================
 *  hash_utils.c  —  SHA-256 via WinCrypt
 * ============================================================= */

int sha256_hex(const char *input, char out[HASH_SHA256_HEX_LEN]) {
    HCRYPTPROV  hProv  = 0;
    HCRYPTHASH  hHash  = 0;
    BYTE        digest[32];
    DWORD       digest_len = sizeof(digest);
    int         ok = -1;

    if (!CryptAcquireContext(&hProv, NULL, NULL,
                             PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        goto fin;

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash))
        goto fin;

    if (!CryptHashData(hHash, (const BYTE *)input,
                       (DWORD)strlen(input), 0))
        goto fin;

    if (!CryptGetHashParam(hHash, HP_HASHVAL, digest, &digest_len, 0))
        goto fin;

    for (DWORD i = 0; i < digest_len; i++)
        sprintf(out + i * 2, "%02x", (unsigned int)digest[i]);
    out[64] = '\0';
    ok = 0;

    fin:
        if (hHash)  CryptDestroyHash(hHash);
    if (hProv)  CryptReleaseContext(hProv, 0);
    return ok;
}

int verificar_clave(const char *clave_plana, const char *hash_almacenado) {
    char hash_calculado[HASH_SHA256_HEX_LEN];
    if (sha256_hex(clave_plana, hash_calculado) != 0)
        return 0;
    return (strcmp(hash_calculado, hash_almacenado) == 0);
}