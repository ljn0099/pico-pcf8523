#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#include "hardware/i2c.h"
#include "sensor/pcf8523.h"

#define I2C_BUS i2c0
#define I2C_SDA 16
#define I2C_SCL 17

#define BASE_CENTURY 2000

void set_datetime(pcf8523_t *pcf8523, pcf8523_HourMode_t hourMode) {
    pcf8523_Datetime_t testDatetime = {
        .sec = 0,
        .min = 0,
        .day = 1,
        .weekDay = 0,            // Saturday (0 = sunday, 6 = saturday)
        .month = 1,
        .year = 0,
    };
    switch (hourMode) {
        case PCF8523_HOUR_MODE_24H:
            testDatetime.hour = 0;
            testDatetime.hourMode = PCF8523_HOUR_MODE_24H;
            break;
        case PCF8523_HOUR_MODE_AM:
            testDatetime.hour = 12;
            testDatetime.hourMode = PCF8523_HOUR_MODE_AM;
            break;
        case PCF8523_HOUR_MODE_PM:
            testDatetime.hour = 6;
            testDatetime.hourMode = PCF8523_HOUR_MODE_PM;
            break;
    }

    if (!pcf8523_set_datetime(pcf8523, &testDatetime)) {
        printf("Error setting the datetime\n");
        exit(-1);
    }
}

void print_datetime(pcf8523_t *pcf8523, uint8_t quantity) {
    pcf8523_Datetime_t datetime;
    for (int i = 0; i < quantity; i++) {
        if (!pcf8523_read_datetime(pcf8523, &datetime)) {
            printf("Error reading datetime");
            exit(-1);
        }

        printf("\n");
        printf("Seconds: %d\n", datetime.sec);
        printf("Minutes: %d\n", datetime.min);
        printf("Hours: %d\n", datetime.hour);
        if (datetime.hourMode == PCF8523_HOUR_MODE_24H)
            printf("HourMode: 24h\n");
        else if (datetime.hourMode == PCF8523_HOUR_MODE_AM)
            printf("HourMode: AM\n");
        else if (datetime.hourMode == PCF8523_HOUR_MODE_PM)
            printf("Hour Mode: PM\n");
        printf("Day: %d\n", datetime.day);
        printf("Weekday: %d\n", datetime.weekDay);
        printf("Month: %d\n", datetime.month);
        printf("Year: %d\n", datetime.year+BASE_CENTURY);
        sleep_ms(1000);
    }
}

void print_datetime_single(pcf8523_t *pcf8523) {
    uint8_t value;
    printf("\n");

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_SECONDS_REG, &value, NULL)) {
        printf("Error reading seconds");
        exit(-1);
    }
    printf("Seconds: %d\n", value);
    sleep_ms(500);

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_MINUTES_REG, &value, NULL)) {
        printf("Error reading minutes");
        exit(-1);
    }
    printf("Minutes: %d\n", value);
    sleep_ms(500);

    pcf8523_HourMode_t hourMode;
    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_HOURS_REG, &value, &hourMode)) {
        printf("Error reading hours");
        exit(-1);
    }
    printf("Hours: %d\n", value);
    if (hourMode == 0)
        printf("HourMode: 24h\n");
    else if (hourMode == 1)
        printf("HourMode: AM\n");
    else
        printf("Hour Mode: PM\n");
    sleep_ms(500);

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_DAYS_REG, &value, NULL)) {
        printf("Error reading day\n");
        exit(-1);
    }
    printf("Day: %d\n", value);
    sleep_ms(500);

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_WEEKDAYS_REG, &value, NULL)) {
        printf("Error reading weekday\n");
        exit(-1);
    }
    printf("Weekday: %d\n", value);
    sleep_ms(500);

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_MONTHS_REG, &value, NULL)) {
        printf("Error reading month\n");
        exit(-1);
    }
    printf("Month: %d\n", value);
    sleep_ms(500);

    if (!pcf8523_read_datetime_field(pcf8523, PCF8523_YEARS_REG, &value, NULL)) {
        printf("Error reading year\n");
        exit(-1);
    }
    printf("Year: %d\n", value+BASE_CENTURY);
    sleep_ms(500);
}

int main() {
    stdio_init_all();

    i2c_init(I2C_BUS, 100000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    pcf8523_t pcf8523;

    if (!pcf8523_init_struct(&pcf8523, I2C_BUS, PCF8523_DEFAULT_ADDR, true, false)) {
        printf("Error initializating the struct\n");
        exit(-1);
    }
    if (!pcf8523_soft_reset(&pcf8523)) {
        printf("Error resetting\n");
        exit(-1);
    }
    sleep_ms(500);

    if (!pcf8523_clear_os_integrity_flag(&pcf8523)) {
        printf("Error clearing os intergrity flag\n");
        exit(-1);
    }

    // Test 24h
    set_datetime(&pcf8523, PCF8523_HOUR_MODE_24H);

    print_datetime(&pcf8523, 5);

    print_datetime_single(&pcf8523);

    if (!pcf8523_set_hour_mode(&pcf8523, true)) {
        printf("Error setting the hour mode\n");
        exit(-1);
    }

    // Test AM
    set_datetime(&pcf8523, PCF8523_HOUR_MODE_AM);

    print_datetime(&pcf8523, 5);

    print_datetime_single(&pcf8523);

    // Test PM
    set_datetime(&pcf8523, PCF8523_HOUR_MODE_PM);

    print_datetime(&pcf8523, 5);

    print_datetime_single(&pcf8523);
    return 0;
}
