//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"
#include "freertos/queue.h"
#include "../include/pwm_manager.h"
#include "../include/board_def.h"
#include "driver/ledc.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define DEBUG_MODULE 1

#define QUEUE_ELEMENT_QUANTITY 15

#define PWM_TIMER LEDC_TIMER_0
#define PWM_MODE LEDC_HIGH_SPEED_MODE
#define PWM_OUTPUT_IO PWM_OUTPUT // Define the output GPIO
#define PWM_CHANNEL LEDC_CHANNEL_0
#define PWM_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define PWM_DUTY_50_PERCEN (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define PWM_FREQUENCY (1000) // Frequency in Hertz. Set frequency at 1 kHz

#define MIN_15_FADING_TIME 900000
#define SEC_1_FADING_TIME 1000

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    CMD_UNDEFINED = 0,
    TURN_ON_PWM = 1,
    TURN_OFF_PWM = 2,
    UPDATE_DUTY_CYCLE = 3,
}pwm_event_cmds_t;

typedef struct{
    pwm_event_cmds_t cmd;
    uint8_t duty_cycle;
}pwm_event_t;
//------------------- DECLARACION DE DATOS LOCALES -----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t pwm_manager_queue;

//------------------- DECLARACION DE FUNCIONES LOCALES -------------------------
//------------------------------------------------------------------------------
static void pwm_manager_task(void* arg);
static void config_pwm_output(void);
static void turn_on_pwm(uint8_t duty_cycle);
static void update_pwm(uint8_t duty_cycle);
static void turn_off_pwm(void);
//------------------- DEFINICION DE DATOS LOCALES ------------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE DATOS GLOBALES -----------------------------
//------------------------------------------------------------------------------

//------------------- DEFINICION DE FUNCIONES LOCALES --------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void config_pwm_output(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = PWM_MODE,
        .timer_num        = PWM_TIMER,
        .duty_resolution  = PWM_DUTY_RES,
        .freq_hz          = PWM_FREQUENCY,  // Set output frequency at 10 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = PWM_MODE,
        .channel        = PWM_CHANNEL,
        .timer_sel      = PWM_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PWM_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ledc_fade_func_install(0);
}
//------------------------------------------------------------------------------
static void turn_on_pwm(uint8_t duty_cycle)
{
    uint32_t target_duty = 0;

    ledc_set_duty(PWM_MODE, PWM_CHANNEL, 41); // starts at duty cycle 1%
    ledc_update_duty(PWM_MODE, PWM_CHANNEL);

    target_duty = (((uint32_t)duty_cycle)*(8191)) / 100;

    ledc_set_fade_with_time(PWM_MODE, PWM_CHANNEL, target_duty, 10);
    ledc_fade_start(PWM_MODE, PWM_CHANNEL, LEDC_FADE_NO_WAIT);
}
//------------------------------------------------------------------------------
static void update_pwm(uint8_t duty_cycle)
{
    uint32_t target_duty = 0;

    target_duty = (((uint32_t)duty_cycle)*(8191)) / 100;

    ledc_set_fade_with_time(PWM_MODE, PWM_CHANNEL, target_duty, 10);
    ledc_fade_start(PWM_MODE, PWM_CHANNEL, LEDC_FADE_NO_WAIT);
}
//------------------------------------------------------------------------------
static void turn_off_pwm(void)
{
    ledc_set_fade_with_time(PWM_MODE, PWM_CHANNEL, 0, 10);
    ledc_fade_start(PWM_MODE, PWM_CHANNEL, LEDC_FADE_NO_WAIT);
}
//------------------------------------------------------------------------------
static void pwm_manager_task(void* arg)
{
    pwm_event_t pwm_ev;   

    config_pwm_output();

    while(1)
    {
        if(xQueueReceive(pwm_manager_queue, &pwm_ev, 10000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            switch(pwm_ev.cmd)
            {
                case CMD_UNDEFINED:
                    break;
                case TURN_ON_PWM:
                    turn_on_pwm(pwm_ev.duty_cycle);
                    break; 
                case TURN_OFF_PWM:
                    turn_off_pwm();
                    break;
                case UPDATE_DUTY_CYCLE:
                    update_pwm(pwm_ev.duty_cycle);
                    break;
            }       
        }
        else
        {
 
        }
    }
}
//------------------- DEFINICION DE FUNCIONES EXTERNAS -------------------------
//------------------------------------------------------------------------------
void pwm_manager_init(void)
{
    pwm_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(pwm_event_t));

    xTaskCreate(pwm_manager_task, "pwm_manager_task", 
        configMINIMAL_STACK_SIZE*4, NULL, configMAX_PRIORITIES-2, NULL);
}
//------------------------------------------------------------------------------
void pwm_manager_turn_on_pwm(uint8_t pwm_power_percent)
{
    pwm_event_t ev;

    ev.cmd = TURN_ON_PWM;
    ev.duty_cycle = pwm_power_percent;

    xQueueSend(pwm_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void pwm_manager_turn_off_pwm(void)
{
    pwm_event_t ev;

    ev.cmd = TURN_OFF_PWM;

    xQueueSend(pwm_manager_queue, &ev, 10);
}
//------------------------------------------------------------------------------
void pwm_manager_update_pwm(uint8_t pwm_power_percent)
{
    pwm_event_t ev;

    ev.cmd = UPDATE_DUTY_CYCLE;
    ev.duty_cycle = pwm_power_percent;

    xQueueSend(pwm_manager_queue, &ev, 10);
}
//---------------------------- END OF FILE -------------------------------------
//------------------------------------------------------------------------------