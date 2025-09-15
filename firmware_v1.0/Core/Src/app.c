/*
 * app.c
 *
 *  Created on: May 5, 2025
 *      Author: dst2001055
 */

#include "app.h"

// ----- INIT TIME DEFINE -----
#define SYSTEM_HOUR  15
#define SYSTEM_MIN 	 40
#define SYSTEM_SEC 	 0

#define SYSTEM_DAY 	 10
#define SYSTEM_MONTH 9
#define SYSTEM_YEAR  25

// ---------------------------

enum states NEXT_STATE = IDLE;
enum states CURRENT_STATE = IDLE;

extern enum errorTypes ERROR_CODE;

bool flag_anomaly_temp = false;
bool flag_anomaly_hum = false;

rtc_calendar system_time = {
	.hour = SYSTEM_HOUR,
	.minute = SYSTEM_MIN,
	.second = SYSTEM_SEC,
	.day = SYSTEM_DAY,
	.month = SYSTEM_MONTH,
	.year = SYSTEM_YEAR
};

//---------------------------------------- CONFIG ----------------------------------------
uint8_t init_device(){

	// Config COMMS (BLE)



	//Config RTC - get info from BLE COMMS or Manual Input from user
	ERROR_CODE = config_rtc(system_time);
	if(ERROR_CODE != NO_ERROR){
		error_handler(ERROR_CODE);
	}

	// config TIMER -> CLK @ 2 MHz -> f=1 Hz
	ERROR_CODE = config_timer(BUTTON_TIMER_NUM, BUTTON_TIMER_ARR, BUTTON_TIMER_PSC);
	if(ERROR_CODE != NO_ERROR){
		error_handler(ERROR_CODE);
	}

	// Config TIMER_TRIGGER FOR DATA_READ -> INTERRUPT

	// Config PWR

	// Config EEPROM

	// Config Sensors -> TEMP & HUM SENSOR + ACCELOMETER
	ERROR_CODE = config_T_H_sensor(TEMP_HIGH_ALERT_VAL, HUM_HIGH_ALERT_VAL);
	if(ERROR_CODE != NO_ERROR){
		error_handler(ERROR_CODE);
	}

	//TODO: ADD ADXL343 CONFIG
	ERROR_CODE = config_ACCEL_sensor();
	if(ERROR_CODE != NO_ERROR){
		error_handler(ERROR_CODE);
	}

	// ADC - READ BATTERY PERCENTAGE

	// Config Interface


	NEXT_STATE = IDLE;

	return NO_ERROR;
}


//-------------------------------------------------------------------------- STATE MACHINE --------------------------------------------------------------------
void app_fsm(){

	//Get current RTC timestamp
	get_sys_time();

	CURRENT_STATE = NEXT_STATE;

	switch(CURRENT_STATE){
	  case IDLE:
		  ERROR_CODE = state_idle();
		  break;

	  case DATA_READ:
		  ERROR_CODE = state_read_sensors();
		  break;

	  case COMMS:
		  ERROR_CODE = state_comms();
		  break;

	  case ANOMALY:
		  ERROR_CODE = state_anomaly();
		  break;

	  case RECONNECT:
		  ERROR_CODE = state_reconnect();
		  break;

	  case LOGS:
		  ERROR_CODE = state_print_logs();
		  break;

	  case CLEAN_MEM:
		  ERROR_CODE = state_clean_memory();
		  break;

	  default:
		  NEXT_STATE = IDLE;
		  break;
	}

	if(ERROR_CODE != NO_ERROR){
		error_handler(ERROR_CODE);
	}
}

//---------------------------------------- STATE DEFINITION ----------------------------------
// TODO: START TIME_TRIGGER COUNT
uint8_t state_idle(){

	ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "IDLE");

	// Activate HDC2080 + ACCEL INTERRUPTS

	// -------------------------------------------------------------- BEGIN SLEEP MODE -----------------------------------------------

	// TODO: não é necessário se dps do state IDLE (stop mode) vier o DATA_READ !!!
	// Start time trigger -> stop + config - SAMPLE_TIME INT + start counting -> IN INTERRUPT WAKE UP MCU and resume operation


	// TODO: STOP MODE WITH RTC -> DEFINE SAMPLE TIME


//		HAL_SuspendTick();
//		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x500B, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
//
//		/* Enter STOP 2 mode */
//		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
//		HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
//		SystemClock_Config();
//		HAL_ResumeTick();
//

	// --------------------------------------------------------------- EXIT SLEEP MODE ----------------------------------------------------------

	//	ERROR_CODE = log_write(INFO_LOG, "Exiting sleep mode...");


	// Verificar condições de conexão - BLE Module
	// if not connected -> NEXT_STATE = RECONNECT -> DATA_READ (ativar flag e no IDLE filtrar e dar return logo para a FSM para ir para DATA_READ)
	// else -> NEXT_STATE = DATA_READ


	//HAL_Delay(5000);		// STOP MODE SIMULATION - ultra low power


	// start timer_INT e na callback -> NEXT_STATE = DATA_READ
	// ou adicionar mecanismo para perceber se ha flag vinda da INT que indique transição de estado, após filtrar - se sim, dá return nesta função para a FSM


	NEXT_STATE = DATA_READ;		//Debug reasons TODO: remove

	return ERROR_CODE;
}


//TODO: desligar o time_trigger que ativa a leitura -> voltar a ligar no IDLE
// SAMPLE SENSOR DATA -> TEMP + HUM + ACCEL
uint8_t state_read_sensors(){

	float temp_buff[SAMPLE_SIZE];
	float hum_buff[SAMPLE_SIZE];
	float sense_temp;
	float sense_hum;

	ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "READ SENSORS");

	//----------------------------------------------------------- HDC2080 -----------------------------------------------------------------------

	sense_temp = 0;
	sense_hum  = 0;

	for(uint8_t i=0; i < SAMPLE_SIZE; i++){

		//Sample T+H sensor
		sample_temp_hum();

		temp_buff[i] = get_temperature();
		hum_buff[i] = get_humidity();

		sense_temp += temp_buff[i];
		sense_hum += hum_buff[i];

		wait_delay(SENSOR_DELAY_MS);
	}

	sense_temp = sense_temp / SAMPLE_SIZE;
	sense_hum = sense_hum / SAMPLE_SIZE;

	// PRINT IN uint8_t -> FLASH SIZE 32 KB-> compile '-u _printf_float' -> too big for FLASH MEMORY
	uint8_t sense_temp_print = (uint8_t) roundf(sense_temp);
	uint8_t sense_hum_print = (uint8_t) roundf(sense_hum);

	ERROR_CODE = log_write(INFO_LOG, "Current Temperature ----> %u C", sense_temp_print);
	ERROR_CODE = log_write(INFO_LOG, "Current Humidity -------> %u %%", sense_hum_print);

	// Compare threshold values -> send to anomaly after COMMS
	if(sense_temp >= TEMP_HIGH_ALERT_VAL) flag_anomaly_temp = true;
	if(sense_hum >= HUM_HIGH_ALERT_VAL) flag_anomaly_hum = true;


	//----------------------------------------------------------- ADXL343 -----------------------------------------------------------------------

	accel_axis current_accel;
	int16_t current_x_accel, current_y_accel, current_z_accel;

	current_accel = get_accel();


	current_x_accel = (int16_t) roundf(current_accel.x_axis_accel);
	current_y_accel = (int16_t) roundf(current_accel.y_axis_accel);
	current_z_accel = (int16_t) roundf(current_accel.z_axis_accel);

	log_write(INFO_LOG, "Current X Acceleration -> %d mg", current_x_accel);
	log_write(INFO_LOG, "Current Y Acceleration -> %d mg", current_y_accel);
	log_write(INFO_LOG, "Current Z Acceleration -> %d mg", current_z_accel);

	//TODO ADD ACCEL THRESHOLD
	//if(sense_accel >= ACCEL_HIGH_ALERT_VAL) flag_anomaly_accel = true;

	NEXT_STATE = COMMS;

	return ERROR_CODE;
}


// Sends info to BLE Module via COMMS UART + Debug UART info about the current state of operation + write in EEPROM (overwrite older information)
uint8_t state_comms(){

	ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "COMMS");

	//TODO ADD ACCEL THRESHOLD

	// Filter if values are within normal defined range (TEMP | HUM | ACCEL)
	if(flag_anomaly_temp || flag_anomaly_hum){
		NEXT_STATE = ANOMALY;
	} else {
		ERROR_CODE = log_write(INFO_LOG, "Sensor Values Inside Defined Margin!");
		HAL_GPIO_WritePin(GPIOA, SYS_LED_PIN, GPIO_PIN_RESET);
		NEXT_STATE = IDLE;
	}

	return ERROR_CODE;
}


// Alert state -> activate interfaces + double check values (NEXT_STATE = DATA_READ)
uint8_t state_anomaly(){

	ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "ANOMALY");

	if(flag_anomaly_temp){
		flag_anomaly_temp = false;
		//start_buzzer();
		//control_led() -> toggle LED 1s intervals
		HAL_GPIO_WritePin(GPIOA, SYS_LED_PIN, GPIO_PIN_SET);
		log_write(WARNING_LOG, "Temperature Threshold!");
	}

	if(flag_anomaly_hum){
		flag_anomaly_hum = false;
		//start_buzzer();
		//control_led() -> toggle LED 1s intervals
		HAL_GPIO_WritePin(GPIOA, SYS_LED_PIN, GPIO_PIN_SET);
		log_write(WARNING_LOG, "Humidity Threshold!");
	}


	NEXT_STATE = DATA_READ;

	return ERROR_CODE;
}


// Reconnect BLE module to destined gateway
uint8_t state_reconnect(){

	//ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "RECONNECT");

	NEXT_STATE = IDLE;

	return ERROR_CODE;
}


// Read EEPROM contents and send via DEBUG UART -> Extra: Send info to SD Card (Future updated PCB version ?)
uint8_t state_print_logs(){

	//ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "LOGS");


	// Print MIN and MAX values registered while functioning


	NEXT_STATE = IDLE;

	return ERROR_CODE;
}


// Clean EEPROM contents and reset pointer to start writing from position 0
uint8_t state_clean_memory(){

	//ERROR_CODE = log_write(DEBUG_LOG, "Current State -> %d - %s", CURRENT_STATE, "CLEAN MEMORY");

	NEXT_STATE = IDLE;

	return ERROR_CODE;
}


// Error Tracker
void error_handler(uint8_t ERROR_CODE){


	HAL_GPIO_TogglePin(GPIOA, SYS_LED_PIN);

	//for every config error -> increase tries_counter
	// if tries_counter >= MAX_TRIES -> go to sleep mode and try in x minutes

	switch(ERROR_CODE){
		case NO_ERROR:
			//log_write(INFO_LOG, "Normal Execution!");
			break;

		case COMMS_ERROR:
			//log_write(ERROR_LOG, "BLE Comms Error!");
			break;

		case DEBUG_UART_ERROR:
			//log_write(ERROR_LOG, "Debug UART Error!");
			break;

		case CONFIG_RTC_ERROR:
			//log_write(ERROR_LOG, "Failed RTC Config!");
			break;

		case RTC_TIME_ERROR:
			//log_write(ERROR_LOG, "RTC Error!");
			break;

		case CONFIG_TIM_ERROR:
			//log_write(ERROR_LOG, "Timer Config Error!");
			break;

		case I2C_ERROR:
			//log_write(ERROR_LOG, "I2C Error!");
			break;

		default:
			//log_write(ERROR_LOG, "Unexpected Execution Error!");
			break;
	}

}
