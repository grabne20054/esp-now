#ifndef ENGINE_C
#define ENGINE_C

#include "../include/engine.h"

static const char *TAG_ENGINE = "ENGINE";

//EventGroupHandle_t engine_event_group = NULL;

void pwm_init(void)
{
    gpio_config_t en_conf = {
        .pin_bit_mask = (1ULL << R_EN_GPIO) | (1ULL << L_EN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&en_conf);

    gpio_set_level(R_EN_GPIO, 1);
    gpio_set_level(L_EN_GPIO, 1);

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t rpwm = {
        .gpio_num = RPWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };


    ledc_channel_config_t lpwm = {
        .gpio_num = LPWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&rpwm));
    ESP_ERROR_CHECK(ledc_channel_config(&lpwm));

    ESP_LOGI(TAG_ENGINE, "BTS7960 initialized");
}

engine_t * init_engine()
{
    //engine_event_group = xEventGroupCreate();

    engine_t *engine = malloc(sizeof(engine_t));
    if (engine == NULL)
    {
        ESP_LOGE(TAG_ENGINE, "Failed to allocate memory for engine");
        return NULL;
    }

    engine->position = 0;
    engine->status = READY;

    //pthread_mutex_init(engine->mutex, NULL);

    return engine;
}

engine_t * get_unq_engine()
{
    static engine_t *engine_instance = NULL;
    if (engine_instance == NULL)
    {
        engine_instance = init_engine();
    }
    return engine_instance;
}


void engine_forward(uint16_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    ESP_LOGI(TAG_ENGINE, "Motor forward with duty: %d", duty);

}

void engine_backward(uint16_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ESP_LOGI(TAG_ENGINE, "Motor backward with duty: %d", duty);
}

void engine_stop(void)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

e_status_t perform_engine_action(engine_t *engine, e_actions_t action)
{

    //pthread_mutex_lock(&engine->mutex);
    engine->status = BUSY;

    switch (action)
    {
        case OPEN:
            engine_forward(1023);
            break;
        case CLOSE:
            engine_backward(1023);
            break;
        // TO DO GET POS,  STATUS
        default:
            ESP_LOGE(TAG_ENGINE, "Unknown action: %d", action);
            return ERROR;
    }

    //pthread_mutex_unlock(&engine->mutex);

    return READY;
}

void check_engine(void *pvParameters)
{
    pwm_init();

    engine_t *engine = get_unq_engine();

    while (1)
    {
        uint32_t duty = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ESP_LOGI(TAG_ENGINE, "Current duty cycle: %d", duty);

        uint32_t duty1 = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        vTaskDelay(pdMS_TO_TICKS(1000));

    }
}

#endif