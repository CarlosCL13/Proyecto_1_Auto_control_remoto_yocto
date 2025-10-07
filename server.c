#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <microhttpd.h>
#include "car_control.h"
#include <my_gpio_lib_rpi4.h>
#include "auth_utils.h"

#define PORT 8888

// Extrae el valor de una clave string de un JSON plano (no anidado)
// Ejemplo: extract_json_string(data, "direction", direction, sizeof(direction));
int extract_json_string(const char* json, const char* key, char* out, size_t outlen) {
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

// Extrae el valor de una clave int de un JSON plano (no anidado)
int extract_json_int(const char* json, const char* key, int* out) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    const char* p = strstr(json, pattern);
    if (!p) return 0;
    p += strlen(pattern);
    while (*p && !isdigit(*p) && *p != '-') p++;
    if (!*p) return 0;
    *out = atoi(p);
    return 1;
}

// Buffer para almacenar datos POST
struct PostData {
    char* data;
    size_t size;
};

static int send_json_response(struct MHD_Connection *connection, const char *json, int status_code) {
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(json), (void *)json, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url,
                                const char *method, const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls)
{
    static int dummy;
    if (0 != strcmp(method, "POST") && 0 != strcmp(method, "GET"))
        return MHD_NO;

    // Manejo de datos POST
    if (strcmp(method, "POST") == 0) {
        if (*con_cls == NULL) {
            struct PostData *post_data = calloc(1, sizeof(struct PostData));
            *con_cls = (void*)post_data;
            return MHD_YES;
        }
        struct PostData *post_data = *con_cls;
        if (*upload_data_size != 0) {
            size_t old_size = post_data->size;
            post_data->size += *upload_data_size;
            post_data->data = realloc(post_data->data, post_data->size + 1);
            memcpy(post_data->data + old_size, upload_data, *upload_data_size);
            post_data->data[post_data->size] = '\0';
            *upload_data_size = 0;
            return MHD_YES;
        }

        // --- ENDPOINTS POST ---
        if (strcmp(url, "/api/vehicle/direction") == 0) {
            char direction[32] = "";
            extract_json_string(post_data->data, "direction", direction, sizeof(direction));
            car_set_direction(direction);
            const char* resp = "{\"result\":\"ok\"}";
            int ret = send_json_response(connection, resp, MHD_HTTP_OK);
            free(post_data->data); free(post_data);
            *con_cls = NULL;
            return (ret == MHD_YES) ? MHD_YES : MHD_NO;
        }
        if (strcmp(url, "/api/vehicle/speed") == 0) {
            int speed = 0;
            extract_json_int(post_data->data, "speed", &speed);
            car_set_speed(speed);
            const char* resp = "{\"result\":\"ok\"}";
            int ret = send_json_response(connection, resp, MHD_HTTP_OK);
            free(post_data->data); free(post_data);
            *con_cls = NULL;
            return (ret == MHD_YES) ? MHD_YES : MHD_NO;
        }
        if (strcmp(url, "/api/vehicle/light") == 0) {
            char type[32] = "";
            int state = 0;
            extract_json_string(post_data->data, "type", type, sizeof(type));
            char* statePos = strstr(post_data->data, "\"state\":");
            if (statePos) {
                statePos += strlen("\"state\":");
                while (*statePos == ' ' || *statePos == '\t') statePos++;
                if (strncmp(statePos, "true", 4) == 0 || *statePos == '1') {
                    state = 1;
                }
            }
            car_toggle_light(type, state);
            const char* resp = "{\"result\":\"ok\"}";
            int ret = send_json_response(connection, resp, MHD_HTTP_OK);
            free(post_data->data); free(post_data);
            *con_cls = NULL;
            return (ret == MHD_YES) ? MHD_YES : MHD_NO;
        }
        if (strcmp(url, "/api/login") == 0) {
            char user[64] = "", pass[64] = "";
            extract_json_string(post_data->data, "username", user, sizeof(user));
            extract_json_string(post_data->data, "password", pass, sizeof(pass));
            if (check_user_password(user, pass)) {
                const char* resp = "{\"token\":\"dummy-token-123\"}";
                int ret = send_json_response(connection, resp, MHD_HTTP_OK);
                free(post_data->data); free(post_data);
                *con_cls = NULL;
                return (ret == MHD_YES) ? MHD_YES : MHD_NO;
            } else {
                const char* resp = "{\"error\":\"Invalid credentials\"}";
                int ret = send_json_response(connection, resp, MHD_HTTP_UNAUTHORIZED);
                free(post_data->data); free(post_data);
                *con_cls = NULL;
                return (ret == MHD_YES) ? MHD_YES : MHD_NO;
            }
        }
        // Si no es endpoint conocido
        const char* resp = "{\"error\":\"Not found\"}";
        int ret = send_json_response(connection, resp, MHD_HTTP_NOT_FOUND);
        free(post_data->data); free(post_data);
        *con_cls = NULL;
    return (ret == MHD_YES) ? MHD_YES : MHD_NO;
    }

    // --- ENDPOINTS GET ---
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/api/vehicle/status") == 0) {
            char status[256];
            car_get_status(status, sizeof(status));
            //printf("DEBUG JSON: %s\n", status);
            fflush(stdout);
            int ret = send_json_response(connection, status, MHD_HTTP_OK);
            return (ret == MHD_YES) ? MHD_YES : MHD_NO;
        }
        // Servir archivos estáticos y frontend
        char filepath[256];
        const char *content_type = "text/plain";
        if (strcmp(url, "/") == 0) {
            strcpy(filepath, "index.html");
            content_type = "text/html";
        } else {
            snprintf(filepath, sizeof(filepath), "%s", url+1);
            const char *ext = strrchr(filepath, '.');
            if (ext) {
                if (strcmp(ext, ".html") == 0) content_type = "text/html";
                else if (strcmp(ext, ".css") == 0) content_type = "text/css";
                else if (strcmp(ext, ".js") == 0) content_type = "application/javascript";
                else if (strcmp(ext, ".json") == 0) content_type = "application/json";
                else if (strcmp(ext, ".png") == 0) content_type = "image/png";
                else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) content_type = "image/jpeg";
                else if (strcmp(ext, ".ico") == 0) content_type = "image/x-icon";
            }
        }
        FILE *file = fopen(filepath, "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long filesize = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *filebuf = malloc(filesize);
            if (filebuf) {
                fread(filebuf, 1, filesize, file);
                struct MHD_Response *response = MHD_create_response_from_buffer(filesize, (void *)filebuf, MHD_RESPMEM_MUST_FREE);
                MHD_add_response_header(response, "Content-Type", content_type);
                int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                MHD_destroy_response(response);
                fclose(file);
            return (ret == MHD_YES) ? MHD_YES : MHD_NO;
            }
            fclose(file);
        }
    }
    // Ruta no encontrada
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen("Not Found"), (void *)"Not Found", MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}


int main()
{
    // Inicializa la biblioteca GPIO
    if (gpioInit() < 0) {
        fprintf(stderr, "No se pudo inicializar GPIO. Ejecuta como root o revisa hardware.\n");
        return 1;
    }

    struct MHD_Daemon *daemon;
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        gpioCleanup();
        return 1;
    }
    printf("Servidor web corriendo en http://localhost:%d\n", PORT);
    getchar();
    MHD_stop_daemon(daemon);

    // Libera recursos GPIO antes de salir
    gpioCleanup();
    return 0;
}
