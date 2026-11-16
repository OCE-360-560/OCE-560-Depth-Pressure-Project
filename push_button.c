// Code for the button trigger to get the pressure sensor reading

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define RED_BUTTON 14

void gpio_irq_handler(void)
{
    uint64_t now = time_us_64();

    // Read the interrupt status for GPIO bank 0
    uint32_t Blue_event = gpio_get_irq_event_mask(BLUE_BUTTON);

    // Button 1 pressing
    if (Blue_event & GPIO_IRQ_EDGE_FALL)
    {
        // Time check
        if ((now - Blue_last_time) > DEBOUNCE_TIME_US)
        {
            Blue_last_time = now;
            Blue_count++;

            // gpio_put(BLUE_LED_PIN, !gpio_get(BLUE_LED_PIN));
            gpio_acknowledge_irq(BLUE_BUTTON, GPIO_IRQ_EDGE_FALL);
        }
    }
}

// Callback for button pushing (single button)
 void gpio_callback(uint gpio, uint32_t events)
{
    bool level_blue = gpio_get(gpio);
    if (level_blue)
    {
        printf("GPIO_blue %d is HIGH\r\n", gpio);
        gpio_put(BLUE_LED_PIN, true);
    }
    else
    {
        printf("GPIO_blue %d is LOW\r\n", gpio);
        gpio_put(BLUE_LED_PIN, false);
        // count=count+1;
    }
 /* bool level_red = gpio_get(gpio);
    if (level_red){
        printf("GPIO_red %d is HIGH\r\n", gpio);
        gpio_put(RED_LED_PIN, true);
    } else{
        printf("GPIO_red %d is LOW\r\n", gpio);
        gpio_put(RED_LED_PIN, false);
        red_count=count+1;
    } */

}  