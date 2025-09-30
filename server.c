#include <microhttpd.h>
#include <stdio.h>
#include <string.h>

#define PORT 8888

static int answer_to_connection(void *cls, struct MHD_Connection *connection, const char *url,
                                const char *method, const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls)
{
    const char *response_str = NULL;
    struct MHD_Response *response;
    int ret = 0;

    // Comandos GET
    if (strcmp(method, "GET") == 0) {
        if (strcmp(url, "/status") == 0) {
            response_str = "OK";
        } else if (strcmp(url, "/adelante") == 0) {
            response_str = "Adelante";
        } else if (strcmp(url, "/atras") == 0) {
            response_str = "Atras";
        } else if (strcmp(url, "/izquierda") == 0) {
            response_str = "Izquierda";
        } else if (strcmp(url, "/derecha") == 0) {
            response_str = "Derecha";
        }
        if (response_str) {
            response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        }
    }

    // Comando POST para luces
    if (strcmp(url, "/luces") == 0 && strcmp(method, "POST") == 0) {
        response_str = "Luces comando recibido";
        response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Comando POST para velocidad
    if (strcmp(url, "/velocidad") == 0 && strcmp(method, "POST") == 0) {
        response_str = "Velocidad comando recibido";
        response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Ruta no encontrada
    response = MHD_create_response_from_buffer(strlen("Not Found"), (void *)"Not Found", MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

int main()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    printf("Servidor web corriendo en http://localhost:%d/status\n", PORT);
    getchar(); // Espera hasta que se presione Enter
    MHD_stop_daemon(daemon);
    return 0;
}
