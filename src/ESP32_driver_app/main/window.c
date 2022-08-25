#include "window.h"

#define WINDOW_COUNT 4

static const char *TAG = "windows";
static SemaphoreHandle_t s_window_sem;

extern esp_mqtt_client_handle_t client;
extern bool connected;

uint8_t window_debounce = 0;

bool window_read_array[] = {false, false, false, false};
int8_t window_state[] = {0, 0, 0, 0};
int8_t prev_window_state[] = {0, 0, 0, 0};
const uint8_t windows[] = {0, 1, 2, 3};
const uint8_t window_button[][2] = {{WINLFUP, WINLFDOWN}, {WINRFUP, WINRFDOWN}, {WINLBUP, WINLBDOWN}, {WINRBUP, WINRBDOWN}};

static void IRAM_ATTR windows_isr_handler(void *args)
{
    if (window_debounce == 0) 
    {
        window_read_array[*(uint8_t*) args] = true;
        xSemaphoreGiveFromISR(s_window_sem, NULL); 
    }
    window_debounce++;
}

void configWindowPins()
{
    gpio_pad_select_gpio(WINLFUP);
    gpio_pad_select_gpio(WINLFDOWN);
    gpio_pad_select_gpio(WINRFUP);
    gpio_pad_select_gpio(WINRFDOWN);
    gpio_pad_select_gpio(WINLBUP);
    gpio_pad_select_gpio(WINLBDOWN);
    gpio_pad_select_gpio(WINRBUP);
    gpio_pad_select_gpio(WINRBDOWN);

    gpio_set_direction(WINLFUP, GPIO_MODE_INPUT);
    gpio_set_direction(WINLFDOWN, GPIO_MODE_INPUT);
    gpio_set_direction(WINRFUP, GPIO_MODE_INPUT);
    gpio_set_direction(WINRFDOWN, GPIO_MODE_INPUT);
    gpio_set_direction(WINLBUP, GPIO_MODE_INPUT);
    gpio_set_direction(WINLBDOWN, GPIO_MODE_INPUT);
    gpio_set_direction(WINRBUP, GPIO_MODE_INPUT);
    gpio_set_direction(WINRBDOWN, GPIO_MODE_INPUT);

    gpio_pullup_en(WINLFUP);
    gpio_pullup_en(WINLFDOWN);
    //gpio_pullup_en(WINRFUP);
    gpio_pullup_en(WINRFDOWN);
    gpio_pullup_en(WINLBUP);
    gpio_pullup_en(WINLBDOWN);
    gpio_pullup_en(WINRBUP);
    gpio_pullup_en(WINRBDOWN);

    gpio_set_intr_type(WINLFUP, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINLFDOWN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINRFUP, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINRFDOWN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINLBUP, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINLBDOWN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINRBUP, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(WINRBDOWN, GPIO_INTR_ANYEDGE);

    gpio_isr_handler_add(WINLFUP, windows_isr_handler, &windows[0]);
    gpio_isr_handler_add(WINLFDOWN, windows_isr_handler, &windows[0]);
    gpio_isr_handler_add(WINRFUP, windows_isr_handler, &windows[1]);
    gpio_isr_handler_add(WINRFDOWN, windows_isr_handler, &windows[1]);
    gpio_isr_handler_add(WINLBUP, windows_isr_handler, &windows[2]);
    gpio_isr_handler_add(WINLBDOWN, windows_isr_handler, &windows[2]);
    gpio_isr_handler_add(WINRBUP, windows_isr_handler, &windows[3]);
    gpio_isr_handler_add(WINRBDOWN, windows_isr_handler, &windows[3]);
}

void readWindow(void *arg)
{
    s_window_sem = xSemaphoreCreateBinary();
    if (s_window_sem == NULL)  
    {
        ESP_LOGE(TAG, "Binary semaphore can not be created");
        vTaskDelete(NULL);
    }

    configWindowPins();

    while(true)
    {
        if (xSemaphoreTake(s_window_sem, portMAX_DELAY) == pdPASS && connected) 
        {
            //vTaskDelay(20);
            for (uint8_t i = 0; i < WINDOW_COUNT; i++)
                if (window_read_array[i])
                {
                    int8_t temp = 0;
                    temp += gpio_get_level(window_button[i][0]);
                    temp -= gpio_get_level(window_button[i][1]);

                    window_state[i] = temp;
                }
            
            char top_char[3];
            char topic[16];
            for (uint8_t i = 0; i < WINDOW_COUNT; i++)
            {
                if (prev_window_state[i] != window_state[i])
                {
                    memset(topic, 0, sizeof topic);
                    itoa(i, top_char, 10);
                    strcat(topic, "/car/window/");
                    strcat(topic, top_char);

                    switch(window_state[i])
                    {
                        case 1:
                            esp_mqtt_client_publish(client, topic, "down", 0, 0, 0);
                            break;
                        case 0:
                            esp_mqtt_client_publish(client, topic, "stop", 0, 0, 0);
                            break;
                        case -1:
                            esp_mqtt_client_publish(client, topic, "up", 0, 0, 0);
                            break;
                    }

                    prev_window_state[i] = window_state[i];
                }
            }
            window_debounce = 0;
        }
    }

    vTaskDelete(NULL);
}
