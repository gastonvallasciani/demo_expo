//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"

#include "../include/board_def.h"
#include "../include/global_manager.h"
#include "../include/led_manager.h"
#include "../include/pwm_manager.h"
#include "../include/vege_manager.h"
#include "../include/analog_input_manager.h"
#include "../include/button_manager.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 50

#define MAX_DUTY_CYCLE 100
#define MIN_DUTY_CYCLE 10
#define DUTY_CYCLE_STEP 20
//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    CMD_UNDEFINED = 0,
    UPDATE_PWM_OUPUT_CHANGE_TIME = 1,
    RELE_VEGE_ON = 2,
    RELE_VEGE_OFF = 3,
} global_event_cmds_t;

typedef struct{
    global_event_cmds_t cmd;
    uint8_t pwm_output_update_new_time;
} global_event_t;
//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t global_manager_queue;
//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void global_manager_task(void *arg);
//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
static void global_manager_task(void *arg)
{
    global_event_t global_ev;
    global_info_t global_info;

    vege_manager_init();

    global_info.pwm_output_info.output_pwm_update_time = 10;
    global_info.rele_vege_info.rele_vege_start_cycle_time = global_info.pwm_output_info.output_pwm_update_time;
    global_info.rele_vege_info.rele_vege_cycle_time_width = 10;
    global_info.rele_vege_info.enable_rele_vege_cycle = false;
    global_info.update_output_counter = 0;
    global_info.pwm_output_info.duty_cycle = MIN_DUTY_CYCLE;
    global_info.pwm_output_info.duty_cycle_step = DUTY_CYCLE_STEP;
    global_info.pwm_output_info.update_orientation = INCREASING;

    pwm_manager_update_pwm(global_info.pwm_output_info.duty_cycle);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    while (1)
    {
        if (xQueueReceive(global_manager_queue, &global_ev, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            switch (global_ev.cmd)
            {
            case CMD_UNDEFINED:
                break;
            case UPDATE_PWM_OUPUT_CHANGE_TIME:
                global_info.pwm_output_info.output_pwm_update_time = global_ev.pwm_output_update_new_time;
                global_info.rele_vege_info.rele_vege_start_cycle_time = global_info.pwm_output_info.output_pwm_update_time;
                global_info.rele_vege_info.rele_vege_cycle_time_width = global_ev.pwm_output_update_new_time;

                global_info.update_output_counter = 0;

                #ifdef DEBUG_MODULE
                    printf(" New output_pwm_update_time: %d cuentas \n", global_info.pwm_output_info.output_pwm_update_time);
                #endif
                break;
            case RELE_VEGE_ON:
                //global_info.rele_vege_status = RELE_VEGE_ENABLE;
                //led_manager_rele_vege_on();
                //vege_manager_turn_on();
                break;
            case RELE_VEGE_OFF:
                //global_info.rele_vege_status = RELE_VEGE_DISABLE;
                //led_manager_rele_vege_off();
                //vege_manager_turn_off();
                break;
            default:
                break;
            }
        }
        else
        {
            global_info.update_output_counter++;

            #ifdef DEBUG_MODULE
                    printf(" Actual update_pwm_output_counter: %d \n", global_info.update_output_counter);
            #endif

            if((global_info.update_output_counter == global_info.pwm_output_info.output_pwm_update_time) && (global_info.rele_vege_info.enable_rele_vege_cycle == false))
            {
                if(global_info.pwm_output_info.update_orientation == INCREASING)
                {
                    global_info.pwm_output_info.duty_cycle += global_info.pwm_output_info.duty_cycle_step;  
                    global_info.update_output_counter = 0;
                    
                    if(global_info.pwm_output_info.duty_cycle > MAX_DUTY_CYCLE)
                    {
                        global_info.pwm_output_info.duty_cycle = MAX_DUTY_CYCLE;
                        global_info.rele_vege_info.enable_rele_vege_cycle = true;
                        global_info.pwm_output_info.update_orientation = DECREASING;
                        
                    }

                    #ifdef DEBUG_MODULE
                        printf(" New duty_cycle: %d \n", global_info.pwm_output_info.duty_cycle);
                        printf(" output_pwm_update_time: %d cuentas\n", global_info.pwm_output_info.output_pwm_update_time);
                    #endif
                    
                    pwm_manager_update_pwm(global_info.pwm_output_info.duty_cycle);
                    
                }
                else if(global_info.pwm_output_info.update_orientation == DECREASING)
                {
                    global_info.pwm_output_info.duty_cycle -= global_info.pwm_output_info.duty_cycle_step; 
                    global_info.update_output_counter = 0;

                    if(global_info.pwm_output_info.duty_cycle < MIN_DUTY_CYCLE)
                    {
                        global_info.pwm_output_info.duty_cycle = 0;
                        global_info.pwm_output_info.update_orientation = INCREASING;
                    }

                    #ifdef DEBUG_MODULE
                        printf(" New duty_cycle: %d \n", global_info.pwm_output_info.duty_cycle);
                        printf(" output_pwm_update_time: %d cuentas\n", global_info.pwm_output_info.output_pwm_update_time);
                    #endif
                    
                    pwm_manager_update_pwm(global_info.pwm_output_info.duty_cycle);
                }
            }
            else if(global_info.rele_vege_info.enable_rele_vege_cycle == true)
            {
                if(global_info.update_output_counter == global_info.rele_vege_info.rele_vege_start_cycle_time)
                {
                    pwm_manager_turn_off_pwm();
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    led_manager_rele_vege_on();
                    vege_manager_turn_on();
                    #ifdef DEBUG_MODULE
                        printf(" TURN ON RELE VEGE \n");
                    #endif
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    pwm_manager_update_pwm(global_info.pwm_output_info.duty_cycle);
                }

                if(global_info.update_output_counter == global_info.rele_vege_info.rele_vege_start_cycle_time + global_info.rele_vege_info.rele_vege_cycle_time_width)
                {
                    pwm_manager_turn_off_pwm();
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    led_manager_rele_vege_off();
                    vege_manager_turn_off();
                    global_info.update_output_counter = 0;
                    #ifdef DEBUG_MODULE
                        printf(" TURN OFF RELE VEGE \n");
                    #endif
                    global_info.rele_vege_info.enable_rele_vege_cycle = false;
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                    pwm_manager_update_pwm(global_info.pwm_output_info.duty_cycle);
                }
                
            }
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void global_manager_init(void)
{
    global_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(global_event_t));

    xTaskCreate(global_manager_task, "global_manager_task", configMINIMAL_STACK_SIZE * 15,
                NULL, configMAX_PRIORITIES - 1, NULL);
}
//------------------------------------------------------------------------------
void global_manager_set_rele_vege_status_off(void)
{
    global_event_t ev;
    ev.cmd = RELE_VEGE_OFF;
    xQueueSend(global_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void global_manager_set_rele_vege_status_on(void)
{
    global_event_t ev;
    ev.cmd = RELE_VEGE_ON;
    xQueueSend(global_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void global_manager_set_pwm_power_value_manual(uint8_t output_pwm_change_time)
{
    global_event_t ev;

    ev.cmd = UPDATE_PWM_OUPUT_CHANGE_TIME;
    ev.pwm_output_update_new_time = output_pwm_change_time;
    xQueueSend(global_manager_queue, &ev, 10);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------
