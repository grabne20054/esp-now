#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>

#include <esp_http_server.h>
#include <esp_system.h>
#include <http_parser.h>

#include "send.h"
static const char *WEBSOCKETTAG = "ws_socket";

static esp_err_t echo_handler(httpd_req_t *req);

esp_err_t send_ws_message(httpd_req_t *req, const char *msg);

esp_err_t send_ws_message_async(const char *msg, httpd_handle_t server, int client_fd);

httpd_handle_t start_websocket(void);

esp_err_t stop_websocket(httpd_handle_t server);

void ws_task(void *pvParameters);

bool check_subpath(const char *subpath, const char *expected);

httpd_handle_t get_ws_server(void);

int get_ws_client_fd(void);


#endif
