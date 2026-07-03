#ifndef WEBSERVER_C
#define WEBSERVER_C

#include "../include/websocket.h"


static const httpd_uri_t ws = {
        .uri        = "/ws/*",
        .method     = HTTP_GET,
        .handler    = echo_handler,
        .user_ctx   = NULL,
};

httpd_handle_t start_websocket(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

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

static void send_response(httpd_req_t *req, char *resp)
{
    ESP_LOGI(WEBSOCKETTAG, "Error send res");
    httpd_resp_send(req, resp, sizeof(resp) );
}


esp_err_t echo_handler(httpd_req_t *req)
{
    bool err = false;
    ESP_LOGI(WEBSOCKETTAG, "Websocket Handler called with uri: %s", req->uri);

    const char *subpath = req->uri + strlen("/ws");

    if (*subpath == '/')
    {    
        subpath++;
    }

    e_actions_t * action = malloc(sizeof(e_actions_t));

    ESP_LOGI(WEBSOCKETTAG, "subpath[1]: %s\n", subpath);

    
    switch (*subpath)
    {
    case 'o':
        if (!check_subpath(subpath, "o")) {
            char * res = "Err";
            send_response(req, res);
            err = true;
            break;
        }
        *action = OPEN;
        break;
    case 'c':
        if (!check_subpath(subpath, "c")) {
            char * res = "Err";
            send_response(req, res);
            err = true;
            break;
        }
        *action = CLOSE;
        break;
    case 's':
        if (!check_subpath(subpath, "s")) {
            char * res = "Err";
            send_response(req, res);
            err = true;
            break;
        }
        *action = GETSTATUS;
        break;
    case 'p':
        if (!check_subpath(subpath, "p")) {
            char * res = "Err";
            send_response(req, res);
            err = true;
            break;
        }
        *action = GETPOSITION;
        break;
    default:
        char * res = "Err";
        send_response(req, res);
        err = true;
        break;
    }

    if (err)
    {
        return ESP_FAIL;
    }
    
    queue_t * unq_queue = get_unq_queue();
    if (unq_queue == NULL)
    {
        ESP_LOGI(WEBSOCKETTAG, "Queue DEAD");
    }
    
    ESP_LOGI(WEBSOCKETTAG, "Got Request");

    bool add_succ = add_to_queue(*action, unq_queue);
    ESP_LOGI(WEBSOCKETTAG, "Adding Queue: %d", add_succ);

    if (add_succ)
    {
        return ESP_OK;
    
    }
    else
    {
        ESP_LOGI(WEBSOCKETTAG, "Failed to add to queue");
        return ESP_FAIL;
    }

}

esp_err_t stop_websocket(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void ws_task(void *pvParameters)
{
    httpd_handle_t server = start_websocket();
    if (server == NULL) {
        ESP_LOGE(WEBSOCKETTAG, "Failed to start websocket");
        vTaskDelete(NULL);
        return;
    }

    vTaskDelete(NULL);
}

bool check_subpath(const char *subpath, const char *expected)
{
    return strcmp(subpath, expected) == 0;
}

#endif