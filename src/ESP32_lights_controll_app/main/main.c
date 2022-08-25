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
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "ESP32_driver_main";

#define CONFIG_BROKER_URL "mqtt://192.168.1.79:1883"
#define OKLED 12
#define LONGLED 19
#define NORMLED 18

esp_mqtt_client_handle_t client;
bool connected = false;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
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
        gpio_set_level(OKLED, true);

        esp_mqtt_client_subscribe(client, "/car/lights/normal", 0);
        esp_mqtt_client_subscribe(client, "/car/lights/long", 0);

        break;
    case MQTT_EVENT_DISCONNECTED:

        ESP_LOGI(TAG, "ESP disconected");
        connected = false;
        gpio_set_level(OKLED, false);
        esp_mqtt_client_reconnect(client);

        break;
    case MQTT_EVENT_SUBSCRIBED:
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        break;
    case MQTT_EVENT_PUBLISHED:
        break;
    case MQTT_EVENT_DATA:
        if (strcmp(event->topic, "/car/lights/normal") >= 0)
        {
            if(strcmp(event->data, "on") >= 0) gpio_set_level(NORMLED, true);
            else                          gpio_set_level(NORMLED, false);
        }
        else
        {
            if(strcmp(event->data, "on") >= 0) gpio_set_level(LONGLED, true);
            else                          gpio_set_level(LONGLED, false);
        }

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

void configPins()
{
    gpio_pad_select_gpio(OKLED);
    gpio_pad_select_gpio(LONGLED);
    gpio_pad_select_gpio(NORMLED);

    gpio_set_direction(OKLED, GPIO_MODE_OUTPUT);
    gpio_set_direction(LONGLED, GPIO_MODE_OUTPUT);
    gpio_set_direction(NORMLED, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    configPins();

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
}