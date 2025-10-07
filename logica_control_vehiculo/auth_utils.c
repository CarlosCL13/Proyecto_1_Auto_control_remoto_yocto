#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "auth_utils.h"
#include <openssl/sha.h>

// Extrae el valor de una clave string de un JSON plano (no anidado)
static int extract_json_string(const char* json, const char* key, char* out, size_t outlen) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    const char* p = strstr(json, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    size_t i = 0;
    while (*p && *p != '"' && i < outlen-1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return i > 0;
}


// Calcula el hash SHA-256 de una cadena y lo devuelve como string hexadecimal
void sha256_hex(const char* input, char* output_hex) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)input, strlen(input), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    }
    output_hex[64] = '\0';
}

// Verifica usuario y password (hash) contra users.json
int check_user_password(const char* username, const char* password) {
    printf("[AUTH DEBUG] username recibido: '%s'\n", username);
    printf("[AUTH DEBUG] password recibido: '%s'\n", password);
    FILE* f = fopen("users.json", "r");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);

    char hash[65];
    sha256_hex(password, hash);
    printf("[AUTH DEBUG] hash calculado: %s\n", hash);

    char* p = buf;
    int found = 0;
    while ((p = strstr(p, "\"username\""))) {
        char uname[64] = "", phash[65] = "";
        // Busca el siguiente "username":"valor"
        char* uname_start = strchr(p, ':');
        if (uname_start) {
            uname_start++;
            while (*uname_start == ' ' || *uname_start == '"') uname_start++;
            char* uname_end = strchr(uname_start, '"');
            if (uname_end && uname_end > uname_start) {
                size_t len = uname_end - uname_start;
                if (len < sizeof(uname)) {
                    strncpy(uname, uname_start, len);
                    uname[len] = 0;
                }
            }
        }
        // Busca el siguiente "password_hash":"valor"
        char* hash_pos = strstr(p, "\"password_hash\"");
        if (hash_pos) {
            char* hash_start = strchr(hash_pos, ':');
            if (hash_start) {
                hash_start++;
                while (*hash_start == ' ' || *hash_start == '"') hash_start++;
                char* hash_end = strchr(hash_start, '"');
                if (hash_end && hash_end > hash_start) {
                    size_t len = hash_end - hash_start;
                    if (len < sizeof(phash)) {
                        strncpy(phash, hash_start, len);
                        phash[len] = 0;
                    }
                }
            }
        }
        printf("[AUTH DEBUG] usuario en json: '%s', hash en json: %s\n", uname, phash);
        if (strcmp(uname, username) == 0 && strcmp(phash, hash) == 0) {
            found = 1;
            break;
        }
        p++;
    }
    free(buf);
    return found;
}
