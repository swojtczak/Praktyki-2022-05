#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "mqtt_client.h"

#define INDILEFTBTN 5
#define INDIRIGHTBTN 19
#define INDIALARMBTN 21

void configDirectionPins();
void readDirection(void * arg);