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

int main()
{

    // Initialize standard I/O (for printf)
    stdio_init_all();
    sleep_ms(2000); // Delay to allow terminal to open
    printf("Start - VEML7700 example\n");

    // Sensor state structure
    veml7700_sensor_t light_sensor;
    lux_sensor lux_reading={0}; 
    // {
    // float lux_value;
    // uint16_t als_raw;
    // uint16_t white_raw;
    // };

    float status = light_sensor_init(&light_sensor);

    device_id(status, &light_sensor);

    // Power on the sensor (it's enabled by default after init, but can be toggled)
    veml7700_power_on(&light_sensor);

sleep_ms(1000); // Short pause after power on

while (true)
{
    // Readi in both ambient light and ALS light
    status = read_light_sensor(&lux_reading, &light_sensor);

    sleep_ms(1000); // Wait 1 second before next reading
}
}

// int main()
// {

//     return 0;
// }