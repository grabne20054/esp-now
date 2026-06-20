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

#include "send.h"

static esp_now_peer_info_t peer = {0};

#if SWITCH == 1
static uint8_t last_good_channel = 6;
static bool peer_found = false; // always false
#endif

esp_err_t init(void);

esp_err_t deinit(void);

esp_err_t set_mode(void);

esp_err_t set_mac(uint8_t mac[6]);

esp_err_t get_mac(uint8_t mac[6]);

bool set_up_tcpip_stack(wifi_config_t conf);

#if SWITCH == 1
bool hopping_channel(void);
#endif

#endif // INIT_WIFI_HPP