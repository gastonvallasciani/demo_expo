// NOTAS: No se puede printear ni loguear desde una ISR se reinicia el ESP32.
//--------------------INCLUDES--------------------------------------------------
//------------------------------------------------------------------------------
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "esp_timer.h"

#include "../include/button_manager.h"
#include "../include/board_def.h"
#include "../include/analog_input_manager.h"
#include "../include/global_manager.h"
//--------------------MACROS Y DEFINES------------------------------------------
//------------------------------------------------------------------------------
#define QUEUE_ELEMENT_QUANTITY 20

#define TIEMPO_ANTIRREBOTE_MS 50
#define TIEMPO_PULSADO_MS 3000

//#define DEBUG_MODULE
//TO DO: ver el tema del button wifi y los led wifi
//------------------------------TYPEDEF-----------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    CMD_UNDEFINED,
    VEGE_BUTTON_PUSHED,
}cmds_t;

typedef struct{
    cmds_t cmd;
}button_events_t;
//--------------------DECLARACION DE DATOS INTERNOS-----------------------------
//------------------------------------------------------------------------------
static QueueHandle_t button_manager_queue;

volatile int64_t start_time = 0;
volatile int64_t start_time_vege = 0;
//--------------------DECLARACION DE FUNCIONES INTERNAS-------------------------
//------------------------------------------------------------------------------
static void button_event_manager_task(void * pvParameters);

static void config_buttons_isr(void);

static void vege_button_interrupt(void *arg);
//--------------------DEFINICION DE DATOS INTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE DATOS EXTERNOS------------------------------
//------------------------------------------------------------------------------

//--------------------DEFINICION DE FUNCIONES INTERNAS--------------------------
//------------------------------------------------------------------------------
static void config_buttons_isr(void)
{
    gpio_config_t config;

    config.pin_bit_mask = (1ULL << VEGE_BUTTON);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLDOWN_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&config);

    // Configurar la interrupción del botón
    gpio_install_isr_service(0);
    gpio_isr_handler_add(VEGE_BUTTON, vege_button_interrupt, NULL);
}
//------------------------------------------------------------------------------
static void IRAM_ATTR vege_button_interrupt(void *arg) 
{
    button_events_t ev;
    int64_t time_now = esp_timer_get_time();

    if(gpio_get_level(VEGE_BUTTON) == 0)
    {
        start_time_vege = time_now;
    }
    else 
    {
        if (start_time_vege != 0)
        {
            int64_t diff = time_now - start_time_vege;

            if (diff > 30000)  // 30ms seconds expressed in microseconds
            {
                ev.cmd = VEGE_BUTTON_PUSHED;
                xQueueSendFromISR(button_manager_queue, &ev, pdFALSE);
            }
            start_time_vege = 0;
        }
    }
}
//------------------------------------------------------------------------------
void button_event_manager_task(void * pvParameters)
{
    button_events_t button_ev;
    
    rele_output_status_t rele_vege_status = RELE_VEGE_DISABLE;

    config_buttons_isr();

    while(true)
    {
        if(xQueueReceive(button_manager_queue, &button_ev, portMAX_DELAY) == pdTRUE)
        {
            switch(button_ev.cmd)
            {
                case CMD_UNDEFINED:
                    break;
                case VEGE_BUTTON_PUSHED:
                    if(rele_vege_status == RELE_VEGE_DISABLE)
                    {
                        //global_manager_set_rele_vege_status_off();
                        rele_vege_status = RELE_VEGE_ENABLE;
                    }
                    else if(rele_vege_status == RELE_VEGE_ENABLE)
                    {
                        //global_manager_set_rele_vege_status_on();
                        rele_vege_status = RELE_VEGE_DISABLE;
                    }
                    break;
                default:
                break;
            }
        }
    }
}
//--------------------DEFINICION DE FUNCIONES EXTERNAS--------------------------
//------------------------------------------------------------------------------
void button_manager_init(void)
{
    button_manager_queue = xQueueCreate(QUEUE_ELEMENT_QUANTITY, sizeof(button_events_t));

    xTaskCreate(button_event_manager_task, "button_event_manager_task", 
               configMINIMAL_STACK_SIZE*5, NULL, configMAX_PRIORITIES, NULL);             
}
//--------------------FIN DEL ARCHIVO-------------------------------------------
//------------------------------------------------------------------------------
