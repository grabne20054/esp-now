#ifndef WEBSERVER_C
#define WEBSERVER_C

#include "../include/websocket.h"

esp_err_t send_ws_message(httpd_req_t *req, const char *msg)
{
    httpd_ws_frame_t ws_pkt = {
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)msg,
        .len = strlen(msg)
    };

    return httpd_ws_send_frame(req, &ws_pkt);
}


esp_err_t send_ws_message_async(const char *msg, httpd_handle_t server, int client_fd)
{
    if (server == NULL || client_fd < 0) {
        return ESP_FAIL;
    }

    httpd_ws_frame_t ws_pkt = {
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)msg,
        .len = strlen(msg)
    };

    return httpd_ws_send_frame_async(
        server,
        client_fd,
        &ws_pkt
    );
}


static esp_err_t echo_handler(httpd_req_t *req)
{
    ESP_LOGI(WEBSOCKETTAG, "WebSocket request: %s, method: %d", req->uri, req->method);
    if (req->method == 0) { // HTTP_GET
        
        ws_client_fd = httpd_req_to_sockfd(req);
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(ws_pkt));

    ws_pkt.type = HTTPD_WS_TYPE_TEXT;


    esp_err_t ret = httpd_ws_recv_frame(
        req,
        &ws_pkt,
        0
    );

    if (ret != ESP_OK) {
        ESP_LOGE(WEBSOCKETTAG,
                 "Failed receiving frame");
        return ret;
    }

    uint8_t *buffer = malloc(ws_pkt.len + 1);

    if (!buffer) {
        ESP_LOGE(WEBSOCKETTAG,
                 "Malloc failed");
        return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = buffer;

    ret = httpd_ws_recv_frame(
        req,
        &ws_pkt,
        ws_pkt.len
    );

    if (ret != ESP_OK) {

        free(buffer);

        ESP_LOGE(WEBSOCKETTAG,
                 "Frame receive failed");

        return ret;
    }


    buffer[ws_pkt.len] = 0;

    ESP_LOGI(WEBSOCKETTAG, "Received: %s", buffer);
    e_actions_t action;

    if (strcmp((char *)buffer, "o\n") == 0) {

        action = OPEN;

    } else if (strcmp((char *)buffer, "c\n") == 0) {

        action = CLOSE;

    } else {

        send_ws_message(req, "Err");

        free(buffer);

        return ESP_OK;
    }

    free(buffer);

    queue_t *unq_queue = get_unq_queue();

    if (unq_queue == NULL) {

        ESP_LOGE(WEBSOCKETTAG, "Queue unavailable");

        send_ws_message(req, "Queue Error");

        return ESP_FAIL;
    }

    bool success = add_action_to_queue(
        action,
        REQUEST,
        unq_queue
    );

    if (success) {

        send_ws_message(req, "Action Queued");

    } else {

        send_ws_message(req, "Failed");

    }

    return ESP_OK;
}



static const httpd_uri_t ws = {

    .uri = "/ws",
    .method = HTTP_GET,
    .handler = echo_handler,
    .user_ctx = NULL,
    .is_websocket = true
};

httpd_handle_t start_websocket(void)
{

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(WEBSOCKETTAG, "Starting server on port %d", config.server_port);


    if (httpd_start(&ws_server, &config) != ESP_OK) {

        ESP_LOGE(WEBSOCKETTAG, "Failed starting HTTP server");
        return NULL;
    }

    httpd_register_uri_handler(ws_server, &ws);

    return ws_server;
}

esp_err_t stop_websocket(httpd_handle_t server)
{
    ws_server = NULL;
    ws_client_fd = -1;

    return httpd_stop(server);
}

void ws_task(void *pvParameters)
{
    httpd_handle_t server = start_websocket();

    if (server == NULL) {

        ESP_LOGE(WEBSOCKETTAG, "WebSocket start failed");

        vTaskDelete(NULL);
        return;
    }

    vTaskDelete(NULL);
}

httpd_handle_t get_ws_server(void)
{
    return ws_server;
}

int get_ws_client_fd(void)
{
    return ws_client_fd;
}

void close_ws_connection(httpd_handle_t server, int client_fd)
{
    if (server == NULL || client_fd < 0) {
        ESP_LOGE(WEBSOCKETTAG, "Invalid server or client fd for closing connection");
        return;
    }

    esp_err_t err = httpd_sess_trigger_close(server, client_fd);
    if (err != ESP_OK) {
        ESP_LOGE(WEBSOCKETTAG, "Failed to close WebSocket connection: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(WEBSOCKETTAG, "WebSocket connection closed successfully");
    }
}

#endif