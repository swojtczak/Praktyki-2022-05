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

#define WINLFUP    25
#define WINLFDOWN  26
#define WINRFUP    13 
#define WINRFDOWN  27
#define WINLBUP    22
#define WINLBDOWN  23
#define WINRBUP    32 
#define WINRBDOWN  14

void configWindowPins();
void readWindow(void *arg);