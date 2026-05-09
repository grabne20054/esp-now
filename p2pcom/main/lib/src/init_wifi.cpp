#ifndef INIT_WIFI_CPP
#define INIT_WIFI_CPP

#include "../include/init_wifi.hpp"


static const char *TAG_WIFI = "init_wifi";

esp_err_t init()
{
    esp_event_loop_create_default();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG_WIFI, "WiFi initialized successfully");

    return ESP_OK;
}

esp_err_t deinit()
{
    esp_wifi_stop();
    esp_wifi_deinit();
    return ESP_OK;
}

esp_err_t set_mode(wifi_mode_t mode)
{
    ESP_LOGI(TAG_WIFI, "Setting WiFi mode to %d", mode);
    return esp_wifi_set_mode(mode);
}

esp_err_t get_mac(uint8_t mac[6])
{
    return esp_wifi_get_mac(ESPNOW_WIFI_IF, mac);
}

#endif //INIT_WIFI_CPP