/* veml7700 light sensor function
link code is used from for the light sensor:
GitHub - JonaszPocwiardowski/veml7700-pico-library: C library for the VEML7700 ambient light sensor on Raspberry Pi Pico. · GitHub



*/
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "veml7700.h" // Include the library header"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"

// I2C pin configuration---need pin numbers still
#define I2C_PORT i2c0
#define I2C_SDA_PIN 
#define I2C_SCL_PIN 
#define I2C_BAUDRATE 100000 // 100 kHz

bool light_sensor_init(veml7700_sensor_t *light_sensor)
{
    // Initialize I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    int status = veml7700_init(&light_sensor, I2C_PORT, I2C_SDA_PIN, I2C_SCL_PIN, I2C_BAUDRATE);

    if (status != VEML7700_OK) {
        printf("VEML7700 initialization error: %d\n", status);
        while (1); // Halt on error
    } else {
        printf("VEML7700 successfully initialized.\n");
    }}

// Read device ID (optional)
void device_id(float *status, veml7700_sensor_t *light_sensor ) 
{
uint16_t device_id;
status = veml7700_read_device_id(&light_sensor, &device_id);
if (status == VEML7700_OK)
{
    printf("Device ID: 0x%04X\n", device_id);
} // Code for the light sensor reading
}



bool read_light_sensor(lux_sensor *lux_sensor, veml7700_sensor_t *light_sensor )
{
    float lux_value=lux_sensor->lux_value;
    uint16_t als_raw=lux_sensor->als_raw;
    uint16_t white_raw=lux_sensor->white_raw;

// Read ambient light in lux
        float status = veml7700_read_lux(&light_sensor, &lux_value);
        if (status == VEML7700_OK) {
            printf("Ambient light: %.4f lx\n", lux_value);
        } else {
            printf("Lux read error: %d\n", status);
        }

        // Read raw ALS value (optional)
        status = veml7700_read_als(&light_sensor, &als_raw);
        if (status == VEML7700_OK) {
            printf("Raw ALS: %u\n", als_raw);
        } else {
            printf("ALS read error: %d\n", status);
        }

        // Read raw White channel value (optional)
        status = veml7700_read_white(&light_sensor, &white_raw);
        if (status == VEML7700_OK) {
            printf("Raw White: %u\n", white_raw);
        } else {
            printf("White read error: %d\n", status);
        }

        printf("----\n");
        return status;
}
