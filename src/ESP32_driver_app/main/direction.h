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

#define INDILEFTBTN 5
#define INDIRIGHTBTN 19
#define INDIALARMBTN 21

void configDirectionPins();
void readDirection(void * arg);
