/*
 * mal.h
 *
 *  Created on: May 6, 2025
 *      Author: dst2001055
 */

#ifndef INC_MAL_H_
#define INC_MAL_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "rtc.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "i2c.h"

//------------------------------- SYSTEM DEFINE -----------------------------

#define SENSOR_I2C &hi2c1
#define HDC2080_ADDR (0x40 << 1)
#define ADXL343_ADDR (0x53 << 1)

#define DEBUG_UART_NUM 1
#define DEBUG_UART &hlpuart1

#define COMMS_UART_NUM 2
#define COMMS_UART &huart2

#define RTC_RETURN_ERR 99
#define YEAR_COEF 2000

#define BUTTON_TIMER_NUM 2
#define BUTTON_TIMER &htim21
#define BUTTON_TIMER_PSC 2500
#define BUTTON_TIMER_ARR 2500

//#define BUZZER_TIMER &htim2
//#define BUZZER_TIMER_PSC 1413
//#define BUZZER_TIMER_ARR 1413

#define ERROR_DELAY_MS 1000

//---------------------------------------------------------

enum errorTypes{
	NO_ERROR,
	COMMS_ERROR,
	DEBUG_UART_ERROR,
	CONFIG_RTC_ERROR,
	RTC_TIME_ERROR,
	CONFIG_TIM_ERROR,
	TIMER_ERROR,
	I2C_ERROR,
	CONFIG_SENSOR_ERROR,
	SENSOR_READ_ERROR,
	GPIO_LED_ERROR,
	CONFIG_BUZZ_ERROR,
	GPIO_BUZZ_ERROR,
	PWR_MANAGE_ERROR
};

enum wireless_module_mode{
	COMMAND_MODE,
	DATA_MODE
};

enum i2c_sensors{
	HDC2080,	// TEMP + HUM
	ADXL343		// ACCEL
};

enum led_number{
	sys_LED,
	user_LED
};

enum led_modes{
	turn_LED_ON,
	turn_LED_OFF,
	toggle_LED
};

enum power_modes{
	run_mode,
	stop_mode_RTC
};

//---------------------------------------------------------

typedef struct{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint8_t year;
}rtc_calendar;



//---------------------- SYSTEM -------------------------------
void wait_delay(uint32_t ms);

//---------------------- COMMS -------------------------------
	// USER DEBUG
uint8_t send_UART_msg(uint8_t uart, const char* msg);

	// BLE COMMS
uint8_t config_ble_comms(uint8_t mode);
uint8_t send_BLE_msg(const char* ble_address, const char* msg);

//---------------------- TIMERs --------------------------------
// TODO: Mudar para uint16_t na STM32L0
uint8_t config_timer(uint8_t tim_num, uint32_t autoreload, uint32_t prescaler);
uint8_t start_timer_INT(TIM_HandleTypeDef *timer);
uint8_t stop_timer(TIM_HandleTypeDef *timer);

//---------------------- RTC ---------------------------------
uint8_t config_rtc(rtc_calendar date_time);
rtc_calendar get_sys_time();

//---------------------- SENSORS ----------------------
uint8_t read_i2c_sensor(uint16_t addr, uint8_t *pData, uint16_t size);
uint8_t write_i2c_sensor(uint16_t addr, uint8_t *pData, uint16_t size);

//---------------------- POWER -------------------------------
// uint8_t power_manage(uint8_t power_mode);
	//MODE -> Stop Mode c/ RTC	|	Normal Mode

//---------------------- EEPROM ------------------------------

// Memory read and write
// uint8_t mem_read(addr, data, size);
// uint8_t mem_write(addr, data, size);
// wipe_mem();

//---------------------- INTERFACEs --------------------------
//uint8_t config_buzzer(uint16_t autoreload, uint16_t prescaler, uint16_t pulse);
//uint8_t start_buzzer();
//uint8_t stop_buzzer();
//uint8_t control_LED(uint8_t led_num, uint8_t mode);

#endif /* INC_MAL_H_ */
