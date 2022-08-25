#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define TIMER_DIVIDER 1000
#define TIMER_ALARM 800
#define NO_OF_SAMPLES 32
#define POTPIN ADC1_CHANNEL_5

void readAnalog(void *arg);