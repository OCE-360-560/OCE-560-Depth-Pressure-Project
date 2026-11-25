
#include "lcd.h"
#include <stddef.h>

// PCF8574 pin assignments
#define PCF_RS       0x01
#define PCF_RW       0x02
#define PCF_EN       0x04
#define PCF_BL       0x08

#define PCF_DATA     0xF0

// Send one byte to PCF8574
static void pcf_write(lcd_t *lcd, uint8_t data)
{
    i2c_write_blocking(
        lcd->i2c,
        lcd->address,
        &data,
        1,
        false
    );
}

// Generate LCD enable pulse
static void lcd_enable(lcd_t *lcd, uint8_t data)
{
    pcf_write(lcd, data | PCF_EN);
    sleep_us(1);

    pcf_write(lcd, data & ~PCF_EN);
    sleep_us(50);
}

// Write 4-bit nibble
static void lcd_write_nibble(lcd_t *lcd,
                             uint8_t nibble,
                             bool rs)
{
    uint8_t data = (nibble << 4);

    if (rs) {
        data |= PCF_RS;
    }

    if (lcd->backlight) {
        data |= PCF_BL;
    }

    // RW remains low (write mode)
    lcd_enable(lcd, data);
}

// Write command
static void hal_write_command(lcd_t *lcd, uint8_t cmd)
{
    lcd_write_nibble(lcd, cmd >> 4, false);
    lcd_write_nibble(lcd, cmd & 0x0F, false);

    if (cmd == LCD_CLR || cmd == LCD_HOME) {
        sleep_ms(2);
    }
}

// Write data
static void hal_write_data(lcd_t *lcd, uint8_t data)
{
    lcd_write_nibble(lcd, data >> 4, true);
    lcd_write_nibble(lcd, data & 0x0F, true);
}

// Backlight control
void lcd_backlight_on(lcd_t *lcd)
{
    lcd->backlight = true;

    pcf_write(lcd, PCF_BL);
}

void lcd_backlight_off(lcd_t *lcd)
{
    lcd->backlight = false;

    pcf_write(lcd, 0x00);
}

// Clear LCD
void lcd_clear(lcd_t *lcd)
{
    hal_write_command(lcd, LCD_CLR);
    hal_write_command(lcd, LCD_HOME);

    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    lcd->implied_newline = false;
}

// Return home
void lcd_home(lcd_t *lcd)
{
    hal_write_command(lcd, LCD_HOME);

    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    lcd->implied_newline = false;
}

// Display control
void lcd_display_on(lcd_t *lcd)
{
    hal_write_command(lcd,
        LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void lcd_display_off(lcd_t *lcd)
{
    hal_write_command(lcd, LCD_ON_CTRL);
}

void lcd_show_cursor(lcd_t *lcd)
{
    hal_write_command(lcd,
        LCD_ON_CTRL | LCD_ON_DISPLAY |
        LCD_ON_CURSOR);
}

void lcd_hide_cursor(lcd_t *lcd)
{
    hal_write_command(lcd,
        LCD_ON_CTRL | LCD_ON_DISPLAY);
}

void lcd_blink_cursor_on(lcd_t *lcd)
{
    hal_write_command(lcd,
        LCD_ON_CTRL | LCD_ON_DISPLAY |
        LCD_ON_CURSOR | LCD_ON_BLINK);
}

void lcd_blink_cursor_off(lcd_t *lcd)
{
    hal_write_command(lcd,
        LCD_ON_CTRL | LCD_ON_DISPLAY |
        LCD_ON_CURSOR);
}

// Move cursor
void lcd_move_to(lcd_t *lcd, uint x, uint y)
{
    if (x >= lcd->columns) {
        x = lcd->columns - 1;
    }

    if (y >= lcd->lines) {
        y = lcd->lines - 1;
    }

    lcd->cursor_x = x;
    lcd->cursor_y = y;

    uint8_t address = x & 0x3F;

    if (y & 1) {
        address += 0x40;
    }

    if (y & 2) {
        address += lcd->columns;
    }

    hal_write_command(lcd, LCD_DDRAM | address);
}

// Write one character
void lcd_putchar(lcd_t *lcd, char c)
{
    if (c == '\n') {
        if (lcd->implied_newline) {
            lcd->implied_newline = false;
        } else {
            lcd->cursor_x = lcd->columns;
        }
    } else {
        hal_write_data(lcd, (uint8_t)c);
        lcd->cursor_x++;
    }

    if (lcd->cursor_x >= lcd->columns) {
        lcd->cursor_x = 0;
        lcd->cursor_y++;

        lcd->implied_newline = (c != '\n');
    }

    if (lcd->cursor_y >= lcd->lines) {
        lcd->cursor_y = 0;
    }

    lcd_move_to(lcd, lcd->cursor_x, lcd->cursor_y);
}

// Write string
void lcd_puts(lcd_t *lcd, const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str) {
        lcd_putchar(lcd, *str++);
    }
}

// Custom character
void lcd_custom_char(lcd_t *lcd,
                     uint location,
                     const uint8_t charmap[8])
{
    if (charmap == NULL) {
        return;
    }

    location &= 0x07;

    hal_write_command(lcd,
        LCD_CGRAM | (location << 3));

    sleep_us(40);

    for (int i = 0; i < 8; i++) {
        hal_write_data(lcd, charmap[i]);
        sleep_us(40);
    }

    lcd_move_to(lcd, lcd->cursor_x, lcd->cursor_y);
}

// Initialize LCD and I2C
void lcd_init(lcd_t *lcd,
              i2c_inst_t *i2c,
              uint sda,
              uint scl,
              uint8_t address,
              uint columns,
              uint lines)
{
    lcd->i2c = i2c;
    lcd->address = address;
    lcd->sda_pin = sda;
    lcd->scl_pin = scl;

    lcd->columns = columns > 40 ? 40 : columns;
    lcd->lines = lines > 4 ? 4 : lines;

    lcd->cursor_x = 0;
    lcd->cursor_y = 0;
    lcd->implied_newline = false;
    lcd->backlight = true;

    if (lcd->columns == 0 || lcd->lines == 0) {
        return;
    }

    // Initialize I2C
    i2c_init(i2c, 100000);

    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);

    gpio_pull_up(sda);
    gpio_pull_up(scl);

    sleep_ms(50);

    // Turn backlight on
    pcf_write(lcd, PCF_BL);

    // Initialize LCD in 4-bit mode
    lcd_write_nibble(lcd, 0x03, false);
    sleep_ms(5);

    lcd_write_nibble(lcd, 0x03, false);
    sleep_us(150);

    lcd_write_nibble(lcd, 0x03, false);
    sleep_us(150);

    lcd_write_nibble(lcd, 0x02, false);
    sleep_us(150);

    // Function set: 4-bit, 2-line, 5x8 font
    hal_write_command(lcd,
        LCD_FUNCTION | LCD_FUNCTION_2LINES);

    // Display off
    lcd_display_off(lcd);

    // Clear display
    lcd_clear(lcd);

    // Entry mode
    hal_write_command(lcd,
        LCD_ENTRY_MODE | LCD_ENTRY_INC);

    // Hide cursor
    lcd_hide_cursor(lcd);

    // Display on
    lcd_display_on(lcd);
}
