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

#include "lib/src/init_wifi.cpp"

#include "lib/src/engine.c"

#include "lib/src/helpers.c"

// You can modify these according to your boards.
#define UART_BAUD_RATE 115200
#define UART_PORT_NUM  0
#define UART_TX_IO     UART_PIN_NO_CHANGE
#define UART_RX_IO     UART_PIN_NO_CHANGE

#define SWITCH        0

#define PEER_MAC_ADDR {0xd4, 0xe9, 0xf4, 0xfb, 0x0a, 0x64}  // placeholder

static const char *TAG = "app_main";

static void app_uart_read_task(void *arg)
{
    esp_err_t ret  = ESP_OK;
    uint32_t count = 0;
    size_t size    = 0;
    uint8_t *data  = ESP_CALLOC(1, ESPNOW_DATA_LEN);

    ESP_LOGI(TAG, "Uart read handle task is running");

    espnow_frame_head_t frame_head = {
        .retransmit_count = CONFIG_RETRY_NUM,
        .broadcast        = true,
    };

    for (;;) {
        size = uart_read_bytes(UART_PORT_NUM, data, ESPNOW_DATA_LEN, pdMS_TO_TICKS(10));
        ESP_ERROR_CONTINUE(size <= 0, "");

        ret = espnow_send(ESPNOW_DATA_TYPE_DATA, ESPNOW_ADDR_BROADCAST, data, size, &frame_head, portMAX_DELAY);
        ESP_ERROR_CONTINUE(ret != ESP_OK, "<%s> espnow_send", esp_err_to_name(ret));

        ESP_LOGI(TAG, "espnow_send, count: %" PRIu32 ", size: %u, data: %s", count++, size, data);
        memset(data, 0, ESPNOW_DATA_LEN);
    }

    ESP_LOGI(TAG, "Uart handle task is exit");

    ESP_FREE(data);
    vTaskDelete(NULL);
}

static void app_uart_initialize()
{
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
#if SOC_UART_SUPPORT_REF_TICK
        .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
        .source_clk = UART_SCLK_XTAL,
#endif
    };

    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX_IO, UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 8 * ESPNOW_DATA_LEN, 8 * ESPNOW_DATA_LEN, 0, NULL, 0));

    xTaskCreate(app_uart_read_task, "app_uart_read_task", 4 * 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}


static esp_err_t app_uart_write_handle(uint8_t *src_addr, void *data,
                                       size_t size, wifi_pkt_rx_ctrl_t *rx_ctrl)
{
    ESP_PARAM_CHECK(src_addr);
    ESP_PARAM_CHECK(data);
    ESP_PARAM_CHECK(size);
    ESP_PARAM_CHECK(rx_ctrl);

    static uint32_t count = 0;

    ESP_LOGI(TAG, "espnow_recv, <%" PRIu32 "> [" MACSTR "][%d][%d][%u]: %.*s",
             count++, MAC2STR(src_addr), rx_ctrl->channel, rx_ctrl->rssi, size, size, (char *)data);

    return ESP_OK;
}

void app_send_cb_handle(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
{
    if (!tx_info) return;
    ESP_LOGI(TAG, "Send callback called, dest=" MACSTR ", src=" MACSTR ", status=%s",
             MAC2STR(tx_info->des_addr), MAC2STR(tx_info->src_addr), esp_err_to_name(status));
}

void app_recv_cb_handle(const esp_now_recv_info_t *rx_info, const uint8_t *data, int size)
{
    if (!rx_info || !data || size <= 0) return;
    ESP_LOGI(TAG, "Receive callback called, src=" MACSTR ", size=%d, data: %.*s",
             MAC2STR(rx_info->src_addr), size, (char *)data);

    auto payload = (data_stream_t*) (data);

    uint32_t response_crc = payload->crc;

    payload->crc = 0;

    printf("Res CRC: %ld", response_crc);
    printf("Calc CRC: %ld", crc32(payload, sizeof(data_stream_t)));

    if (response_crc == crc32(payload, sizeof(data_stream_t)))
    {
        ESP_LOGI(TAG, "Success CRC are correct");
    }
    else {
        ESP_LOGE(TAG, "Error CRC");
    }
    


}

void app_main()
{
    uint8_t peer_mac[6] = PEER_MAC_ADDR;

    espnow_storage_init();
    app_uart_initialize();

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


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        e_actions_t action = 1;
        time_t rawtime;

        data_stream_t data = {0};
        data.command = action;
        memcpy(data.dest, peer_mac, 6);
        data.sent = time(&rawtime);
        data.ttl = 1212;

        memset(&data, 0, sizeof(data));

        uint32_t crc = crc32(&data, sizeof(data));
        data.crc = crc;


        ESP_ERROR_CHECK(esp_now_send(peer_mac,(uint8_t*)&data , sizeof(data)));
        
    }

}
