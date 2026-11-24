#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include <stdint.h>
#include "tf_card.h"
#include "light_sensor.h"
#include "Pressure_sensor.h"
#include "push_button.h"
#include "sd_card_reader.h" // SD card logging
#include "hardware/i2c.h"
#include "bno08x.h"
#include "air530.h"
#include "veml7700.h"

/* -------------------------------
   LPS28 Configuration
-------------------------------- */
/* Sensor registers */
#define STATUS_REG      0x27
#define PRESS_READY     0x01




int main()
{
    // Initialize standard I/O (for printf)
    stdio_init_all();
    sleep_ms(2000); // Delay to allow terminal to open
    printf("Start - VEML7700 example\n");



    // Light sensor state structure initialization
    veml7700_sensor_t light_sensor;
    lux_sensor lux_reading={0}; 
    // {
    // float lux_value;
    // uint16_t als_raw;
    // uint16_t white_raw;
    // };

    float status_lux = light_sensor_init(&light_sensor);

    device_id(&status_lux, &light_sensor);

    // Power on the sensor (it's enabled by default after init, but can be toggled)
    veml7700_power_on(&light_sensor);

sleep_ms(1000); // Short pause after power on

// Pressure sensor initialization
    uint8_t status_pres;

    float pressure;
    float temperature;

    printf("\nLPS28 Test\n");

    /* Initialize I2C */
    I2C_Init();

    sleep_ms(100);

    /* Initialize sensor */
    if (!LPS28_Init())
    {
        printf("Failed to find LPS28 sensor!\n");

        while (1)
        {
            sleep_ms(100);
        }
    }
    printf("LPS28 sensor found!\n");



while (true)
{
    // Read in both ambient light and ALS light
    status_lux = read_light_sensor(&lux_reading, &light_sensor);

    sleep_ms(1000); // Wait 1 second before next reading

    
    // Pressure Sensor readings
    /* Check pressure data-ready flag */
        status_pres = LPS28_ReadRegister(STATUS_REG);

        if (status_pres & PRESS_READY)
        {
            /* Read pressure */
            pressure = LPS28_ReadPressure();

            /* Read temperature */
            temperature = LPS28_ReadTemperature();

            /* Print measurements */
            if (pressure >= 0.0f &&
                temperature != -999.0f)
            {
                printf("Pressure (hPa): %.2f\n",
                       pressure);

                printf("Temperature (C): %.2f\n",
                       temperature);

                printf("----------------------\n");
            }
        }
}


}