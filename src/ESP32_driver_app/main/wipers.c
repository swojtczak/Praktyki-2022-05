#include "wipers.h"

static const char *TAG = "wipers";

extern esp_mqtt_client_handle_t client;
extern bool connected;

static SemaphoreHandle_t s_wipers_sem;

uint8_t wipers_debounce = 0;

bool site = false;
bool fluid = false;
bool wipe = false;
bool once = false;

static void IRAM_ATTR side_isr_handler(void *args)
{
    if (wipers_debounce == 0) 
    {
        site = !site;
        xSemaphoreGiveFromISR(s_wipers_sem, NULL);  
    }
    wipers_debounce++; 
}

static void IRAM_ATTR fluid_isr_handler(void *args)
{
    if (wipers_debounce == 0) 
    {
        fluid = !fluid;
        xSemaphoreGiveFromISR(s_wipers_sem, NULL);  
    }
    wipers_debounce++; 
}

static void IRAM_ATTR on_isr_handler(void *args)
{
    if (wipers_debounce == 0) 
    {
        wipe = !wipe;
        xSemaphoreGiveFromISR(s_wipers_sem, NULL);  
    }
    wipers_debounce++; 
}

static void IRAM_ATTR once_isr_handler(void *args)
{
    once = true;
    xSemaphoreGiveFromISR(s_wipers_sem, NULL);  
}

void configWipersPins()
{
    gpio_pad_select_gpio(WIPFB);
    gpio_pad_select_gpio(WIPFLU);
    gpio_pad_select_gpio(WIPONCE);
    gpio_pad_select_gpio(WIPON);
    gpio_pad_select_gpio(LEDFLU);
    gpio_pad_select_gpio(LEDFB);

    gpio_set_direction(WIPFB, GPIO_MODE_INPUT);
    gpio_set_direction(WIPFLU, GPIO_MODE_INPUT);
    gpio_set_direction(WIPONCE, GPIO_MODE_INPUT);
    gpio_set_direction(WIPON, GPIO_MODE_INPUT);
    gpio_set_direction(LEDFLU, GPIO_MODE_OUTPUT);
    gpio_set_direction(LEDFB, GPIO_MODE_OUTPUT);

    gpio_pullup_en(WIPFB);
    gpio_pullup_en(WIPFLU);
    //gpio_pullup_en(WIPONCE);
    //gpio_pullup_en(WIPON);

    gpio_set_intr_type(WIPFB, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(WIPFLU, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(WIPONCE, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(WIPON, GPIO_INTR_NEGEDGE);


    gpio_isr_handler_add(WIPFB, side_isr_handler, NULL);
    gpio_isr_handler_add(WIPFLU, fluid_isr_handler, NULL);
    gpio_isr_handler_add(WIPONCE, on_isr_handler, NULL);
    gpio_isr_handler_add(WIPON, once_isr_handler, NULL);
}

void readWiper(void *arg)
{
    s_wipers_sem = xSemaphoreCreateBinary();
    if (s_wipers_sem == NULL)  
    {
        printf("Binary semaphore can not be created");
        vTaskDelete(NULL);
    }

    configWipersPins();

    char topic[20];
    char payload[8];

    while (true)
    {
        if (xSemaphoreTake(s_wipers_sem, portMAX_DELAY) == pdPASS && connected) 
        {
            ESP_LOGI(TAG, "ggg");
            gpio_set_level(LEDFB, site);
            gpio_set_level(LEDFLU, fluid);

            if(site) strcpy(topic, "/car/wipers/back");
            else     strcpy(topic, "/car/wipers/front");

            if (fluid)
            {
                if (once)
                {
                    esp_mqtt_client_publish(client, topic, "fluid_once", 0, 0, 0);
                    wipers_debounce = 0;
                    once = false;
                    continue;
                }

                if(wipe)
                {
                    esp_mqtt_client_publish(client, topic, "fluid", 0, 0, 0);
                }
                else
                {
                    esp_mqtt_client_publish(client, topic, "off", 0, 0, 0);
                }
            }
            else
            {
                if (once)
                {
                    esp_mqtt_client_publish(client, topic, "once", 0, 0, 0);
                    wipers_debounce = 0;
                    once = false;
                    continue;
                }

                if(wipe)
                {
                    esp_mqtt_client_publish(client, topic, "on", 0, 0, 0);
                }
                else
                {
                    esp_mqtt_client_publish(client, topic, "off", 0, 0, 0);
                }
            }

            vTaskDelay(20);

            wipers_debounce = 0;
        }
    }

    vTaskDelete(NULL);
}