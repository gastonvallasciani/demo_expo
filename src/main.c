#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "analog_input_manager.h"
#include "led_manager.h"
#include "pwm_manager.h"
#include "button_manager.h"
#include "global_manager.h"
#include "../include/version.h"

static char fw_version[4];
static uint8_t version_len = 0;

void app_main() 
{
    get_version(fw_version, &version_len);
    printf("FIRMWARE VERSION: %s \n", fw_version);

    led_manager_init();
    button_manager_init();
    analog_input_manager_init();
    pwm_manager_init();
    global_manager_init();

    while(true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}