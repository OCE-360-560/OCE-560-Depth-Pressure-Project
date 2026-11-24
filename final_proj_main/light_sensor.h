/*
Light sensor heading and initializion 
veml7700 light sensor function

Nov. 22nd 2026
*/

#ifndef LIGHT_SENSOR_H 
#define LIGHT_SENSOR_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "VEML7700.h"

typedef struct {
    float lux_value;
    uint16_t als_raw;
    uint16_t white_raw;
} lux_sensor;


bool light_sensor_init(veml7700_sensor_t *light_sensor);

// Read device ID (optional)
bool device_id(float *status, veml7700_sensor_t *light_sensor);

bool read_light_sensor(lux_sensor *lux_sensor, veml7700_sensor_t *light_sensor);


#endif //LIGHT_SENSOR_H



