/*
 * app.h
 *
 *  Created on: May 5, 2025
 *      Author: dst2001055
 */

#ifndef INC_APP_H_
#define INC_APP_H_

#include "mal.h"
//#include "system.h"
#include "sensors.h"
#include "logger.h"

// ----- Thresholds define --------
#define TEMP_HIGH_ALERT_VAL 35.00
#define HUM_HIGH_ALERT_VAL  79.00

// ----- App timing define --------
#define APP_DELAY 5000
#define DEBOUNCE_DELAY 100

// ----- GPIO define --------
#define USER_BTN GPIO_PIN_12
#define INT_HDC2080_PIN GPIO_PIN_11

#define USER_LED_PIN GPIO_PIN_3
#define SYS_LED_PIN GPIO_PIN_2

#define ALARM_PWM_PIN GPIO_PIN_8

// --------------------------------
enum states{IDLE, DATA_READ, COMMS, ANOMALY, RECONNECT, LOGS, CLEAN_MEM};

// CONFIG
uint8_t init_device();

// APPLICATION
void app_fsm();
uint8_t state_idle();
uint8_t state_read_sensors();
uint8_t state_comms();
uint8_t state_anomaly();
uint8_t state_reconnect();
uint8_t state_print_logs();
uint8_t state_clean_memory();

// ERROR
void error_handler(uint8_t error_code);


#endif /* INC_APP_H_ */
