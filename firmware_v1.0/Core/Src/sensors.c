/*
 * sensors.c
 *
 *  Created on: Jun 17, 2025
 *      Author: dst2001055
 */

#include "sensors.h"

// HDC2080
uint8_t  sensor_data[4];
uint16_t temp_value;
uint16_t hum_value;

// ADXL343
int16_t raw_acceleration[3];

// -----------------------------------------------------------------	HDC2080 - T/H Sensor	----------------------------------------------------------------------

uint8_t config_T_H_sensor(uint8_t temp_max, uint8_t hum_max){

	uint8_t config_command[2];

	config_command[0] = 0x0E;
	config_command[1] = 0x80;

	write_i2c_sensor(HDC2080_ADDR, config_command, sizeof(config_command));


	// Set thresholds of TEMP & HUM for INTERRUPT
	set_thresholds_T_H(temp_max, hum_max);


	// INT Config (0x07):  7	   6      5      4	    3		2 1 0
	// 				     DRY_EN  TH_EN  TL_EN  HH_EN  HL_EN 	 RES
	// TRIGG		       0       1      0      1      0       0 0 0 -> 0x50

	config_command[0] = 0x07;
	config_command[1] = 0x50;

	//config_command[1] = 0x80;

	write_i2c_sensor(HDC2080_ADDR, config_command, sizeof(config_command));

	// RESET + DRDY Config (0x0E):  7	6  5  4	     3		  2		     1		 0
	// 				 			  S_RST AMM[6:4]  HEAT_EN DDRY/INT_EN INT_POL INT_MODE
	// TRIGG		   				0   0  1  1      0        1          1       0 -> 0x36

	config_command[0] = 0x0E;
	config_command[1] = 0x36;

	write_i2c_sensor(HDC2080_ADDR, config_command, sizeof(config_command));


	// Measure (0x0F): 7	 6	  5	    4	3	2	         1	     0
	// 				  TRES[7:6]  HRES[5:4]  x  MEAS_CONFIG[2:1]  MEAS_TRIG
	// TRIGG		   0     0	  0     0   0   0            0      0/1  -> 0x00/0x01

	config_command[0] = 0x0F;
	config_command[1] = 0X00;

	write_i2c_sensor(HDC2080_ADDR, config_command, sizeof(config_command));

	// 1st HDC2080 measure
	sample_temp_hum();

	return NO_ERROR;
}


uint8_t sample_temp_hum(){

	uint8_t  measure_command[2];
	uint8_t  reading_command[1];

	// Send measure command
	measure_command[0] = 0x0F;
	measure_command[1] = 0X01;

	write_i2c_sensor(HDC2080_ADDR, measure_command, sizeof(measure_command));

	wait_delay(SENSOR_DELAY_MS);

	// Read data registers
	reading_command[0] = 0x00;
	write_i2c_sensor(HDC2080_ADDR, reading_command, sizeof(reading_command));
	read_i2c_sensor(HDC2080_ADDR, sensor_data, sizeof(sensor_data));

	return NO_ERROR;
}


float get_temperature(){

	float temp_value_float;

	temp_value = (sensor_data[TEMP_HIGH] << 8 | sensor_data[TEMP_LOW]);
	temp_value_float = temp_value;

	temp_value_float = ((temp_value_float * 165.0f) / 65536.0f) - (40.5f + 0.08f * (3.3f - 1.8f));

	return temp_value_float;
}


float get_humidity(){

	float hum_value_float;

	hum_value = (sensor_data[HUM_HIGH] << 8 | sensor_data[HUM_LOW]);

	hum_value_float = hum_value;
	hum_value_float = (hum_value_float / 65536.0f) * 100.0f;

	return hum_value_float;
}


uint8_t set_thresholds_T_H(uint8_t temp_max, uint8_t hum_max){

	uint8_t temp_high, hum_high;
	uint8_t configh_TRSHLD_command[2];

//----------------------------------- TEMPERATURE ------------------------------------------

	if(temp_max >= TEMP_MAX_LIMIT){
		temp_max = TEMP_MAX_LIMIT;
	}
	else if(temp_max <= TEMP_MIN_LIMIT){
		temp_max = TEMP_MIN_LIMIT;
	}

	temp_high = (uint8_t)(256.0f * (temp_max + 40.0f) / 165.0f);

	configh_TRSHLD_command[0] = 0x0B;
	configh_TRSHLD_command[1] = temp_high;

	write_i2c_sensor(HDC2080_ADDR, configh_TRSHLD_command, sizeof(configh_TRSHLD_command));

//----------------------------------- HUMIDITY ------------------------------------------

	if(hum_max >= HUM_MAX_LIMIT){
		hum_max = HUM_MAX_LIMIT;
	}
	else if(hum_max <= HUM_MIN_LIMIT){
		hum_max = HUM_MIN_LIMIT;
	}

	hum_high = (uint8_t)(256.0f * hum_max / 100.0f);

	configh_TRSHLD_command[0] = 0x0D;
	configh_TRSHLD_command[1] = hum_high;

	write_i2c_sensor(HDC2080_ADDR, configh_TRSHLD_command, sizeof(configh_TRSHLD_command));

	return NO_ERROR;
}

// -----------------------------------------------------------------	ADXL343 - ACCELEROMETER		----------------------------------------------------------------------

uint8_t config_ACCEL_sensor(){

	uint8_t reading_command[1];
	uint8_t config_command[2];

	// Reading device ID
	reading_command[0] = 0x00;

	write_i2c_sensor(ADXL343_ADDR, reading_command, sizeof(reading_command));
	read_i2c_sensor(ADXL343_ADDR, sensor_data, sizeof(sensor_data));

	if(sensor_data[0] != 0xE5){
		return CONFIG_SENSOR_ERROR;
	}

	// CONFIG DATA FORMAT
	config_command[0] = 0x31;
	config_command[1] = 0x09;

	write_i2c_sensor(ADXL343_ADDR, config_command, sizeof(config_command));


	// MEASURE MODE
	config_command[0] = 0x2D;
	//config_command[1] = 0x08; //0x38 -> LINK + AUTO_SLEEP + MEASURE
	config_command[1] = 0x18;

	write_i2c_sensor(ADXL343_ADDR, config_command, sizeof(config_command));


	return NO_ERROR;
}

uint8_t sample_accel(){

	uint8_t reading_command[2];
	uint8_t config_command[2];
	uint8_t accel_data[6];


	// MEASURE MODE
	config_command[0] = 0x2D;
	config_command[1] = 0x08;

	write_i2c_sensor(ADXL343_ADDR, config_command, sizeof(config_command));


	// Get X, Y, Z
	reading_command[0] = 0x32;

	write_i2c_sensor(ADXL343_ADDR, reading_command, sizeof(reading_command));
	read_i2c_sensor(ADXL343_ADDR, accel_data, sizeof(accel_data));

	raw_acceleration[0] = (accel_data[1] << 8 | accel_data[0]);
	raw_acceleration[1] = (accel_data[3] << 8 | accel_data[2]);
	raw_acceleration[2] = (accel_data[5] << 8 | accel_data[4]);

	return NO_ERROR;
}


accel_axis get_accel(){

	accel_axis acceleration[SAMPLE_SIZE];
	accel_axis accel_average;

	for(uint8_t i=0; i < SAMPLE_SIZE; i++){

		sample_accel();

		acceleration[i].x_axis_accel = (raw_acceleration[0] * ACCEL_SENSE);
		accel_average.x_axis_accel += acceleration[i].x_axis_accel;

		acceleration[i].y_axis_accel = (raw_acceleration[1] * ACCEL_SENSE);
		accel_average.y_axis_accel += acceleration[i].y_axis_accel;

		acceleration[i].z_axis_accel = (raw_acceleration[2] * ACCEL_SENSE);
		accel_average.z_axis_accel += acceleration[i].z_axis_accel;

		wait_delay(ACCEL_DELAY_MS);
	}


	accel_average.x_axis_accel = (accel_average.x_axis_accel / SAMPLE_SIZE) * ACCEL_MG_FACTOR;
	accel_average.y_axis_accel = (accel_average.y_axis_accel / SAMPLE_SIZE) * ACCEL_MG_FACTOR;
	accel_average.z_axis_accel = (accel_average.z_axis_accel / SAMPLE_SIZE) * ACCEL_MG_FACTOR;


	//PUT ACCEL in sleep mode
	//SLEEP MODE -> bit 2 in 0x2D REGISTER

	return accel_average;
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------------------

// CHECK -> is bit set TH_STATUS & HH_STATUS
bool check_threshold_active(){

	uint8_t threshold_reg_data[1];
	uint8_t reading_command[1];

	// ADDR 0X04 -> Interrupt DRDY -> Read interrupt source
	reading_command[0] = 0x04;

	write_i2c_sensor(HDC2080_ADDR, reading_command, sizeof(reading_command));
	read_i2c_sensor(HDC2080_ADDR, threshold_reg_data, sizeof(threshold_reg_data));

	// Check bit 6 -> TEMP | bit 4 -> HUM
	if(threshold_reg_data[0] & (1 << TEMP_INT_BIT) || threshold_reg_data[0] & (1 << HUM_INT_BIT)){

		if(threshold_reg_data[0] & (1 << TEMP_INT_BIT)){
			printf("INT -> TEMP THRESHOLD\r\n");
		}

		if (threshold_reg_data[0] & (1 << HUM_INT_BIT)){
			printf("INT -> HUM THRESHOLD\r\n");
		}
		return true;
	}

	// Check ADXL343 INT thresholds

	return false;
}

