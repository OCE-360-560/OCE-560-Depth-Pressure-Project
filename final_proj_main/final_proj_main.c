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
#include "pico/stdlib.h"
#include "lcd.h"
#include "pico/stdlib.h"
#include "lcd.h"
// #include "lps28dfw.h"
#include "VEML7700.h"

#include <stdio.h>
#include <string.h>

#define LCD_ADDRESS     0x27
#define SENSOR_ADDRESS  0x5C

// Write a complete 16-character LCD line
static void lcd_write_line(lcd_t *lcd,
                           uint row,
                           const char *text)
{
    char line[17];

    snprintf(line, sizeof(line), "%-16.16s", text);

    lcd_move_to(lcd, 0, row);
    lcd_puts(lcd, line);
}



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

    char pressure_text[17];
    char temperature_text[17];

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





    // LCD Display init
        // Configure LCD pins
    /* lcd_t lcd = {
        .rs = 16,
        .en = 17,
        .d4 = 18,
        .d5 = 19,
        .d6 = 20,
        .d7 = 21,
        .backlight = -1
    };

        // Initialize 16-column, 2-line LCD
    lcd_init(&lcd, 16, 2);

    // Display first line
    lcd_puts(&lcd, "Hello Raspberry!");

    // Move to second line
    lcd_move_to(&lcd, 0, 1);

    lcd_puts(&lcd, "Pico 2 C Code"); */


// LCD configuration (newer)
    lcd_t lcd;

    // Initialize LCD using I2C0
    lcd_init(&lcd,
             i2c0,
             4, 5,
             LCD_ADDRESS,
             16, 2);

    lcd_clear(&lcd);

    lcd_write_line(&lcd, 0, "Starting system");
    lcd_write_line(&lcd, 1, "Please wait...");

    // Initialize pressure sensor
//    if (!lps28dfw_init(i2c0, SENSOR_ADDRESS))
    if (!LPS28_Init())
    {
        lcd_clear(&lcd);
        lcd_write_line(&lcd, 0, "Sensor ERROR");
        lcd_write_line(&lcd, 1, "Check wiring");
    }

lcd_clear(&lcd);





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

                lcd_write_line(&lcd, 0, pressure_text);
            lcd_write_line(&lcd, 1, temperature_text);
            }
            else{
                 lcd_write_line(&lcd, 0, "Sensor read ERROR");
            lcd_write_line(&lcd, 1, "Check connection");
            }
        }
}


}


