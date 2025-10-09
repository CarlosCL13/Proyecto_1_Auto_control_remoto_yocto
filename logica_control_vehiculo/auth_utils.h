#ifndef AUTH_UTILS_H
#define AUTH_UTILS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Calcula el hash SHA-256 de una cadena (hex string)
void sha256_hex(const char* input, char* output_hex);

// Verifica usuario y password (hash) contra users.json
int check_user_password(const char* username, const char* password);

#endif // AUTH_UTILS_H
