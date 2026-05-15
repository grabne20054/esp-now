#ifndef INIT_WIFI_HPP
#define INIT_WIFI_HPP

#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   WIFI_IF_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF   WIFI_IF_AP
#endif

#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_system.h"

esp_err_t init();

esp_err_t deinit();

esp_err_t set_mode();

esp_err_t get_mac(uint8_t mac[6]);

void set_up_tcpip_stack(wifi_config_t conf);

#endif // INIT_WIFI_HPP