#include "hardware/i2c.h"
#include "pcf8523_private.h"
#include "pico/stdlib.h"
#include "sensor/pcf8523.h"
#include <string.h>

bool pcf8523_init_struct(pcf8523_t *pcf8523, i2c_inst_t *i2c, uint8_t i2cAddress, bool is24hFormat,
                         bool checkFormat) {
    if (!pcf8523 || !i2c)
        return false;

    pcf8523->i2c = i2c;
    pcf8523->i2cAddress = i2cAddress;
    if (checkFormat) {
        bool is12hModeNow;
        if (!pcf8523_read_hour_mode(pcf8523, &is12hModeNow))
            return false;
        pcf8523->format24h = !is12hModeNow;
    }
    else {
        pcf8523->format24h = is24hFormat;
    }

    return true;
}

bool pcf8523_write_register(pcf8523_t *pcf8523, uint8_t reg, uint8_t data) {
    if (!pcf8523)
        return false;

    uint8_t buffer[2] = {reg, data};
    if (i2c_write_blocking(pcf8523->i2c, pcf8523->i2cAddress, buffer, 2, false) != 2)
        return false;

    return true;
}

bool pcf8523_read_register(pcf8523_t *pcf8523, uint8_t reg, uint8_t *data) {
    if (!pcf8523 || !data)
        return false;

    uint8_t buffer;

    if (i2c_write_blocking(pcf8523->i2c, pcf8523->i2cAddress, &reg, 1, true) != 1)
        return false;

    if (i2c_read_blocking(pcf8523->i2c, pcf8523->i2cAddress, &buffer, 1, false) != 1)
        return false;

    *data = buffer;

    return true;
}

bool pcf8523_write_block(pcf8523_t *pcf8523, uint8_t startReg, uint8_t *data, size_t len) {
    if (!pcf8523 || !data)
        return false;

    uint8_t buffer[len + 1];
    buffer[0] = startReg;
    memcpy(&buffer[1], data, len);

    if (i2c_write_blocking(pcf8523->i2c, pcf8523->i2cAddress, buffer, len + 1, false) != len + 1)
        return false;

    return true;
}

bool pcf8523_read_block(pcf8523_t *pcf8523, uint8_t startReg, uint8_t *data, size_t len) {
    if (!pcf8523 || !data)
        return false;

    uint8_t buffer[len];

    if (i2c_write_blocking(pcf8523->i2c, pcf8523->i2cAddress, &startReg, 1, true) != 1)
        return false;

    if (i2c_read_blocking(pcf8523->i2c, pcf8523->i2cAddress, buffer, len, false) != len)
        return false;

    memcpy(data, buffer, len);

    return true;
}

bool pcf8523_set_bit(pcf8523_t *pcf8523, uint8_t reg, uint8_t mask, bool value) {
    if (!pcf8523)
        return false;

    uint8_t buffer;
    if (!pcf8523_read_register(pcf8523, reg, &buffer))
        return false;

    if (reg == PCF8523_CTRL2_REG)
        buffer |= PCF8523_CTRL2_FLAG_MASK;

    if (value)
        buffer |= mask;
    else
        buffer &= ~mask;

    if (!pcf8523_write_register(pcf8523, reg, buffer))
        return false;

    return true;
}

bool pcf8523_read_bit(pcf8523_t *pcf8523, uint8_t reg, uint8_t mask, bool *value) {
    if (!pcf8523 || !value)
        return false;

    uint8_t buffer;
    if (!pcf8523_read_register(pcf8523, reg, &buffer))
        return false;

    if (buffer & mask)
        *value = true;
    else
        *value = false;

    return true;
}

bool pcf8523_soft_reset(pcf8523_t *pcf8523) {
    if (!pcf8523)
        return false;

    return pcf8523_write_register(pcf8523, PCF8523_CTRL1_REG, PCF8523_RESET_COMMAND);
}

pcf8523_HourMode_t pcf8523_extract_hour_mode(uint8_t *hourRaw, bool pcf8523Format24h) {
    if (!hourRaw)
        return 0;

    if (pcf8523Format24h) {
        return PCF8523_HOUR_MODE_24H;
    }
    else {
        if (*hourRaw & PCF8523_HOUR_PM_MASK) { // It's PM
            *hourRaw &= ~PCF8523_HOUR_PM_MASK; // Delete the PM bit
            return PCF8523_HOUR_MODE_PM;
        }
        else {
            return PCF8523_HOUR_MODE_AM;
        }
    }
}

bool pcf8523_read_datetime(pcf8523_t *pcf8523, pcf8523_Datetime_t *datetime) {
    if (!pcf8523 || !datetime)
        return false;

    uint8_t buffer[7];
    if (!pcf8523_read_block(pcf8523, PCF8523_SECONDS_REG, buffer, 7))
        return false;

    if (buffer[PCF8523_SEC] & PCF8523_SECONDS_OS_MASK)
        // The bit 7 is 1 indicating that the clock integrity is not guaranteed
        return false;

    datetime->hourMode = pcf8523_extract_hour_mode(&buffer[PCF8523_HOUR], pcf8523->format24h);

    datetime->sec = pcf8523_bcd_to_decimal(buffer[PCF8523_SEC]);
    datetime->min = pcf8523_bcd_to_decimal(buffer[PCF8523_MIN]);
    datetime->hour = pcf8523_bcd_to_decimal(buffer[PCF8523_HOUR]);
    datetime->day = pcf8523_bcd_to_decimal(buffer[PCF8523_DAY]);
    datetime->weekDay = buffer[PCF8523_WEEKDAY];
    datetime->month = pcf8523_bcd_to_decimal(buffer[PCF8523_MONTH]);
    datetime->year = pcf8523_bcd_to_decimal(buffer[PCF8523_YEAR]);

    return true;
}

bool pcf8523_read_datetime_field(pcf8523_t *pcf8523, pcf8523_DatetimeReg_t reg, uint8_t *value,
                                 pcf8523_HourMode_t *hourMode) {
    if (!pcf8523)
        return false;

    uint8_t buffer;
    if (!pcf8523_read_register(pcf8523, reg, &buffer))
        return false;

    if (reg == PCF8523_SECONDS_REG && (buffer & PCF8523_SECONDS_OS_MASK))
        // The bit 7 is 1 indicating that the clock integrity is not guaranteed
        return false;

    if (reg == PCF8523_HOURS_REG && !hourMode)
        return false;

    if (reg == PCF8523_HOURS_REG)
        *hourMode = pcf8523_extract_hour_mode(&buffer, pcf8523->format24h);

    if (reg != PCF8523_WEEKDAYS_REG)
        buffer = pcf8523_bcd_to_decimal(buffer);

    if (value)
        *value = buffer;

    return true;
}

bool pcf8523_set_datetime(pcf8523_t *pcf8523, pcf8523_Datetime_t *datetime) {
    if (!pcf8523 || !datetime)
        return false;

    uint8_t buffer[7];

    if (!pcf8523_validate_datetime(datetime, pcf8523->format24h))
        return false;

    buffer[PCF8523_SEC] = pcf8523_decimal_to_bcd(datetime->sec);
    buffer[PCF8523_MIN] = pcf8523_decimal_to_bcd(datetime->min);
    buffer[PCF8523_HOUR] = pcf8523_decimal_to_bcd(datetime->hour);
    buffer[PCF8523_DAY] = pcf8523_decimal_to_bcd(datetime->day);
    buffer[PCF8523_WEEKDAY] = datetime->weekDay;
    buffer[PCF8523_MONTH] = pcf8523_decimal_to_bcd(datetime->month);
    buffer[PCF8523_YEAR] = pcf8523_decimal_to_bcd(datetime->year);

    if (datetime->hourMode == PCF8523_HOUR_MODE_PM)
        buffer[PCF8523_HOUR] |= PCF8523_HOUR_PM_MASK;

    if (!pcf8523_write_block(pcf8523, PCF8523_SECONDS_REG, buffer, 7))
        return false;

    return true;
}

bool pcf8523_set_datetime_field(pcf8523_t *pcf8523, pcf8523_DatetimeReg_t reg, uint8_t value,
                                pcf8523_HourMode_t *hourMode) {
    if (!pcf8523)
        return false;

    if (reg == PCF8523_HOURS_REG && !hourMode)
        return false;

    if (!pcf8523_validate_time_field(reg, value, hourMode, pcf8523->format24h))
        return false;

    if (reg != PCF8523_WEEKDAYS_REG)
        value = pcf8523_decimal_to_bcd(value);

    if (*hourMode == PCF8523_HOUR_MODE_PM && reg == PCF8523_HOURS_REG)
        value |= PCF8523_HOUR_PM_MASK;

    if (!pcf8523_write_register(pcf8523, reg, value))
        return false;

    return true;
}

bool pcf8523_read_alarm(pcf8523_t *pcf8523, pcf8523_Alarm_t *alarm) {
    if (!pcf8523 || !alarm)
        return false;

    uint8_t buffer[4];

    if (!pcf8523_read_block(pcf8523, PCF8523_MINUTES_ALARM_REG, buffer, 4))
        return false;

    alarm->hourMode = pcf8523_extract_hour_mode(&buffer[PCF8523_HOUR_ALARM], pcf8523->format24h);

    alarm->enableMinAlarm = true;
    alarm->enableHourAlarm = true;
    alarm->enableDayAlarm = true;
    alarm->enableWeekDayAlarm = true;

    if (buffer[PCF8523_MIN_ALARM] & PCF8523_DISABLE_ALARM_MASK) {
        alarm->enableMinAlarm = false;
        buffer[PCF8523_MIN_ALARM] &= ~PCF8523_DISABLE_ALARM_MASK;
    }
    if (buffer[PCF8523_HOUR_ALARM] & PCF8523_DISABLE_ALARM_MASK) {
        alarm->enableHourAlarm = false;
        buffer[PCF8523_HOUR_ALARM] &= ~PCF8523_DISABLE_ALARM_MASK;
    }
    if (buffer[PCF8523_DAY_ALARM] & PCF8523_DISABLE_ALARM_MASK) {
        alarm->enableDayAlarm = false;
        buffer[PCF8523_DAY_ALARM] &= ~PCF8523_DISABLE_ALARM_MASK;
    }
    if (buffer[PCF8523_WEEKDAY_ALARM] & PCF8523_DISABLE_ALARM_MASK) {
        alarm->enableWeekDayAlarm = false;
        buffer[PCF8523_WEEKDAY_ALARM] &= ~PCF8523_DISABLE_ALARM_MASK;
    }

    alarm->minAlarm = pcf8523_bcd_to_decimal(buffer[PCF8523_MIN_ALARM]);
    alarm->hourAlarm = pcf8523_bcd_to_decimal(buffer[PCF8523_HOUR_ALARM]);
    alarm->dayAlarm = pcf8523_bcd_to_decimal(buffer[PCF8523_DAY_ALARM]);
    alarm->weekDayAlarm = buffer[PCF8523_WEEKDAY_ALARM];

    return true;
}

bool pcf8523_read_alarm_field(pcf8523_t *pcf8523, pcf8523_AlarmReg_t reg, uint8_t *value,
                              bool *enabled, pcf8523_HourMode_t *hourMode) {
    if (!pcf8523)
        return false;

    if (reg == PCF8523_HOURS_ALARM_REG && !hourMode)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, reg, &buffer))
        return false;

    if (reg == PCF8523_HOURS_ALARM_REG)
        *hourMode = pcf8523_extract_hour_mode(&buffer, pcf8523->format24h);

    if (buffer & PCF8523_DISABLE_ALARM_MASK) {
        if (enabled)
            *enabled = false;
        buffer &= ~PCF8523_DISABLE_ALARM_MASK;
    }
    else if (enabled) {
        *enabled = true;
    }

    if (reg != PCF8523_WEEKDAY_ALARM && value)
        *value = pcf8523_bcd_to_decimal(buffer);

    return true;
}

bool pcf8523_set_alarm(pcf8523_t *pcf8523, pcf8523_Alarm_t *alarm) {
    if (!pcf8523 || !alarm)
        return false;

    if (!pcf8523_validate_alarm(alarm, pcf8523->format24h))
        return false;

    uint8_t buffer[4];

    buffer[PCF8523_MIN_ALARM] = pcf8523_decimal_to_bcd(alarm->minAlarm);
    buffer[PCF8523_HOUR_ALARM] = pcf8523_decimal_to_bcd(alarm->hourAlarm);
    buffer[PCF8523_DAY_ALARM] = pcf8523_decimal_to_bcd(alarm->dayAlarm);
    buffer[PCF8523_WEEKDAY_ALARM] = alarm->weekDayAlarm;

    if (!alarm->enableMinAlarm)
        buffer[PCF8523_MIN_ALARM] |= PCF8523_DISABLE_ALARM_MASK;
    if (!alarm->enableHourAlarm)
        buffer[PCF8523_HOUR_ALARM] |= PCF8523_DISABLE_ALARM_MASK;
    if (!alarm->enableDayAlarm)
        buffer[PCF8523_DAY_ALARM] |= PCF8523_DISABLE_ALARM_MASK;
    if (!alarm->enableWeekDayAlarm)
        buffer[PCF8523_WEEKDAY_ALARM] |= PCF8523_DISABLE_ALARM_MASK;

    if (alarm->hourMode == PCF8523_HOUR_MODE_PM)
        buffer[PCF8523_HOUR_ALARM] |= PCF8523_HOUR_PM_MASK;

    if (!pcf8523_write_block(pcf8523, PCF8523_MINUTES_ALARM_REG, buffer, 4))
        return false;

    return true;
}

bool pcf8523_set_alarm_field(pcf8523_t *pcf8523, pcf8523_AlarmReg_t reg, uint8_t value, bool enable,
                             pcf8523_HourMode_t *hourMode) {
    if (!pcf8523)
        return false;

    if (!hourMode && reg == PCF8523_HOURS_ALARM_REG)
        return false;

    if (!pcf8523_validate_time_field(reg, value, hourMode, pcf8523->format24h))
        return false;

    if (reg != PCF8523_WEEKDAYS_ALARM_REG)
        value = pcf8523_decimal_to_bcd(value);

    if (reg == PCF8523_HOURS_ALARM_REG && *hourMode == PCF8523_HOUR_MODE_PM)
        value |= PCF8523_HOUR_PM_MASK;

    if (!enable)
        value |= PCF8523_DISABLE_ALARM_MASK;

    if (!pcf8523_write_register(pcf8523, reg, value))
        return false;

    return true;
}

bool pcf8523_set_power_mode(pcf8523_t *pcf8523, pcf8523_PowerModes_t powerMode) {
    if (!pcf8523)
        return false;

    uint8_t buffer;
    if (!pcf8523_read_register(pcf8523, PCF8523_CTRL3_REG, &buffer))
        return false;

    // Clear Power Mode Bits
    buffer &= ~PCF8523_CTRL3_POWER_MODE_MASK;
    // Set Power Mode Bits
    buffer |= powerMode;

    if (!pcf8523_write_register(pcf8523, PCF8523_CTRL3_REG, buffer))
        return false;

    return true;
}

bool pcf8523_read_power_mode(pcf8523_t *pcf8523, pcf8523_PowerModes_t *powerMode) {
    if (!pcf8523 || !powerMode)
        return false;

    uint8_t buffer;
    if (!pcf8523_read_register(pcf8523, PCF8523_CTRL3_REG, &buffer))
        return false;

    buffer = (buffer >> 5);

    *powerMode = (pcf8523_PowerModes_t)buffer;

    return true;
}

bool pcf8523_set_hour_mode(pcf8523_t *pcf8523, bool set12hMode) {
    if (!pcf8523)
        return false;

    if (!pcf8523_set_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_HOUR_MODE_MASK, set12hMode))
        return false;

    if (set12hMode)
        pcf8523->format24h = false;
    else
        pcf8523->format24h = true;

    return true;
}

bool pcf8523_read_hour_mode(pcf8523_t *pcf8523, bool *is12hMode) {
    if (!pcf8523 || !is12hMode)
        return false;

    if (!pcf8523_read_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_HOUR_MODE_MASK, is12hMode))
        return false;

    return true;
}

bool pcf8523_set_oscilator_capacitor_value(pcf8523_t *pcf8523, pcf8523_CapacitorValue_t capValue) {
    if (!pcf8523)
        return false;

    return pcf8523_set_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_CAP_SEL_MASK, capValue);
}

bool pcf8523_read_oscilator_capacitor_value(pcf8523_t *pcf8523,
                                            pcf8523_CapacitorValue_t *capValue) {
    if (!pcf8523 || !capValue)
        return false;

    return pcf8523_read_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_CAP_SEL_MASK,
                            (bool *)capValue);
}

bool pcf8523_clear_os_integrity_flag(pcf8523_t *pcf8523) {
    if (!pcf8523)
        return false;

    return pcf8523_set_bit(pcf8523, PCF8523_SECONDS_REG, PCF8523_SECONDS_OS_MASK, false);
}

bool pcf8523_enable_interrupt_source(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                     pcf8523_InterruptSource_t pinMask, bool enable) {
    if (!pcf8523)
        return false;

    return pcf8523_set_bit(pcf8523, reg, pinMask, enable);
}

bool pcf8523_is_interrupt_source_enabled(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                         pcf8523_InterruptSource_t pinMask, bool *enabled) {
    if (!pcf8523 || !enabled)
        return false;

    return pcf8523_read_bit(pcf8523, reg, pinMask, enabled);
}

bool pcf8523_read_interrupt_flag(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                 pcf8523_InterruptFlag_t pinMask, bool *enabled) {
    if (!pcf8523 || !enabled)
        return false;

    return pcf8523_read_bit(pcf8523, reg, pinMask, enabled);
}

bool pcf8523_clear_interrupt_flag(pcf8523_t *pcf8523, pcf8523_CtrlReg_t reg,
                                  pcf8523_InterruptFlag_t pinMask) {
    if (!pcf8523)
        return false;

    // RO pins
    if (pinMask == PCF8523_CTRL3_BATT_STATUS_INT_FLAG_MASK_RO ||
        pinMask == PCF8523_CTRL2_WATCHDOG_TMR_A_INT_FLAG_MASK_RO)
        return false;

    if (!pcf8523_set_bit(pcf8523, reg, pinMask, true))
        return false;

    return true;
}

bool pcf8523_freeze_time(pcf8523_t *pcf8523, bool freeze) {
    if (!pcf8523)
        return false;

    return pcf8523_set_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_STOP_MASK, !freeze);
}

bool pcf8523_is_time_frozen(pcf8523_t *pcf8523, bool *frozen) {
    if (!pcf8523 || !frozen)
        return false;

    bool buffer;
    if (!pcf8523_read_bit(pcf8523, PCF8523_CTRL1_REG, PCF8523_CTRL1_STOP_MASK, &buffer))
        return false;

    *frozen = !buffer;

    return true;
}

bool pcf8523_set_offset(pcf8523_t *pcf8523, pcf8523_OffsetMode_t mode, int8_t offset) {
    if (!pcf8523)
        return false;

    if (offset > 63 || offset < -64)
        return false;

    uint8_t buffer = (uint8_t)offset;

    if (mode == PCF8523_OFFSET_EVERY_MIN)
        buffer |= PCF8523_OFFSET_MODE_MASK;

    if (!pcf8523_write_register(pcf8523, PCF8523_OFFSET_REG, buffer))
        return false;

    return true;
}

bool pcf8523_read_offset(pcf8523_t *pcf8523, pcf8523_OffsetMode_t *mode, int8_t *offset) {
    if (!pcf8523 || !offset)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, PCF8523_OFFSET_REG, &buffer))
        return false;

    if (buffer & PCF8523_OFFSET_MODE_MASK) {
        *mode = PCF8523_OFFSET_EVERY_MIN;
        buffer &= ~PCF8523_OFFSET_MODE_MASK;
    }
    else {
        *mode = PCF8523_OFFSET_EVERY_2_HOURS;
    }

    *offset = (int8_t)buffer;

    return true;
}

bool pcf8523_set_timer_a_mode(pcf8523_t *pcf8523, pcf8523_TmrAMode_t mode) {
    if (!pcf8523)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, PCF8523_TMR_CTRL_REG, &buffer))
        return false;

    buffer &= ~PCF8523_TMR_CTRL_TMR_A_MODE_MASK;
    buffer |= mode;

    if (!pcf8523_write_register(pcf8523, PCF8523_TMR_CTRL_REG, buffer))
        return false;

    return true;
}

bool pcf8523_read_timer_a_mode(pcf8523_t *pcf8523, pcf8523_TmrAMode_t *mode) {
    if (!pcf8523 || !mode)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, PCF8523_TMR_CTRL_REG, &buffer))
        return false;

    buffer &= PCF8523_TMR_CTRL_TMR_A_MODE_MASK;

    *mode = (pcf8523_TmrAMode_t)buffer;

    return true;
}

bool pcf8523_set_timer_b_mode(pcf8523_t *pcf8523, bool enable) {
    if (!pcf8523)
        return false;

    return pcf8523_set_bit(pcf8523, PCF8523_TMR_CTRL_REG, PCF8523_TMR_CTRL_TMR_B_ENABLED_MASK,
                           enable);
}

bool pcf8523_read_timer_b_mode(pcf8523_t *pcf8523, bool *enabled) {
    if (!pcf8523 || !enabled)
        return false;

    return pcf8523_read_bit(pcf8523, PCF8523_TMR_CTRL_REG, PCF8523_TMR_CTRL_TMR_B_ENABLED_MASK,
                            enabled);
}

bool pcf8523_set_timer_int_mode(pcf8523_t *pcf8523, pcf8523_Tmr_t tmr, pcf8523_TmrIntMode intMode) {
    if (!pcf8523)
        return false;

    uint8_t pinMask;
    if (tmr == PCF8523_TMR_A_TMR_SEC)
        pinMask = PCF8523_TMR_CTRL_PERM_TMR_A_TMR_SEC_INT_MASK;
    else
        pinMask = PCF8523_TMR_CTRL_PERM_TMR_B_INT_MASK;

    if (!pcf8523_set_bit(pcf8523, PCF8523_TMR_CTRL_REG, pinMask, intMode))
        return false;
}

bool pcf8523_read_timer_int_mode(pcf8523_t *pcf8523, pcf8523_Tmr_t tmr,
                                 pcf8523_TmrIntMode *intMode) {
    if (!pcf8523 || !intMode)
        return false;

    return pcf8523_read_bit(pcf8523, PCF8523_TMR_CTRL_REG, tmr, (bool *)intMode);
}

bool pcf8523_set_timer_a_duration(pcf8523_t *pcf8523, pcf8523_TimerAValue *tmrA) {
    if (!pcf8523 || !tmrA)
        return false;

    uint8_t buffer[2];
    buffer[0] = tmrA->sourceFreq;
    buffer[1] = tmrA->value;

    if (!pcf8523_write_block(pcf8523, PCF8523_TMR_A_FREQ_CTRL_REG, buffer, 2))
        return false;

    return true;
}

bool pcf8523_read_timer_a_duration(pcf8523_t *pcf8523, pcf8523_TimerAValue *tmrA) {
    if (!pcf8523 || !tmrA)
        return false;

    uint8_t buffer[2];

    if (!pcf8523_read_block(pcf8523, PCF8523_TMR_A_FREQ_CTRL_REG, buffer, 2))
        return false;

    tmrA->sourceFreq = (pcf8523_ClkSourceFreq_t)buffer[0];
    tmrA->value = buffer[1];

    return true;
}

bool pcf8523_set_timer_b_duration(pcf8523_t *pcf8523, pcf8523_TimerBValue *tmrB) {
    if (!pcf8523 || !tmrB)
        return false;

    uint8_t buffer[2];

    buffer[0] = tmrB->intWidth;
    buffer[0] |= tmrB->sourceFreq;
    buffer[1] = tmrB->value;

    if (!pcf8523_write_block(pcf8523, PCF8523_TMR_B_FREQ_CTRL_REG, buffer, 2))
        return false;

    return true;
}

bool pcf8523_read_timer_b_duration(pcf8523_t *pcf8523, pcf8523_TimerBValue *tmrB) {
    if (!pcf8523 || !tmrB)
        return false;

    uint8_t buffer[2];

    if (!pcf8523_read_block(pcf8523, PCF8523_TMR_B_FREQ_CTRL_REG, buffer, 2))
        return false;

    uint8_t widthBuf = buffer[0] & PCF8523_TMR_B_INT_WIDTH_MASK;
    tmrB->intWidth = (pcf8523_TmrBIntWidth_t)(widthBuf >> 4);

    tmrB->sourceFreq = (pcf8523_ClkSourceFreq_t)(buffer[0] & PCF8523_TMR_B_INT_WIDTH_MASK);

    tmrB->value = buffer[1];

    return true;
}

bool pcf8523_set_clk_out_mode(pcf8523_t *pcf8523, pcf8523_ClkOutFreq_t clkOutFreq) {
    if (!pcf8523)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, PCF8523_TMR_CTRL_REG, &buffer))
        return false;

    buffer &= ~PCF8523_TMR_CTRL_CLKOUT_FREQ_MASK;
    buffer |= clkOutFreq;

    if (!pcf8523_write_register(pcf8523, PCF8523_TMR_CTRL_REG, buffer))
        return false;

    return true;
}

bool pcf8523_read_clk_out_mode(pcf8523_t *pcf8523, pcf8523_ClkSourceFreq_t *clkOutFreq) {
    if (!pcf8523 || !clkOutFreq)
        return false;

    uint8_t buffer;

    if (!pcf8523_read_register(pcf8523, PCF8523_TMR_CTRL_REG, &buffer))
        return false;

    buffer &= PCF8523_TMR_CTRL_CLKOUT_FREQ_MASK;

    *clkOutFreq = (pcf8523_ClkSourceFreq_t)buffer;

    return true;
}

static inline uint8_t pcf8523_decimal_to_bcd(uint8_t decimal) {
    return decimal + 6 * (decimal / 10);
}

static inline uint8_t pcf8523_bcd_to_decimal(uint8_t bcd) {
    return bcd - 6 * (bcd >> 4);
}

bool pcf8523_validate_time_field(uint8_t reg, uint8_t value, pcf8523_HourMode_t *hourMode,
                                 bool pcf8523Format24h) {
    switch (reg) {
        case PCF8523_SECONDS_REG:
            return pcf8523_validate_sec(value);

        case PCF8523_MINUTES_ALARM_REG:
        case PCF8523_MINUTES_REG:
            return pcf8523_validate_min(value);

        case PCF8523_HOURS_ALARM_REG:
        case PCF8523_HOURS_REG:
            if (!hourMode)
                return false;
            return pcf8523_validate_hour(value, *hourMode, pcf8523Format24h);

        case PCF8523_WEEKDAYS_ALARM_REG:
        case PCF8523_WEEKDAYS_REG:
            return pcf8523_validate_weekday(value);

        case PCF8523_MONTHS_REG:
            return pcf8523_validate_month(value);

        case PCF8523_YEARS_REG:
            return pcf8523_validate_year(value);

        default:
            return false;
    }
}

bool pcf8523_validate_datetime(pcf8523_Datetime_t *datetime, bool pcf8523Format24h) {
    if (!datetime)
        return false;

    return pcf8523_validate_sec(datetime->sec) && pcf8523_validate_min(datetime->min) &&
           pcf8523_validate_hour(datetime->hour, datetime->hourMode, pcf8523Format24h) &&
           pcf8523_validate_day(datetime->day) && pcf8523_validate_weekday(datetime->weekDay) &&
           pcf8523_validate_month(datetime->month) && pcf8523_validate_year(datetime->year);
}

bool pcf8523_validate_alarm(pcf8523_Alarm_t *alarm, bool pcf8523Format24h) {
    if (!alarm)
        return false;

    return pcf8523_validate_min(alarm->minAlarm) &&
           pcf8523_validate_hour(alarm->hourAlarm, alarm->hourMode, pcf8523Format24h) &&
           pcf8523_validate_day(alarm->dayAlarm) && pcf8523_validate_weekday(alarm->weekDayAlarm);
}

static inline bool pcf8523_validate_sec(uint8_t sec) {
    return sec <= 59;
}

static inline bool pcf8523_validate_min(uint8_t min) {
    return min <= 59;
}

static inline bool pcf8523_validate_day(uint8_t day) {
    return day >= 1 && day <= 31;
}

static inline bool pcf8523_validate_weekday(uint8_t weekDay) {
    return weekDay <= 6;
}

static inline bool pcf8523_validate_month(uint8_t month) {
    return month >= 1 && month <= 12;
}

static inline bool pcf8523_validate_year(uint8_t year) {
    return year <= 99;
}

static inline bool pcf8523_validate_hour(uint8_t hour, pcf8523_HourMode_t hourMode,
                                         bool pcf8523Format24h) {
    if (hourMode == PCF8523_HOUR_MODE_24H && !pcf8523Format24h)
        return false;

    if (hourMode == PCF8523_HOUR_MODE_24H) {
        return hour <= 23;
    }
    else {
        return hour >= 1 && hour <= 12;
    }
}
