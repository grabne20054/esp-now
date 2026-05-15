#ifndef WEBSERVER_C
#define WEBSERVER_C

#include "../include/websocket.h"

static httpd_handle_t start_websocket(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(WEBSOCKETTAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(WEBSOCKETTAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(WEBSOCKETTAG, "Error starting server!");
    return NULL;
}

static void send_response(httpd_req_t *req)
{
    const char * resp = "echo";
    httpd_resp_send(req, resp, sizeof(resp) );
}


static esp_err_t echo_handler(httpd_req_t *req)
{

    ESP_LOGI(WEBSOCKETTAG, "Got Request");
    send_response(req);

    return ESP_OK;

}

static esp_err_t stop_websocket(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}


#endif