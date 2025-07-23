#include "pico/stdlib.h"
#include <stdio.h>

#include "hardware/i2c.h"
#include "sensor/hdc3022.h"

#define I2C_BUS i2c0
#define I2C_SDA 16
#define I2C_SCL 17

int main() {
    stdio_init_all();

    i2c_init(I2C_BUS, 100000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    return 0;
}
