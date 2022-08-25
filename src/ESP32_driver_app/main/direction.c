#include "direction.h"

static const char *TAG = "indicators";

bool direction[2];
static SemaphoreHandle_t s_direction_sem;

extern esp_mqtt_client_handle_t client;
extern bool connected;

uint8_t direction_debounce = 0;

static void IRAM_ATTR lever_isr_handler(void * args)
{
    xSemaphoreGiveFromISR(s_direction_sem, NULL);  
}

static void IRAM_ATTR hazard_isr_handler(void * args)
{
    if (direction_debounce == 0) xSemaphoreGiveFromISR(s_direction_sem, NULL);  
    direction_debounce++;
}

void configDirectionPins()
{
    gpio_pad_select_gpio(INDILEFTBTN);
    gpio_pad_select_gpio(INDIRIGHTBTN);
    gpio_pad_select_gpio(INDIALARMBTN);

    gpio_set_direction(INDILEFTBTN, GPIO_MODE_INPUT);
    gpio_set_direction(INDIRIGHTBTN, GPIO_MODE_INPUT);
    gpio_set_direction(INDIALARMBTN, GPIO_MODE_INPUT);

    gpio_pullup_en(INDILEFTBTN);
    gpio_pullup_en(INDIRIGHTBTN);
    gpio_pullup_en(INDIALARMBTN);

    gpio_set_intr_type(INDILEFTBTN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(INDIRIGHTBTN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(INDIALARMBTN, GPIO_INTR_NEGEDGE);

    gpio_isr_handler_add(INDILEFTBTN, lever_isr_handler, NULL);
    gpio_isr_handler_add(INDIRIGHTBTN, lever_isr_handler, NULL);
    gpio_isr_handler_add(INDIALARMBTN, hazard_isr_handler, NULL);
}

void readDirection(void * arg)
{
    s_direction_sem = xSemaphoreCreateBinary();
    if (s_direction_sem == NULL)  
    {
        printf("Binary semaphore can not be created");
        vTaskDelete(NULL);
    }

    configDirectionPins();
    bool hazard = false;

    while (true)
    {
        if (xSemaphoreTake(s_direction_sem, portMAX_DELAY) == pdPASS && connected) 
        {   
            
            vTaskDelay(20);
            if (direction_debounce > 0)
            {
                if (hazard)
                {
                    esp_mqtt_client_publish(client, "/car/indicator/hazard", "off", 0, 0, 0);
                }
                else
                {
                    esp_mqtt_client_publish(client, "/car/indicator/hazard", "on", 0, 0, 0);
                }

                hazard = !hazard;
                direction_debounce = 0;
                
                continue;
            }

            esp_mqtt_client_publish(client, "/car/indicator/left", "off", 0, 0, 0);
            esp_mqtt_client_publish(client, "/car/indicator/right", "off", 0, 0, 0);

            if (!gpio_get_level(INDILEFTBTN))       esp_mqtt_client_publish(client, "/car/indicator/left", "on", 0, 0, 0);
            else if(!gpio_get_level(INDIRIGHTBTN))  esp_mqtt_client_publish(client, "/car/indicator/right", "on", 0, 0, 0);
        }
    }

    vTaskDelete(NULL);
}

