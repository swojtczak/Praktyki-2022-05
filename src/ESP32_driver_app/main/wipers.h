#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define WIPFB    17
#define WIPFLU   16
#define WIPONCE  34 
#define WIPON    35

#define LEDFLU   2
#define LEDFB    18

void configWipersPins();
void readWiper(void *arg);