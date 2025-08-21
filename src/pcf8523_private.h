/**
 * @file pcf8523_private.h
 * @brief Internal functions and private definitions for the PCF8523 sensor driver.
 *
 * This file contains internal implementations and macros intended for
 * use within the PCF8523 driver only. It is not meant for direct use by
 * application code.
 *
 * Users should not include this file outside of the driver context.
 *
 * @author ljn0099
 *
 * @license MIT License
 * Copyright (c) 2025 ljn0099
 *
 * See LICENSE file for details.
 */
#ifndef PCF8523_PRIVATE_H
#define PCF8523_PRIVATE_H

#include "hardware/i2c.h"
#include "sensor/pcf8523.h"

#define PCF8523_RESET_COMMAND 0x58

#define PCF8523_OFFSET_REG 0x0E
#define PCF8523_OFFSET_MODE_MASK (1 << 7)

#define PCF8523_TMR_CTRL_REG 0x0F
#define PCF8523_TMR_A_FREQ_CTRL_REG 0x10
#define PCF8523_TMR_A_REF 0x11
#define PCF8523_TMR_B_FREQ_CTRL_REG 0x12
#define PCF8523_TMR_B_REG 0x13

#define PCF8523_SECONDS_OS_MASK (1 << 7)
#define PCF8523_HOUR_PM_MASK (1 << 5)
#define PCF8523_DISABLE_ALARM_MASK (1 << 7)

#define PCF8523_CTRL3_POWER_MODE_MASK 0b11100000 // PM

#define PCF8523_TMR_B_INT_WIDTH_MASK 0b01110000

#define PCF8523_CTRL2_FLAG_MASK 0b11111000

#define PCF8523_CTRL1_CAP_SEL_MASK (1 << 7)   // CAP_SEL
#define PCF8523_CTRL1_STOP_MASK (1 << 5)      // STOP
#define PCF8523_CTRL1_RESET_MASK (1 << 4)     // SR
#define PCF8523_CTRL1_HOUR_MODE_MASK (1 << 3) // 12_24

typedef enum {
    PCF8523_TMR_CTRL_PERM_TMR_A_TMR_SEC_INT_MASK = (1 << 7),
    PCF8523_TMR_CTRL_PERM_TMR_B_INT_MASK = (1 << 6),
    PCF8523_TMR_CTRL_CLKOUT_FREQ_MASK = 0b00111000,
    PCF8523_TMR_CTRL_TMR_A_MODE_MASK = 0b00000110,
    PCF8523_TMR_CTRL_TMR_B_ENABLED_MASK = (1 << 0)
} pcf8523_TmrCtrlMask_t;

typedef enum {
    PCF8523_SEC = 0,
    PCF8523_MIN,
    PCF8523_HOUR,
    PCF8523_DAY,
    PCF8523_WEEKDAY,
    PCF8523_MONTH,
    PCF8523_YEAR
} pcf8523_DatetimeIndex_t;

typedef enum {
    PCF8523_MIN_ALARM = 0,
    PCF8523_HOUR_ALARM,
    PCF8523_DAY_ALARM,
    PCF8523_WEEKDAY_ALARM,
} pcf8523_AlarmIndex_t;


bool pcf8523_write_register(pcf8523_t *pcf8523, uint8_t reg, uint8_t data);
bool pcf8523_read_register(pcf8523_t *pcf8523, uint8_t reg, uint8_t *data);

bool pcf8523_write_block(pcf8523_t *pcf8523, uint8_t startReg, uint8_t *data, size_t len);
bool pcf8523_read_block(pcf8523_t *pcf8523, uint8_t startReg, uint8_t *data, size_t len);

bool pcf8523_set_bit(pcf8523_t *pcf8523, uint8_t reg, uint8_t mask, bool value);
bool pcf8523_read_bit(pcf8523_t *pcf8523, uint8_t reg, uint8_t mask, bool *value);

pcf8523_HourMode_t pcf8523_extract_hour_mode(uint8_t *hourRaw, bool pcf8523Format24h);

static inline uint8_t pcf8523_decimal_to_bcd(uint8_t decimal);
static inline uint8_t pcf8523_bcd_to_decimal(uint8_t bcd);

bool pcf8523_validate_datetime(pcf8523_Datetime_t *datetime, bool pcf8523Format24h);
bool pcf8523_validate_alarm(pcf8523_Alarm_t *alarm, bool pcf8523Format24h);

bool pcf8523_validate_time_field(uint8_t reg, uint8_t value, pcf8523_HourMode_t *hourMode,
                                 bool pcf8523Format24h);

static inline bool pcf8523_validate_sec(uint8_t sec);
static inline bool pcf8523_validate_min(uint8_t min);
static inline bool pcf8523_validate_day(uint8_t day);
static inline bool pcf8523_validate_weekday(uint8_t weekDay);
static inline bool pcf8523_validate_month(uint8_t month);
static inline bool pcf8523_validate_year(uint8_t year);
static inline bool pcf8523_validate_hour(uint8_t hour, pcf8523_HourMode_t hourMode,
                                         bool pcf8523Format24h);
#endif
