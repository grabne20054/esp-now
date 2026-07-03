#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>

#include <esp_http_server.h>

#include "send.h"
static const char *WEBSOCKETTAG = "ws_socket";


esp_err_t echo_handler(httpd_req_t *req);

static void send_response(httpd_req_t *req, char *resp);

httpd_handle_t start_websocket(void);

esp_err_t stop_websocket(httpd_handle_t server);

void ws_task(void *pvParameters);

bool check_subpath(const char *subpath, const char *expected);


#endif
