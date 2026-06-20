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

// 0a 64 is ws, 4a 6c is fieldside


// You can modify these according to your boards.
#define UART_BAUD_RATE 115200
#define UART_PORT_NUM  0
#define UART_TX_IO     UART_PIN_NO_CHANGE
#define UART_RX_IO     UART_PIN_NO_CHANGE

#define SSID "gramesch"
#define PASS "gramesch!?"

static const char *TAG = "app_main";

static EventGroupHandle_t espnow_event_channel_found;

void app_send_cb_handle(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (!tx_info) return;
    ESP_LOGI(TAG, "Send callback called, dest=" MACSTR ", src=" MACSTR ", status=%s",
             MAC2STR(tx_info->des_addr), MAC2STR(tx_info->src_addr), esp_err_to_name(status));
    #if SWITCH == 1
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        peer_found = true;
        ESP_LOGI(TAG, "Peer found, setting peer_found to true");
    }
    else
    {
        peer_found = false;
    }
    #endif


    if (status == ESP_OK) {
        ESP_LOGI(TAG, "Data sent successfully");
    } else {
        ESP_LOGE(TAG, "Data send failed with status: %d", status);
    }
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

    ESP_LOGI(TAG, "global seq: %d", global_seq);
    ESP_LOGI(TAG, "local seq: %d", data_payload->seq);

    if ((data_payload->seq == global_seq) && (waiting_for_recv))
    {
        ESP_LOGI(TAG, "right package recv");
        waiting_for_recv=false;

        if (response_crc == crc32(data_payload, sizeof(*data_payload)))
        {
            
            ESP_LOGI(TAG, "Success CRC are correct");

            queue_t * unq_queue = get_unq_queue();

            // ready to perform desired action
            bool recvadd = add_to_queue(data_payload, unq_queue);
            if (recvadd)
            {
                ESP_LOGI(TAG, "Data added to queue successfully");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to add data to queue");
            }
            free(data_payload);
        }
        else {
            ESP_LOGE(TAG, "Error CRC");
        }

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
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    ESP_LOGI(TAG, "WiFi MAC address: [" MACSTR "]", MAC2STR(mac));

    
    ESP_ERROR_CHECK( esp_now_init());

    // setup tcpip stack if on ws side
    if (SWITCH == 0)
    {

        wifi_config_t cfg = {
        .sta = {
            .ssid = SSID,
            .password = PASS,

        }
        };
        bool restcip = set_up_tcpip_stack(cfg);
        if (!restcip)
        {
            ESP_LOGE(TAG, "Failed to set up TCP/IP stack");
            return;
        }
    }

    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);

    ESP_LOGI(TAG, "Current WiFi channel: %d", primary);

    memcpy(peer.peer_addr, peer_mac, 6);
    #if SWITCH == 0
    peer.channel = primary; // set to current channel
    #endif
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

   

    if (SWITCH == 0)
    {   
        httpd_handle_t server = start_websocket();
    }
    #if SWITCH == 1
    else if (SWITCH == 1)
    {

        // main loop for field side

        // perform channel hopping if on field side
        if (!hopping_channel())
        {
            ESP_LOGI(TAG, "Failed to find peer during channel hopping, continuing with last known good channel");
            return;
        }


        queue_t * queue = get_unq_queue();

        while (1)
        {
            if (!is_empty(queue))
            {
                data_stream_t stream = dequeue(queue);

                // engine inf here
                // simulating engine 
                vTaskDelay(pdMS_TO_TICKS(1000));

                e_actions_t action = 4;

                data_stream_t * resstream = prepare_data_stream(action);

                // locking!!

                ESP_ERROR_CHECK(transmit(resstream));


            }
            else
            {
                vTaskDelay(pdMS_TO_TICKS(1000));
                ESP_LOGI(TAG, "waiting...");
                continue;

            }
            
        }
    
    
    }
    #endif

}

#endif
