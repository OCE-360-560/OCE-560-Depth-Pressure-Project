
#ifndef LCD_H
#define LCD_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>
#include <stdbool.h>

#define LCD_CLR             0x01
#define LCD_HOME            0x02

#define LCD_ENTRY_MODE      0x04
#define LCD_ENTRY_INC       0x02
#define LCD_ENTRY_SHIFT     0x01

#define LCD_ON_CTRL         0x08
#define LCD_ON_DISPLAY      0x04
#define LCD_ON_CURSOR       0x02
#define LCD_ON_BLINK        0x01

#define LCD_MOVE            0x10
#define LCD_MOVE_DISP       0x08
#define LCD_MOVE_RIGHT      0x04

#define LCD_FUNCTION        0x20
#define LCD_FUNCTION_2LINES 0x08

#define LCD_CGRAM           0x40
#define LCD_DDRAM           0x80

typedef struct {
    i2c_inst_t *i2c;
    uint8_t address;
    uint sda_pin;
    uint scl_pin;

    uint columns;
    uint lines;

    uint cursor_x;
    uint cursor_y;

    bool backlight;
    bool implied_newline;
} lcd_t;

void lcd_init(lcd_t *lcd, i2c_inst_t *i2c,
              uint sda, uint scl,
              uint8_t address,
              uint columns, uint lines);

void lcd_clear(lcd_t *lcd);
void lcd_home(lcd_t *lcd);

void lcd_display_on(lcd_t *lcd);
void lcd_display_off(lcd_t *lcd);

void lcd_show_cursor(lcd_t *lcd);
void lcd_hide_cursor(lcd_t *lcd);

void lcd_blink_cursor_on(lcd_t *lcd);
void lcd_blink_cursor_off(lcd_t *lcd);

void lcd_backlight_on(lcd_t *lcd);
void lcd_backlight_off(lcd_t *lcd);

void lcd_move_to(lcd_t *lcd, uint x, uint y);

void lcd_putchar(lcd_t *lcd, char c);
void lcd_puts(lcd_t *lcd, const char *str);

void lcd_custom_char(lcd_t *lcd, uint location,
                     const uint8_t charmap[8]);

#endif
