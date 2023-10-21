#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "analog_input_manager.h"
#include "led_manager.h"
#include "pwm_manager.h"
#include "button_manager.h"
#include "global_manager.h"

void app_main() 
{
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