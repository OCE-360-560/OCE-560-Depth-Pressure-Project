
/*  
New Pressure sensor code base from chatgpt conversion of arduiono code from here: 
 https://learn.adafruit.com/adafruit-lps28-pressure-sensor/arduino 
 
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "lps28dfw_reg.h"


/* -------------------------------
   LPS28 Configuration
-------------------------------- */

#define I2C_PORT        i2c0
#define I2C_SDA         4
#define I2C_SCL         5

#define LPS28_ADDRESS   0x5C

/* Sensor registers */
#define WHO_AM_I        0x0F
#define CTRL_REG1       0x10
#define CTRL_REG2       0x11
#define CTRL_REG3       0x12
#define CTRL_REG4       0x13

// #define STATUS_REG      0x27
#define PRESS_OUT       0x28
#define TEMP_OUT        0x2B

#define LPS28_ID        0xB4

// #define PRESS_READY     0x01

/* -------------------------------
   I2C Register Functions
-------------------------------- */

/* Read sensor registers */
bool LPS28_ReadRegisters(uint8_t reg, uint8_t *data, uint8_t length)
{
    int result;

    result = i2c_write_blocking( I2C_PORT, LPS28_ADDRESS, &reg, 1, true);

    if (result < 0)
        return false;

    result = i2c_read_blocking(I2C_PORT, LPS28_ADDRESS, data, length, false);

    return result == length;
}

/* Write one sensor register */
bool LPS28_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2];

    buffer[0] = reg;
    buffer[1] = value;

    return i2c_write_blocking( I2C_PORT, LPS28_ADDRESS, buffer, 2, false) == 2;
}

/* Read one register */
uint8_t LPS28_ReadRegister(uint8_t reg)
{
    uint8_t data = 0;

    LPS28_ReadRegisters(reg, &data, 1);

    return data;
}

/* -------------------------------
   Initialize I2C
-------------------------------- */

void I2C_Init(void)
{
    i2c_init(I2C_PORT, 400000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

/* -------------------------------
   Initialize LPS28 Sensor
-------------------------------- */

bool LPS28_Init(void)
{
    uint8_t sensor_id;

    /* Check sensor identity */
    sensor_id = LPS28_ReadRegister(WHO_AM_I);

    if (sensor_id != LPS28_ID)
    {
        return false;
    }

    /* Set 200 Hz ODR and 4-sample averaging */
    if (!LPS28_WriteRegister(CTRL_REG1, 0x40))
        return false;

    /* Enable 4060 hPa full-scale mode */
    if (!LPS28_WriteRegister(CTRL_REG2, 0x40))
        return false;

    /* Active-high, push-pull interrupt */
    if (!LPS28_WriteRegister(CTRL_REG3, 0x08))
        return false;

    /* Enable data-ready interrupt */
    if (!LPS28_WriteRegister(CTRL_REG4, 0x20))
        return false;

    return true;
}

/* -------------------------------
   Read Pressure
-------------------------------- */

float LPS28_ReadPressure(void)
{
    uint8_t data[3];
    uint32_t raw_pressure;

    if (!LPS28_ReadRegisters(PRESS_OUT, data, 3))
        return -1.0f;

    raw_pressure = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8)  | ((uint32_t)data[0]);

    /* FS_MODE = 1:
       Resolution = 1/2048 hPa per LSB
    */

    return (float)raw_pressure / 2048.0f;
}

/* -------------------------------
   Read Temperature
-------------------------------- */

float LPS28_ReadTemperature(void)
{
    uint8_t data[2];
    int16_t raw_temperature;

    if (!LPS28_ReadRegisters(TEMP_OUT, data, 2))
        return -999.0f;

    raw_temperature = (int16_t)(((uint16_t)data[1] << 8) | data[0] );

    /* Resolution = 0.01 degrees C */

    return (float)raw_temperature * 0.01f;
}

/* -------------------------------
   Main Program
-------------------------------- */

// int main(void)
// {
//     // Moved to final_proj_main

//     /* uint8_t status;

//     float pressure;
//     float temperature;

//     /* Initialize USB serial */
// /*     stdio_init_all();

//     sleep_ms(2000);

//     printf("\nLPS28 Test\n");

//     /* Initialize I2C 
//     I2C_Init();

//     sleep_ms(100); */

//     /* Initialize sensor */
//     /* if (!LPS28_Init())
//     {
//         printf("Failed to find LPS28 sensor!\n");

//         while (1)
//         {
//             sleep_ms(100);
//         }
//     }  */

//     // printf("LPS28 sensor found!\n");

//     /* Main measurement loop */
//     // while (1)
//     // {
//     //     /* Check pressure data-ready flag */
//     //     status = LPS28_ReadRegister(STATUS_REG);

//     //     if (status & PRESS_READY)
//     //     {
//     //         /* Read pressure */
//     //         pressure = LPS28_ReadPressure();

//     //         /* Read temperature */
//     //         temperature = LPS28_ReadTemperature();

//     //         /* Print measurements */
//     //         if (pressure >= 0.0f &&
//     //             temperature != -999.0f)
//     //         {
//     //             printf("Pressure (hPa): %.2f\n",
//     //                    pressure);

//     //             printf("Temperature (C): %.2f\n",
//     //                    temperature);

//     //             printf("----------------------\n");
//     //         }
//     //     }

//     //     sleep_ms(10);
//     }

//     return 0;
// }




