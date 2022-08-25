#include "lights.h"


static const char *TAG = "lights";

extern esp_mqtt_client_handle_t client;
extern bool connected;

static SemaphoreHandle_t s_lights_sem;

uint8_t lights_debounce = 0;

bool lights_normal = false;
bool lights_long = false;

static void IRAM_ATTR normal_isr_handler(void *args)
{
    if (lights_debounce == 0) 
    {
        lights_normal = !lights_normal;
        xSemaphoreGiveFromISR(s_lights_sem, NULL);  
    }
    lights_debounce++; 
}

static void IRAM_ATTR long_isr_handler(void *args)
{
    if (lights_debounce == 0) 
    {
        lights_long = !lights_long;
        xSemaphoreGiveFromISR(s_lights_sem, NULL);  
    }
    lights_debounce++; 
}

void configLightsPins()
{
    gpio_pad_select_gpio(LIGNOR);
    gpio_pad_select_gpio(LIGLON);

    gpio_set_direction(LIGNOR, GPIO_MODE_INPUT);
    gpio_set_direction(LIGLON, GPIO_MODE_INPUT);

    gpio_pullup_en(LIGNOR);
    //gpio_pullup_en(LIGLON);

    gpio_set_intr_type(LIGNOR, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(LIGLON, GPIO_INTR_NEGEDGE);

    gpio_isr_handler_add(LIGNOR, normal_isr_handler, NULL);
    gpio_isr_handler_add(LIGLON, long_isr_handler, NULL);
}

void readLights(void *arg)
{
    s_lights_sem = xSemaphoreCreateBinary();
    if (s_lights_sem == NULL)  
    {
        printf("Binary semaphore can not be created");
        vTaskDelete(NULL);
    }

    configLightsPins();

    while (true)
    {
        if (xSemaphoreTake(s_lights_sem, portMAX_DELAY) == pdPASS && connected) 
        {
            if (lights_normal) esp_mqtt_client_publish(client, "/car/lights/normal", "on", 0, 0, 0);
            else               esp_mqtt_client_publish(client, "/car/lights/normal", "off", 0, 0, 0);

            if (lights_long) esp_mqtt_client_publish(client, "/car/lights/long", "on", 0, 0, 0);
            else               esp_mqtt_client_publish(client, "/car/lights/long", "off", 0, 0, 0);

            vTaskDelay(20);
            lights_debounce = 0;
        }
    }

    vTaskDelete(NULL);
}