#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>

#include <esp_http_server.h>
static const char *WEBSOCKETTAG = "ws_socket";


static esp_err_t echo_handler(httpd_req_t *req);

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
};

static httpd_handle_t start_websocket(void);

static esp_err_t stop_websocket(httpd_handle_t server);


#endif
