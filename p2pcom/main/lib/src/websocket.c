#ifndef WEBSERVER_C
#define WEBSERVER_C

#include "../include/websocket.h"


static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
};

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
    queue_t * unq_queue = get_unq_queue();

    ESP_LOGI(WEBSOCKETTAG, "Got Request");

    //e_actions_t action;

    char action = 'o'; // to do choose action based on ws conn

    switch (action)
    {
    case 'o':
        action=0;
        break;
    
    default:
        break;
    }

    bool add_succ = add_to_queue(action, unq_queue);

    if (add_succ)
    {
        pthread_mutex_lock(&unq_queue->mutex);

        esp_err_t err = transmit((data_stream_t *) unq_queue->data_stream);
    
    }
    else
    {
        return 0;
    }

    pthread_mutex_unlock(&unq_queue->mutex);

    return ESP_OK;

}

static esp_err_t stop_websocket(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

#endif