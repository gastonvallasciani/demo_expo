#ifndef GLOBAL_MANAGER_H__
#define GLOBAL_MANAGER_H__
//------------------- INCLUDES -------------------------------------------------
//------------------------------------------------------------------------------
#include <stdint.h>
//------------------- MACROS Y DEFINES -----------------------------------------
//------------------------------------------------------------------------------

//------------------- TYPEDEF --------------------------------------------------
//------------------------------------------------------------------------------
typedef enum{
    RELE_VEGE_ENABLE = 0,
    RELE_VEGE_DISABLE = 1,
}rele_output_status_t;

typedef enum{
    INCREASING = 0,
    DECREASING = 1,
}pwm_update_orientation_t;

typedef struct{
    uint8_t output_pwm_update_time;
    uint8_t duty_cycle;
    uint8_t duty_cycle_step;
    pwm_update_orientation_t update_orientation;
}pwm_output_info_t;

typedef struct{
    uint8_t rele_vege_start_cycle_time;
    uint8_t rele_vege_cycle_time_width;
    bool enable_rele_vege_cycle;
}rele_vege_info_t;

typedef struct{
    rele_vege_info_t rele_vege_info;
    pwm_output_info_t pwm_output_info;
    uint8_t update_output_counter;
}global_info_t;
//------------------- DECLARACION DE DATOS EXTERNOS ----------------------------
//------------------------------------------------------------------------------

//------------------- DECLARACION DE FUNCIONES EXTERNAS ------------------------
//------------------------------------------------------------------------------
void global_manager_init(void);

void global_manager_set_rele_vege_status_off(void);
void global_manager_set_rele_vege_status_on(void);
void global_manager_set_pwm_power_value_manual(uint8_t output_pwm_change_time);
//------------------- FIN DEL ARCHIVO ------------------------------------------
#endif /* GLOBAL_MANAGER_H__ */