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


static const char *TAG = "app_main";

void app_main(void)
{
    pwm_init();

    ESP_LOGI(TAG, "here");

    motor_forward(PWM_MAX_DUTY /2);

    vTaskDelay(pdMS_TO_TICKS(10000));

    motor_stop();

    vTaskDelay(pdMS_TO_TICKS(10000));

    motor_backward(PWM_MAX_DUTY/2);

}

#endif
