#include "wheel.h"

static const char *TAG = "wheel";

extern esp_mqtt_client_handle_t client;
extern bool connected;

static SemaphoreHandle_t s_timer_sem;

static void IRAM_ATTR timer_group_isr_callback(void *args) 
{
    xSemaphoreGiveFromISR(s_timer_sem, NULL);
}

void createTimer()
{
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN
    };

    timer_init(TIMER_GROUP_0, TIMER_0, &config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_ALARM);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, timer_group_isr_callback, NULL, 0);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

int32_t toAngle(uint32_t reading)
{
    return ((float)(reading) / 4.422222) - 450;
}

void readAnalog(void * arg)
{
    s_timer_sem = xSemaphoreCreateBinary();
    if (s_timer_sem == NULL)  
    {
        printf("Binary semaphore can not be created");
        vTaskDelete(NULL);
    }

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten( POTPIN, ADC_ATTEN_DB_0);

    createTimer();

    char payload[10];
    uint32_t adc_reading = 0;
    int32_t prev_value = 2100;

    while (true)
    {
        if (xSemaphoreTake(s_timer_sem, portMAX_DELAY) == pdPASS && connected) 
        {
            for (size_t i = 0; i < NO_OF_SAMPLES; i++) adc_reading += adc1_get_raw(POTPIN);

            adc_reading /= NO_OF_SAMPLES;

            if (((prev_value - 20) < adc_reading && (prev_value + 20) > adc_reading)) continue;

            prev_value = adc_reading;

            sprintf(payload, "%d", toAngle(adc_reading));
            esp_mqtt_client_publish(client, "/car/wheel/angle", payload, 0, 0, 0);

            //printf("wyslano %d\n", toAngle(adc_reading));
        }
    }

    vTaskDelete(NULL);
}