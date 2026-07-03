#ifndef INIT_WIFI_CPP
#define INIT_WIFI_CPP

#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#include "../include/init_wifi.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static const char *TAG_WIFI = "init_wifi";

static int s_retry_num = 0;

#if SWITCH == 1
uint8_t last_good_channel = 1; // default to channel 1
#endif

static EventGroupHandle_t s_wifi_event_group;

esp_err_t init()
{
   esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    if (SWITCH==1)
    {
       ESP_ERROR_CHECK(esp_wifi_start());
    }

    ESP_LOGI(TAG_WIFI, "WiFi initialized successfully");

    return ESP_OK;
}

esp_err_t deinit()
{
    esp_wifi_stop();
    esp_wifi_deinit();
    return ESP_OK;
}

esp_err_t set_mode()
{
    ESP_LOGI(TAG_WIFI, "Setting WiFi mode to %d", WIFI_MODE_STA);
    return esp_wifi_set_mode(WIFI_MODE_STA);
}

esp_err_t get_mac(uint8_t mac[6])
{
    return esp_wifi_get_mac(ESPNOW_WIFI_IF, mac);
}


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG_WIFI,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG_WIFI, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool set_up_tcpip_stack(wifi_config_t config)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s",
                 config.sta.ssid, config.sta.password);
                 return true;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s",
                 config.sta.ssid, config.sta.password);
                 return false;
    } else {
        ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
        return true;
    }
}

static void set_channel(uint8_t channel)
{
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    ESP_LOGI(TAG_WIFI, "WiFi channel set to %d", channel);
}

#if SWITCH == 1
bool hopping_channel()
{

    if (channel_hopping == NULL)
    {
        ESP_LOGE(TAG_WIFI, "Channel hopping event group is NULL");
        return false;
    }

    ESP_LOGI(TAG_WIFI, "Starting channel hopping to find peer...");
    bool found = false;

    for (uint8_t channel = 1; channel <= 13; channel++) {
        set_channel(channel);

        peer.channel = channel;

        vTaskDelay(pdMS_TO_TICKS(1000)); // wait for a while on the new channel

        // send a test frame to check if the peer is on this channel
        data_stream_t *test_stream = prepare_data_stream(0); // action 0 for testing

        test_stream->action = HOPPING;
        transmit(*test_stream);

        EventBits_t bits = xEventGroupWaitBits(channel_hopping, BIT0 | BIT1, pdFALSE, pdFALSE, portMAX_DELAY);


        ESP_LOGI(TAG_WIFI, "Checking if peer is found on channel %d", channel);

        if (bits & BIT0 && !(bits & BIT1))
        {
            ESP_LOGI(TAG_WIFI, "Peer found on channel %d", channel);
            last_good_channel = channel; // update last good channel

            xEventGroupClearBits(channel_hopping, BIT0); // clear the event bit for next time
            return true;
        }

        ESP_LOGI(TAG_WIFI, "Peer not found on channel %d, continue", channel);
        xEventGroupClearBits(channel_hopping, BIT1);
    }

    ESP_LOGE(TAG_WIFI, "Peer not found on any channel, reverting to last known good channel %d", last_good_channel);
    return false;
}
#endif 

#endif //INIT_WIFI_CPP