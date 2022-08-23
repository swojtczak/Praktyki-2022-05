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

#define WINLFUP    23
#define WINLFDOWN  22
#define WINRFUP    25 
#define WINRFDOWN  26
#define WINLBUP    0
#define WINLBDOWN  0
#define WINRBUP    0 
#define WINRBDOWN  0

void configWindowPins();
void readWindow(void *arg);