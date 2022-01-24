/**
****************************************************************************
* @file      tuya_com_calendar.h
* @brief     tuya_com_calendar
* @author    xiaojian
* @version   V0.0.1
* @date      2021-10
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_COM_CALENDAR_H__
#define __TUYA_COM_CALENDAR_H__
#include <stdint.h>
#include "stdbool.h"
#include <stdio.h>
#include "tuya_type.h"

typedef enum
{
    TUYA_MON = 0x01,
    TUYA_TUE = 0x02,
    TUYA_WED = 0x03,
    TUYA_THU = 0x04,
    TUYA_FRI = 0x05,
    TUYA_SAT = 0x06,
    TUYA_SUN = 0x07
}tuya_com_week_t;

typedef uint32_t (*com_timestamp_get_cbk)(void);
typedef void (*com_timestamp_set_cbk)(uint32_t timestamp);

typedef struct clock
{
	uint8_t second;
	uint8_t miniute;
	uint8_t hour;
	uint8_t week;
	uint8_t day;
	uint8_t month;
	uint32_t year;
}tuya_com_time_t;

void tuya_com_time_set(tuya_com_time_t value);

tuya_com_time_t tuya_com_time_get(void);

uint32_t tuya_com_timestamp_get(void);

void tuya_com_timestamp_set(uint32_t timestamp);

void tuya_com_utc_time_convert(tuya_com_time_t *tm, uint32_t sec_time);

void tuya_com_calendar_init(com_timestamp_set_cbk timestamp_set_cbk, com_timestamp_get_cbk timestamp_get_cbk);

#endif
