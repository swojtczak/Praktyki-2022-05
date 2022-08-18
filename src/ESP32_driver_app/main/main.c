#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "freertos/queue.h"

#include "driver/adc.h"
#include "driver/gpio.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "ESP32_DRIVER";

#define CONFIG_BROKER_URL "mqtt://192.168.1.79:1883"
#define TIMER_DIVIDER 1000
#define TIMER_ALARM 800
#define NO_OF_SAMPLES 32

#define POTPIN ADC1_CHANNEL_5

static SemaphoreHandle_t s_timer_sem;
esp_mqtt_client_handle_t client;
bool connected = false;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
}

static bool IRAM_ATTR timer_group_isr_callback(void * args) 
{
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(s_timer_sem, &high_task_awoken);
    return (high_task_awoken == pdTRUE);
}

static void createTimer()
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

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    //esp_mqtt_client_handle_t client = event->client;
    //int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) 
    {
    case MQTT_EVENT_CONNECTED:

        connected = true;
        ESP_LOGI(TAG, "ESP connected");

        break;
    case MQTT_EVENT_DISCONNECTED:

        ESP_LOGI(TAG, "ESP disconected");
        connected = false;
        esp_mqtt_client_reconnect(client);

        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGW(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) 
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };

    ESP_LOGI(TAG, "client configuration");
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

int32_t toAngle(uint32_t reading)
{
    return ((float)(reading) / 4.422222) - 450;
}

void readAnalog(void * arg)
{
    static char text[10];
    uint32_t adc_reading = 0;

    while (true)
    {
        if (xSemaphoreTake(s_timer_sem, portMAX_DELAY) == pdPASS && connected) 
        {
            for (size_t i = 0; i < NO_OF_SAMPLES; i++) adc_reading += adc1_get_raw(POTPIN);

            adc_reading /= NO_OF_SAMPLES;

            sprintf(text, "%d", toAngle(adc_reading));
            esp_mqtt_client_publish(client, "/car/wheel/angle", text, 0, 0, 0);

            printf("wyslano %d\n", toAngle(adc_reading));
        }
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    s_timer_sem = xSemaphoreCreateBinary();
    if (s_timer_sem == NULL)  printf("Binary semaphore can not be created");

    xTaskCreate(readAnalog, "ReadAnalog", 1024 * 10, NULL, 1, NULL);

    adc1_config_width( ADC_WIDTH_BIT_12 );
    adc1_config_channel_atten( POTPIN, ADC_ATTEN_DB_11);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

    createTimer();
}
