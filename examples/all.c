#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/i2c.h"
#include "sensor/pcf8523.h"

#define I2C_BUS i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define BASE_CENTURY 2000

void assert_ok(bool condition, const char *msg) {
    if (!condition) {
        printf("ERROR: %s\n", msg);
        exit(-1);
    }
}

void test_datetime(pcf8523_t *pcf) {
    pcf8523_Datetime_t dt = {
        .sec = 30, .min = 15, .hour = 10, .hourMode = PCF8523_HOUR_MODE_24H,
        .day = 9, .weekDay = 6, .month = 8, .year = 25
    };
    assert_ok(pcf8523_set_datetime(pcf, &dt), "set_datetime failed");

    pcf8523_Datetime_t rd;
    assert_ok(pcf8523_read_datetime(pcf, &rd), "read_datetime failed");
    printf("Datetime set/read OK: %02d:%02d:%02d %02d-%02d-%04d\n",
           rd.hour, rd.min, rd.sec, rd.day, rd.month, rd.year + BASE_CENTURY);
}

void test_alarm(pcf8523_t *pcf) {
    pcf8523_Alarm_t alarm = {
        .enableMinAlarm = true, .minAlarm = 20,
        .enableHourAlarm = true, .hourAlarm = 11, .hourMode = PCF8523_HOUR_MODE_24H,
        .enableDayAlarm = true, .dayAlarm = 10,
        .enableWeekDayAlarm = true, .weekDayAlarm = 0
    };
    assert_ok(pcf8523_set_alarm(pcf, &alarm), "set_alarm failed");

    pcf8523_Alarm_t rd;
    assert_ok(pcf8523_read_alarm(pcf, &rd), "read_alarm failed");
    printf("Alarm set/read OK: %02d:%02d day=%d weekday=%d\n",
           rd.hourAlarm, rd.minAlarm, rd.dayAlarm, rd.weekDayAlarm);
}

void test_power_mode(pcf8523_t *pcf) {
    assert_ok(pcf8523_set_power_mode(pcf, PCF8523_PWR_SWITCH_OVER_STANDARD_LOW_DETECT_ENABLED),
              "set_power_mode failed");
    pcf8523_PowerModes_t mode;
    assert_ok(pcf8523_read_power_mode(pcf, &mode), "read_power_mode failed");
    printf("Power mode read: %d\n", mode);
}

void test_offset(pcf8523_t *pcf) {
    assert_ok(pcf8523_set_offset(pcf, PCF8523_OFFSET_EVERY_MIN, 5), "set_offset failed");
    pcf8523_OffsetMode_t mode;
    int8_t offset;
    assert_ok(pcf8523_read_offset(pcf, &mode, &offset), "read_offset failed");
    printf("Offset mode=%d value=%d\n", mode, offset);
}

void test_timers(pcf8523_t *pcf) {
    pcf8523_TimerAValue ta = { .sourceFreq = PCF8523_CLK_SOURCE_FREQ_1_HZ, .value = 5 };
    assert_ok(pcf8523_set_timer_a_duration(pcf, &ta), "set_timer_a_duration failed");

    pcf8523_TimerAValue ta_rd;
    assert_ok(pcf8523_read_timer_a_duration(pcf, &ta_rd), "read_timer_a_duration failed");
    printf("Timer A: freq=%d val=%d\n", ta_rd.sourceFreq, ta_rd.value);

    pcf8523_TimerBValue tb = { .sourceFreq = PCF8523_CLK_SOURCE_FREQ_64_HZ,
                               .intWidth = PCF8523_TMR_B_INT_WIDTH_125_000_MS, .value = 10 };
    assert_ok(pcf8523_set_timer_b_duration(pcf, &tb), "set_timer_b_duration failed");

    pcf8523_TimerBValue tb_rd;
    assert_ok(pcf8523_read_timer_b_duration(pcf, &tb_rd), "read_timer_b_duration failed");
    printf("Timer B: freq=%d width=%d val=%d\n", tb_rd.sourceFreq, tb_rd.intWidth, tb_rd.value);
}

void test_clkout(pcf8523_t *pcf) {
    assert_ok(pcf8523_set_clk_out_mode(pcf, PCF8523_CLK_OUT_FREQ_1_HZ), "set_clk_out_mode failed");
    pcf8523_ClkSourceFreq_t freq;
    assert_ok(pcf8523_read_clk_out_mode(pcf, &freq), "read_clk_out_mode failed");
    printf("CLKOUT freq=%d\n", freq);
}

void test_capacitor(pcf8523_t *pcf) {
    assert_ok(pcf8523_set_oscilator_capacitor_value(pcf, PCF8523_12_5PF_CAPACITOR),
              "set_capacitor failed");
    pcf8523_CapacitorValue_t cap;
    assert_ok(pcf8523_read_oscilator_capacitor_value(pcf, &cap), "read_capacitor failed");
    printf("Capacitor value=%d\n", cap);
}

void test_interrupts(pcf8523_t *pcf) {
    assert_ok(pcf8523_enable_interrupt_source(pcf, PCF8523_CTRL1_REG,
                PCF8523_CTRL1_ENABLE_ALARM_INT_MASK, true), "enable_interrupt failed");

    bool en;
    assert_ok(pcf8523_is_interrupt_source_enabled(pcf, PCF8523_CTRL1_REG,
                PCF8523_CTRL1_ENABLE_ALARM_INT_MASK, &en), "read_interrupt_enabled failed");
    printf("Interrupt enabled? %d\n", en);

    bool flag;
    assert_ok(pcf8523_read_interrupt_flag(pcf, PCF8523_CTRL2_REG,
                PCF8523_CTRL2_ALARM_INT_FLAG_MASK, &flag), "read_interrupt_flag failed");
    printf("Alarm flag? %d\n", flag);

    assert_ok(pcf8523_clear_interrupt_flag(pcf, PCF8523_CTRL2_REG,
                PCF8523_CTRL2_ALARM_INT_FLAG_MASK), "clear_interrupt_flag failed");
}

int main() {
    stdio_init_all();
    i2c_init(I2C_BUS, 100000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    pcf8523_t pcf;
    assert_ok(pcf8523_init_struct(&pcf, I2C_BUS, PCF8523_DEFAULT_ADDR, true, false),
              "init_struct failed");
    assert_ok(pcf8523_soft_reset(&pcf), "soft_reset failed");
    sleep_ms(500);
    assert_ok(pcf8523_clear_os_integrity_flag(&pcf), "clear_os_integrity_flag failed");

    test_datetime(&pcf);
    test_alarm(&pcf);
    test_power_mode(&pcf);
    test_offset(&pcf);
    test_timers(&pcf);
    test_clkout(&pcf);
    test_capacitor(&pcf);
    test_interrupts(&pcf);

    printf("\nAll tests completed successfully!\n");
    return 0;
}
