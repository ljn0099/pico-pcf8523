/**
 * @file pcf8523.h
 * @brief PCF8523 I2C Library for the Raspberry Pi pico-sdk
 *
 * @author ljn0099
 *
 * @license MIT License
 * Copyright (c) 2025 ljn0099
 *
 * See LICENSE file for details.
 */
#ifndef PCF8523_H
#define PCF8523_H

#include "hardware/i2c.h"

#define PCF8523_DEFAULT_ADDR 0x68

typedef enum {
    PCF8523_CTRL1_REG = 0x00,
    PCF8523_CTRL2_REG = 0x01,
    PCF8523_CTRL3_REG = 0x02
} pcf8523_CtrlReg_t;

typedef enum {
    PCF8523_SECONDS_REG = 0x03,
    PCF8523_MINUTES_REG = 0x04,
    PCF8523_HOURS_REG = 0x05,
    PCF8523_DAYS_REG = 0x06,
    PCF8523_WEEKDAYS_REG = 0x07,
    PCF8523_MONTHS_REG = 0x08,
    PCF8523_YEARS_REG = 0x09
} pcf8523_DatetimeReg_t;

typedef enum {
    PCF8523_MINUTES_ALARM_REG = 0x0A,
    PCF8523_HOURS_ALARM_REG = 0x0B,
    PCF8523_DAYS_ALARM_REG = 0x0C,
    PCF8523_WEEKDAYS_ALARM_REG = 0x0D
} pcf8523_AlarmReg_t;

typedef enum {
    PCF8523_OFFSET_EVERY_2_HOURS = false,
    PCF8523_OFFSET_EVERY_MIN = true
} pcf8523_OffsetMode_t;

typedef enum { PCF8523_TMR_A_TMR_SEC = 0, PCF8523_TMR_B } pcf8523_Tmr_t;

typedef enum { PCF8523_TMR_PERM_INT = false, PCF8523_TMR_PULSED_INT = true } pcf8523_TmrIntMode;

typedef enum {
    PCF8523_TMR_A_DISABLED = (0 << 1),
    PCF8523_TMR_A_DISABLED_ALT = (0 << 1),
    PCF8523_TMR_A_COUNTDOWN = (1 << 1),
    PCF8523_TMR_A_WATCHDOG = (2 << 1)
} pcf8523_TmrAMode_t;

typedef enum {
    PCF8523_CLK_OUT_FREQ_32768_HZ = (0 << 3),
    PCF8523_CLK_OUT_FREQ_16384_HZ = (1 << 3),
    PCF8523_CLK_OUT_FREQ_8192_HZ = (2 << 3),
    PCF8523_CLK_OUT_FREQ_4096_HZ = (3 << 3),
    PCF8523_CLK_OUT_FREQ_1024_HZ = (4 << 3),
    PCF8523_CLK_OUT_FREQ_32_HZ = (5 << 3),
    PCF8523_CLK_OUT_FREQ_1_HZ = (6 << 3),
    PCF8523_CLK_OUT_FREQ_DISABLED = (7 << 3)
} pcf8523_ClkOutFreq_t;

typedef enum {
    PCF8523_CLK_SOURCE_FREQ_4096_HZ = 0,             // Period 244us aprox
    PCF8523_CLK_SOURCE_FREQ_64_HZ = 1,               // Period 15.625ms aprox
    PCF8523_CLK_SOURCE_FREQ_1_HZ = 2,                // Period 1s
    PCF8523_CLK_SOURCE_FREQ_1_DIV_60_HZ = 3,         // Period 1min
    PCF8523_CLK_SOURCE_FREQ_1_DIV_3600_HZ = 4,       // Period 1h
    PCF8523_CLK_SOURCE_FREQ_1_DIV_3600_HZ_ALT_1 = 5, // Period 1h
    PCF8523_CLK_SOURCE_FREQ_1_DIV_3600_HZ_ALT_2 = 6  // Period 1h
} pcf8523_ClkSourceFreq_t;

typedef enum {
    PCF8523_TMR_B_INT_WIDTH_46_875_MS = (0 << 4),
    PCF8523_TMR_B_INT_WIDTH_62_500_MS = (1 << 4),
    PCF8523_TMR_B_INT_WIDTH_78_125_MS = (2 << 4),
    PCF8523_TMR_B_INT_WIDTH_93_750_MS = (3 << 4),
    PCF8523_TMR_B_INT_WIDTH_125_000_MS = (4 << 4),
    PCF8523_TMR_B_INT_WIDTH_156_250_MS = (5 << 4),
    PCF8523_TMR_B_INT_WIDTH_187_500_MS = (6 << 4),
    PCF8523_TMR_B_INT_WIDTH_218_750_MS = (7 << 4)
} pcf8523_TmrBIntWidth_t;

typedef struct {
    pcf8523_ClkSourceFreq_t sourceFreq;
    uint8_t value;
} pcf8523_TimerAValue;

typedef struct {
    pcf8523_ClkSourceFreq_t sourceFreq;
    pcf8523_TmrBIntWidth_t intWidth;
    uint8_t value;
} pcf8523_TimerBValue;

typedef enum {
    PCF8523_CTRL1_ENABLE_SECOND_INT_MASK = (1 << 2),           // SIE
    PCF8523_CTRL1_ENABLE_ALARM_INT_MASK = (1 << 1),            // AIE
    PCF8523_CTRL1_ENABLE_CORRETION_INT_MASK = (1 << 0),        // CIE
    PCF8523_CTRL2_ENABLE_WATCHDOG_TMR_A_INT_MASK = (1 << 2),   // WTAIE
    PCF8523_CTRL2_ENABLE_COUNTDOWN_TMR_A_INT_MASK = (1 << 1),  // CTAIE
    PCF8523_CTRL2_ENABLE_COUNTDOWN_TMR_B_INT_MASK = (1 << 0),  // CTBIE
    PCF8523_CTRL3_ENABLE_BATT_SWITCH_OVER_INT_MASK = (1 << 1), // BSIE
    PCF8523_CTRL3_ENABLE_BATT_STATUS_INT_MASK = (1 << 0)       // BLIE
} pcf8523_InterruptSource_t;

typedef enum {
    PCF8523_CTRL2_WATCHDOG_TMR_A_INT_FLAG_MASK_RO = (1 << 7), // WTAF
    PCF8523_CTRL2_COUNTDOWN_TMR_A_INT_FLAG_MASK = (1 << 6),   // CTAF
    PCF8523_CTRL2_COUNTDOWN_TMR_B_INT_FLAG_MASK = (1 << 5),   // CTBF
    PCF8523_CTRL2_SECOND_INT_FLAG_MASK = (1 << 4),            // SF
    PCF8523_CTRL2_ALARM_INT_FLAG_MASK = (1 << 3),             // AF
    PCF8523_CTRL3_BATT_SWITCH_OVER_INT_FLAG_MASK = (1 << 3),  // BSF
    PCF8523_CTRL3_BATT_STATUS_INT_FLAG_MASK_RO = (1 << 2)     // BLF
} pcf8523_InterruptFlag_t;

// They are shifted 5 positions to be in place when written to the register
typedef enum {
    PCF8523_PWR_SWITCH_OVER_STANDARD_LOW_DETECT_ENABLED = (0 << 5),
    PCF8523_PWR_SWITCH_OVER_DIRECT_LOW_DETECT_ENABLED = (1 << 5),
    PCF8523_PWR_SWITCH_OVER_DISABLED_LOW_DETECT_ENABLED = (2 << 5),
    PCF8523_PWR_SWITCH_OVER_DISABLED_LOW_DETECT_ENABLED_ALT = (3 << 5),
    PCF8523_PWR_SWITCH_OVER_STANDARD_LOW_DETECT_DISABLED = (4 << 5),
    PCF8523_PWR_SWITCH_OVER_DIRECT_LOW_DETECT_DISABLED = (5 << 5),
    PCF8523_PWR_SWITCH_OVER_DISABLED_LOW_DETECT_DISABLED = (7 << 5)
} pcf8523_PowerModes_t;

typedef enum {
    PCF8523_HOUR_MODE_24H = 0,
    PCF8523_HOUR_MODE_AM,
    PCF8523_HOUR_MODE_PM
} pcf8523_HourMode_t;

typedef enum {
    PCF8523_7PF_CAPACITOR = false,
    PCF8523_12_5PF_CAPACITOR = true
} pcf8523_CapacitorValue_t;

typedef struct {
    bool enableMinAlarm;
    uint8_t minAlarm;

    bool enableHourAlarm;
    pcf8523_HourMode_t hourMode;
    uint8_t hourAlarm;

    bool enableDayAlarm;
    uint8_t dayAlarm;

    bool enableWeekDayAlarm;
    uint8_t weekDayAlarm;
} pcf8523_Alarm_t;

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    pcf8523_HourMode_t hourMode;
    uint8_t day;
    uint8_t weekDay;
    uint8_t month;
    uint8_t year;
} pcf8523_Datetime_t;

typedef struct {
    i2c_inst_t *i2c;
    uint8_t i2cAddress;
    bool format24h;
} pcf8523_t;

bool pcf8523_init_struct(pcf8523_t *pcf8523, i2c_inst_t *i2c, uint8_t i2cAddress, bool is24hFormat,
                         bool checkFormat);

bool pcf8523_soft_reset(pcf8523_t *pcf8523);

bool pcf8523_read_datetime(pcf8523_t *pcf8523, pcf8523_Datetime_t *datetime);

uint64_t pcf8523_datetime_to_epoch(const pcf8523_Datetime_t *dt, uint16_t century);

bool pcf8523_read_datetime_field(pcf8523_t *pcf8523, pcf8523_DatetimeReg_t reg, uint8_t *value,
                                 pcf8523_HourMode_t *hourMode);

bool pcf8523_set_datetime(pcf8523_t *pcf8523, pcf8523_Datetime_t *datetime);

bool pcf8523_set_datetime_field(pcf8523_t *pcf8523, pcf8523_DatetimeReg_t reg, uint8_t value,
                                pcf8523_HourMode_t *hourMode);

bool pcf8523_read_alarm(pcf8523_t *pcf8523, pcf8523_Alarm_t *alarm);

bool pcf8523_read_alarm_field(pcf8523_t *pcf8523, pcf8523_AlarmReg_t reg, uint8_t *value,
                              bool *enabled, pcf8523_HourMode_t *hourMode);

bool pcf8523_set_alarm(pcf8523_t *pcf8523, pcf8523_Alarm_t *alarm);

bool pcf8523_set_alarm_field(pcf8523_t *pcf8523, pcf8523_AlarmReg_t reg, uint8_t value, bool enable,
                             pcf8523_HourMode_t *hourMode);

bool pcf8523_set_power_mode(pcf8523_t *pcf8523, pcf8523_PowerModes_t powerMode);

bool pcf8523_read_power_mode(pcf8523_t *pcf8523, pcf8523_PowerModes_t *powerMode);

bool pcf8523_set_hour_mode(pcf8523_t *pcf8523, bool set12hMode);

bool pcf8523_read_hour_mode(pcf8523_t *pcf8523, bool *is12hMode);

bool pcf8523_set_oscilator_capacitor_value(pcf8523_t *pcf8523, pcf8523_CapacitorValue_t capValue);

bool pcf8523_read_oscilator_capacitor_value(pcf8523_t *pcf8523, pcf8523_CapacitorValue_t *capValue);

bool pcf8523_clear_os_integrity_flag(pcf8523_t *pcf8523);

bool pcf8523_enable_interrupt_source(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                     pcf8523_InterruptSource_t pinMask, bool enable);

bool pcf8523_is_interrupt_source_enabled(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                         pcf8523_InterruptSource_t pinMask, bool *enabled);

bool pcf8523_read_interrupt_flag(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                 pcf8523_InterruptFlag_t pinMask, bool *enabled);

bool pcf8523_clear_interrupt_flag(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                  pcf8523_InterruptFlag_t pinMask);

bool pcf8523_freeze_time(pcf8523_t *pcf8523, bool freeze);

bool pcf8523_is_time_frozen(pcf8523_t *pcf8523, bool *frozen);

bool pcf8523_set_offset(pcf8523_t *pcf8523, pcf8523_OffsetMode_t mode, int8_t offset);

bool pcf8523_read_offset(pcf8523_t *pcf8523, pcf8523_OffsetMode_t *mode, int8_t *offset);

bool pcf8523_set_timer_a_mode(pcf8523_t *pcf8523, pcf8523_TmrAMode_t mode);

bool pcf8523_read_timer_a_mode(pcf8523_t *pcf8523, pcf8523_TmrAMode_t *mode);

bool pcf8523_set_timer_b_mode(pcf8523_t *pcf8523, bool enable);

bool pcf8523_read_timer_b_mode(pcf8523_t *pcf8523, bool *enabled);

bool pcf8523_set_timer_int_mode(pcf8523_t *pcf8523, pcf8523_Tmr_t tmr, pcf8523_TmrIntMode intMode);

bool pcf8523_read_timer_int_mode(pcf8523_t *pcf8523, pcf8523_Tmr_t tmr,
                                 pcf8523_TmrIntMode *intMode);

bool pcf8523_set_timer_a_duration(pcf8523_t *pcf8523, pcf8523_TimerAValue *tmrA);

bool pcf8523_read_timer_a_duration(pcf8523_t *pcf8523, pcf8523_TimerAValue *tmrA);

bool pcf8523_set_timer_b_duration(pcf8523_t *pcf8523, pcf8523_TimerBValue *tmrB);

bool pcf8523_read_timer_b_duration(pcf8523_t *pcf8523, pcf8523_TimerBValue *tmrB);

bool pcf8523_set_clk_out_mode(pcf8523_t *pcf8523, pcf8523_ClkOutFreq_t clkOutFreq);

bool pcf8523_read_clk_out_mode(pcf8523_t *pcf8523, pcf8523_ClkSourceFreq_t *clkOutFreq);
#endif
