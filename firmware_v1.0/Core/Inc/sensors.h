/*
 * sensors.h
 *
 *  Created on: Jun 17, 2025
 *      Author: dst2001055
 */

#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_

#include "mal.h"

#define SAMPLE_SIZE 5
#define SENSOR_DELAY_MS 50
#define ACCEL_DELAY_MS 10

#define TEMP_LOW 0
#define TEMP_HIGH 1
#define HUM_LOW 2
#define HUM_HIGH 3

#define TEMP_MAX_LIMIT 90
#define TEMP_MIN_LIMIT 15
#define HUM_MAX_LIMIT 100
#define HUM_MIN_LIMIT 0

#define TEMP_INT_BIT 6
#define HUM_INT_BIT 4

#define ADXL343_REG_DEVID       0x00
#define ADXL343_REG_POWER_CTL   0x2D
#define ADXL343_REG_DATA_FORMAT 0x31
#define ADXL343_REG_DATAX0      0x32

#define ACCEL_SENSE 0.004f // 256 LSB/g -> full resolution
#define ACCEL_MG_FACTOR 1000

// --------------------------------------------------------------

typedef struct{
	float x_axis_accel;
	float y_axis_accel;
	float z_axis_accel;
}accel_axis;


// -------------------------------------------------------------	HDC2080 - T/H Sensor		------------------------------------------------
uint8_t config_T_H_sensor(uint8_t temp_max, uint8_t hum_max);
uint8_t sample_temp_hum();
float	get_temperature();
float	get_humidity();
uint8_t set_thresholds_T_H(uint8_t temp_max, uint8_t hum_max);

// -------------------------------------------------------------	ADXL343 - Accel	Sensor	------------------------------------------------
uint8_t config_ACCEL_sensor();
uint8_t sample_accel();
accel_axis get_accel();

// --------------------------------------------------------------------------------------------------------------------------------
bool check_threshold_active();


#endif /* INC_SENSORS_H_ */
