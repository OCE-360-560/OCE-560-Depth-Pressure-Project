c

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ==========================================
// LPS28 I2C CONFIGURATION
// ==========================================

#define I2C_PORT       i2c0
#define I2C_SDA        4
#define I2C_SCL        5
#define I2C_BAUDRATE   400000

#define LPS28_ADDR     0x5C

// ==========================================
// LPS28 REGISTERS
// ==========================================

#define WHO_AM_I       0x0F
#define CTRL_REG1      0x10
#define CTRL_REG2      0x11
#define CTRL_REG3      0x12
#define CTRL_REG4      0x13

#define STATUS_REG     0x27
#define PRESS_OUT      0x28
#define TEMP_OUT       0x2B

#define EXPECTED_ID    0xB4

#define PRESS_READY    0x01

// ==========================================
// I2C REGISTER FUNCTIONS
// ==========================================

// Write one byte to a sensor register
bool LPS28_WriteReg(uint8_t reg, uint8_t data)
{
    uint8_t buffer[2];

    buffer[0] = reg;
    buffer[1] = data;

    int result = i2c_write_blocking(
        I2C_PORT,
        LPS28_ADDR,
        buffer,
        2,
        false
    );

    return (result == 2);
}

// Read one byte from a sensor register
bool LPS28_ReadReg(uint8_t reg, uint8_t *data)
{
    int result;

    result = i2c_write_blocking(
        I2C_PORT,
        LPS28_ADDR,
        &reg,
        1,
        true
    );

    if (result != 1)
        return false;

    result = i2c_read_blocking(
        I2C_PORT,
        LPS28_ADDR,
        data,
        1,
        false
    );

    return (result == 1);
}

// Read multiple consecutive registers
bool LPS28_ReadMulti(uint8_t reg, uint8_t *data, size_t length)
{
    int result;

    result = i2c_write_blocking(
        I2C_PORT,
        LPS28_ADDR,
        &reg,
        1,
        true
    );

    if (result != 1)
        return false;

    result = i2c_read_blocking(
        I2C_PORT,
        LPS28_ADDR,
        data,
        length,
        false
    );

    return (result == (int)length);
}

// ==========================================
// SENSOR INITIALIZATION
// ==========================================

bool LPS28_Init(void)
{
    uint8_t id;
    uint8_t value;

    // Check sensor identity
    if (!LPS28_ReadReg(WHO_AM_I, &id))
        return false;

    if (id != EXPECTED_ID)
        return false;

    // CTRL_REG1:
    // ODR = 200 Hz (bits 6:3 = 1000)
    // Averaging = 4 samples (bits 2:0 = 000)

    if (!LPS28_WriteReg(CTRL_REG1, 0x40))
        return false;

    // CTRL_REG2:
    // FS_MODE = 1 (4060 hPa range)

    if (!LPS28_ReadReg(CTRL_REG2, &value))
        return false;

    value |= (1 << 6);

    if (!LPS28_WriteReg(CTRL_REG2, value))
        return false;

    // CTRL_REG3:
    // INT active high (bit 3 = 1)
    // Push-pull output (bit 1 = 0)

    if (!LPS28_ReadReg(CTRL_REG3, &value))
        return false;

    value |= (1 << 3);
    value &= ~(1 << 1);

    if (!LPS28_WriteReg(CTRL_REG3, value))
        return false;

    // CTRL_REG4:
    // Enable data-ready interrupt (bit 5)
    // Pulsed mode disabled
    // Other interrupts disabled

    if (!LPS28_ReadReg(CTRL_REG4, &value))
        return false;

    value |= (1 << 5);
    value &= ~(1 << 6);
    value &= ~(1 << 4);
    value &= ~(1 << 2);
    value &= ~(1 << 1);
    value &= ~(1 << 0);

    if (!LPS28_WriteReg(CTRL_REG4, value))
        return false;

    return true;
}

// ==========================================
// READ PRESSURE
// ==========================================

bool LPS28_GetPressure(float *pressure)
{
    uint8_t data[3];

    if (!LPS28_ReadMulti(PRESS_OUT, data, 3))
        return false;

    // Combine 3 bytes into a 24-bit value
    uint32_t raw_pressure =
        ((uint32_t)data[2] << 16) |
        ((uint32_t)data[1] << 8)  |
        ((uint32_t)data[0]);

    // FS_MODE = 1:
    // Resolution = 1/2048 hPa per LSB

    *pressure = raw_pressure / 2048.0f;

    return true;
}

// ==========================================
// READ TEMPERATURE
// ==========================================

bool LPS28_GetTemperature(float *temperature)
{
    uint8_t data[2];

    if (!LPS28_ReadMulti(TEMP_OUT, data, 2))
        return false;

    // Combine bytes into signed 16-bit value
    int16_t raw_temperature =
        (int16_t)(((uint16_t)data[1] << 8) |
                   (uint16_t)data[0]);

    // Resolution = 0.01 degrees C per LSB

    *temperature = raw_temperature * 0.01f;

    return true;
}

// ==========================================
// MAIN PROGRAM
// ==========================================

int main(void)
{
    uint8_t status;

    float pressure;
    float temperature;

    // Initialize USB serial output
    stdio_init_all();

    sleep_ms(2000);

    printf("\nLPS28 Test\n");

    // Initialize I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    printf("Initializing LPS28...\n");

    // Initialize sensor
    if (!LPS28_Init())
    {
        printf("Failed to find or initialize LPS28!\n");

        while (1)
        {
            sleep_ms(100);
        }
    }

    printf("LPS28 sensor found!\n");

    // Main measurement loop
    while (1)
    {
        // Check pressure data-ready flag
        if (LPS28_ReadReg(STATUS_REG, &status))
        {
            if (status & PRESS_READY)
            {
                // Read pressure and temperature
                if (LPS28_GetPressure(&pressure) &&
                    LPS28_GetTemperature(&temperature))
                {
                    printf("Pressure (hPa): %.2f\n", pressure);

                    printf("Temperature (C): %.2f\n",
                           temperature);

                    printf("----------------------\n");
                }
            }
        }

        // Poll every 1 ms
        sleep_ms(1);
    }

    return 0;
}// Code for the pressure sensor reading
