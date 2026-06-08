#ifndef APP_MAIN_C
#define APP_MAIN_C
/* Get Start Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)
#include "esp_mac.h"
#endif

#include "espnow.h"
#include "espnow_storage.h"
#include "espnow_utils.h"

#include "driver/uart.h"

#include "lib/include/init_wifi.h"

#include "lib/include/engine.h"

#include "lib/include/helpers.h"

#include "lib/include/websocket.h"

#include "lib/include/queue.h"


// You can modify these according to your boards.
#define UART_BAUD_RATE 115200
#define UART_PORT_NUM  0
#define UART_TX_IO     UART_PIN_NO_CHANGE
#define UART_RX_IO     UART_PIN_NO_CHANGE

#define SSID "TP-Link_AA48"
#define PASS "17298865"

#define SWITCH 0 // 0=ws side   1=field side

static const char *TAG = "app_main";

void app_send_cb_handle(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (!tx_info) return;
    ESP_LOGI(TAG, "Send callback called, dest=" MACSTR ", src=" MACSTR ", status=%s",
             MAC2STR(tx_info->des_addr), MAC2STR(tx_info->src_addr), esp_err_to_name(status));
}

void app_recv_cb_handle(const esp_now_recv_info_t *rx_info, const uint8_t *data, int size)
{
    if (!rx_info || !data || size <= 0) return;
    ESP_LOGI(TAG, "Receive callback called, src=" MACSTR ", size=%d", MAC2STR(rx_info->src_addr), size);

    auto payload = (data_stream_t *) (data);

    data_stream_t *data_payload = calloc(1, sizeof(*payload));
    memcpy(data_payload, payload, sizeof(*data_payload));

    uint32_t response_crc = data_payload->crc;

    data_payload->crc = 0;

    if ((data_payload->seq == global_seq) && (waiting_for_recv))
    {
        ESP_LOGI(TAG, "right package recv");

        if (response_crc == crc32(data_payload, sizeof(*data_payload)))
        {
            ESP_LOGI(TAG, "Success CRC are correct");
        }
        else {
            ESP_LOGE(TAG, "Error CRC");
        }

        // ready to perform desired action

    }
    else 
    {
        ESP_LOGI(TAG, "package not looking for drop it");
        //free(payload);
        free(data_payload);
    }

}

void app_main()
{
    uint8_t peer_mac[6] = PEER_MAC_ADDR;

    espnow_storage_init();

    // wifi initialize section
    if (init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi");
        return;
    }
    if (set_mode() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode");
        return;
    }

    uint8_t mac[6];
    esp_wifi_get_mac(ESPNOW_WIFI_IF, mac);
    ESP_LOGI(TAG, "WiFi MAC address: [" MACSTR "]", MAC2STR(mac));

    wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASS,

        }
    };
 
    // TCP IP Stack setup
    set_up_tcpip_stack(cfg);

    ESP_ERROR_CHECK( esp_now_init());
    // Add peer
    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);


    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, peer_mac, 6);
    peer.channel = primary;
    peer.encrypt = false;
    peer.ifidx = WIFI_IF_STA;

    esp_err_t res = esp_now_add_peer(&peer);

    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add peer: %s", esp_err_to_name(res));
        return;
    }
    ESP_LOGI(TAG, "Peer added successfully: [" MACSTR "]", MAC2STR(peer_mac));
    ESP_LOGI(TAG, "Peer information: channel=%d, encrypt=%d, ifidx=%d", peer.channel, peer.encrypt, peer.ifidx);
    ESP_LOGI(TAG, "ESP-NOW initialized successfully");


    // set rate config long range optimised
    esp_wifi_set_protocol(WIFI_IF_STA,
    WIFI_PROTOCOL_11B |
    WIFI_PROTOCOL_LR);


    esp_now_rate_config_t rate = {0};
    rate.phymode = WIFI_PHY_MODE_LR;
    rate.rate = WIFI_PHY_RATE_LORA_250K; // or 500K
    rate.dcm = true;
    rate.ersu = true;

    ESP_ERROR_CHECK( esp_now_set_peer_rate_config(peer_mac, &rate) );


    esp_now_register_send_cb(app_send_cb_handle);
    esp_now_register_recv_cb(app_recv_cb_handle);

    //httpd_handle_t server = start_websocket();



    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        /*e_actions_t action = 2;
        time_t rawtime;

        data_stream_t *data = malloc(sizeof(data_stream_t));
        data->command = action;
        memcpy(data->dest, peer_mac, 6);
        data->sent = time(&rawtime);
        data->ttl = 1212;

        data->crc = 0;

        uint32_t crc = crc32(data, sizeof(*data));
        data->crc = crc;

        ESP_ERROR_CHECK(esp_now_send(peer_mac,(uint8_t*)data , sizeof(*data)));

        free(data);*/

       //ESP_LOGI( TAG, "test queue: %d", test_queue());

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
            return;
        }

        pthread_mutex_unlock(&unq_queue->mutex);
            
        }

}
#endif