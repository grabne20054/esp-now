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
#include "freertos/event_groups.h"
#include "../include/globals.h"

#include "../include/engine.h"

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

#include "lib/include/helpers.h"

#include "lib/include/websocket.h"

#include "lib/include/data_queue.h"

// You can modify these according to your boards.
#define UART_BAUD_RATE 115200
#define UART_PORT_NUM  0
#define UART_TX_IO     UART_PIN_NO_CHANGE
#define UART_RX_IO     UART_PIN_NO_CHANGE

#define SSID "Grabner_2.4GHz_Buero"
#define PASS "erDWue8crs"

static const char *TAG = "app_main";

#if SWITCH == 0
bool hopping_recv = false;

uint32_t max_send = 1;
#endif

void app_send_cb_handle(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (!tx_info) return;
    ESP_LOGI(TAG, "Send callback called, dest=" MACSTR ", src=" MACSTR ", status=%s",
             MAC2STR(tx_info->des_addr), MAC2STR(tx_info->src_addr), esp_err_to_name(status));
    #if SWITCH == 1
        if (status == ESP_NOW_SEND_SUCCESS)
        {
            ESP_LOGI(TAG, "Peer acknowledged the data successfully");
            xEventGroupSetBits(channel_hopping, BIT0);

        }
        else
        {
            ESP_LOGE(TAG, "Peer did not acknowledge the data");
            xEventGroupSetBits(channel_hopping, BIT1);
        }
    #endif

    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "Data sent successfully");
        send_counter++;
    } else {
        ESP_LOGE(TAG, "Data send failed with status: %d", status);
        failed_send_counter++;
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

    global_seq = get_max_clock_value(global_seq, data_payload->seq) + 1;

    // current time
    time_t current_time = time(NULL);

    #if SWITCH == 0
    if (data_payload->action != HOPPING)
    {
        recv_counter++;
    }
    
    if (data_payload->action == HOPPING)
    {
        ESP_LOGI(TAG, "Received HOPPING command, performing channel hopping");
        hopping_recv = true;
        free(data_payload);
        return;
    }
    else if (data_payload->action == REQUEST)
    {
       ESP_LOGE(TAG, "Received REQUEST PROBABLY TRASH NOT IMPLEMENTED YET");
       recv_wrong_action++;
       free(data_payload);
       return;
    }
    else if (waiting_for_recv && data_payload->action == RESPONSE)
    {
        ESP_LOGI(TAG, "Received RESPONSE, setting waiting_for_recv to false");
        waiting_for_recv = false;

    }
    
    #endif

    #if SWITCH == 1
    if (data_payload->action == RESPONSE)
    {
        ESP_LOGE(TAG, "Received REQUEST PROBABLY TRASH NOT IMPLEMENTED YET");
        free(data_payload);
        return;
    }
    #endif
    

    if ((current_time - data_payload->sent) <= MAX_TTL)
    {
        ESP_LOGI(TAG, "TTL VALID");

        if (response_crc == crc32(data_payload, sizeof(*data_payload)))
        {
            ESP_LOGI(TAG, "CRC VALID");

            if (data_payload->action == RESPONSE)
            {
                ESP_LOGI(TAG, "Received RESPONSE, writing to websocket");
                // write to websocket

                free(data_payload);
                return;
            }

            queue_t * unq_queue = get_unq_queue();

            bool recvadd = add_data_stream_to_queue(data_payload, unq_queue);
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
        else
        {
            ESP_LOGE(TAG, "CRC INVALID");
            recv_wrong_crc++;
            free(data_payload);
            return;
        }
        
    }
    else {
        ESP_LOGE(TAG, "PACKET DEAD");
        free(data_payload);
    }
}

void app_main()
{

    waiting_for_recv = false;
    global_seq = 0;

    #if SWITCH == 1
    channel_hopping = xEventGroupCreate();
    if (channel_hopping == NULL)
    {
        ESP_LOGI(TAG, "Failed to create channel_hopping event group");
        return;
    }
    #endif

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
    #if SWITCH == 0
    

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
    
    #endif

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

    ESP_LOGI(TAG, "WIFI channel: %d, peer channel: %d", primary, peer.channel);

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

    // QUEUE + WS
    TaskHandle_t queue_handle = NULL;

    BaseType_t queue_task_handle = xTaskCreatePinnedToCore(
        do_queue,
        "queue_task",
        4096,
        NULL,
        1,
        &queue_handle,
        0
    );

    #if SWITCH == 1

    TaskHandle_t engine_handle = NULL;

    BaseType_t engine_task_handle = xTaskCreatePinnedToCore(
        check_engine,
        "engine_task",
        4096,
        NULL,
        1,
        &engine_handle,
        1
    );

    if (queue_task_handle == pdPASS)
    {
        ESP_LOGI(TAG, "TASK QUEUE SPAWNED SUCCESSFULLY");
    }
    else
    {
        return;
    }

    #endif

    #if SWITCH == 0

    BaseType_t websocket_task_handle = xTaskCreatePinnedToCore(
        ws_task,
        "websocket_task",
        4096,
        NULL,
        1,
        NULL,
        1
    );

    while (!hopping_recv)
    {
        ESP_LOGI(TAG, "Waiting for HOPPING command from peer...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    

    // test section
    for (size_t i = 0; i < max_send; i++)
    {

        data_stream_t *test = prepare_data_stream(OPEN, REQUEST);

        bool added = add_data_stream_to_queue(test, get_unq_queue());

        if (added)
        {
            add_queue_counter++;
        }
        else {
            failed_queue_counter++;
        }

        vTaskDelay(pdMS_TO_TICKS(3000));
        
    }

    vTaskDelay(pdMS_TO_TICKS(30000));
    
    ESP_LOGI(TAG, "Statistics:");
    ESP_LOGI(TAG, "Total data streams added to queue: %d", add_queue_counter);
    ESP_LOGI(TAG, "Total failed attempts to add data streams to queue: %d", failed_queue_counter);
    ESP_LOGI(TAG, "Total data streams sent: %d", send_counter);
    ESP_LOGI(TAG, "Total failed attempts to send data streams: %d", failed_send_counter);
    ESP_LOGI(TAG, "Total data streams received: %d", recv_counter);
    ESP_LOGI(TAG, "Total received data streams with wrong CRC: %d", recv_wrong_crc);
    ESP_LOGI(TAG, "Total received data streams with wrong action: %d", recv_wrong_action);

    // percentage calculations
    float failed_send_percentage = (float)failed_send_counter / (send_counter + failed_send_counter) * 100.0f;
    float failed_queue_percentage = (float)failed_queue_counter / (add_queue_counter + failed_queue_counter) * 100.0f;

    ESP_LOGI(TAG, "Percentage of failed send attempts: %.2f%% (%d/%d)", failed_send_percentage, failed_send_counter, send_counter + failed_send_counter);
    ESP_LOGI(TAG, "Percentage of failed queue attempts: %.2f%% (%d/%d)", failed_queue_percentage, failed_queue_counter, add_queue_counter + failed_queue_counter);

    float failed_recv_percentage = (float)recv_wrong_crc / (recv_counter) * 100.0f;
    ESP_LOGI(TAG, "Percentage of failed receive attempts (wrong CRC): %.2f%%", failed_recv_percentage);

    float failed_recv_action_percentage = (float)recv_wrong_action / (recv_counter) * 100.0f;
    ESP_LOGI(TAG, "Percentage of failed receive attempts (wrong action): %.2f%%", failed_recv_action_percentage);

    float send_recv_mismatch_percentage = (float)recv_counter / (send_counter) * 100.0f;
    ESP_LOGI(TAG, "Percentage of send/receive mismatch: %.2f%% (%d/%d)", send_recv_mismatch_percentage, send_counter, recv_counter);

    #endif

    #if SWITCH == 1
        // perform channel hopping if on field side
        if (!hopping_channel())
        {
            ESP_LOGI(TAG, "Failed to find peer during channel hopping, continuing with last known good channel");
            return;
        }
    
    #endif
}
    

#endif

