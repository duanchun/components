
/**
****************************************************************************
* @file      tuya_com_433.h
* @brief     tuya_com_433
* @author    xiaojian
* @version   V0.0.1
* @date      2021-11
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __TUYA_COM_433_H_
#define __TUYA_COM_433_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tuya_ble_stdlib.h"

typedef enum {
    TUYA_433_CODE_BIT_24  = 24,
    TUYA_433_CODE_BIT_28  = 28,
}TUYA_433_CODE_BITS_TYPE_E;

typedef enum {
    RCU_WORK_SHUTDOWN,
    RCU_WORK_RUNING,
}RCU_WORK_MODE_E;

void tuya_com_433_init(uint32_t data_pin, uint32_t shut_pin);

void tuya_com_433_start(void);

void tuya_com_433_stop(void);

void tuya_com_433_resume(void);

void tuya_com_433_pause(void);

void tuya_com_433_set_code_bits(uint8_t bits);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
