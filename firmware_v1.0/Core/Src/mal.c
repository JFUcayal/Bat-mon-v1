/*
 * mal.c
 *
 *  Created on: May 6, 2025
 *      Author: dst2001055
 */

#include "mal.h"

HAL_StatusTypeDef system_status = HAL_OK;

enum errorTypes ERROR_CODE = NO_ERROR;

extern bool flag_timer_on;

//----------------------------------------- SYSTEM -----------------------------------------------------
void wait_delay(uint32_t ms){
	HAL_Delay(ms);
}

//----------------------------------------- COMMS ------------------------------------------------------
uint8_t send_UART_msg(uint8_t uart, const char* msg){

	switch(uart){
		case DEBUG_UART_NUM:

			system_status = HAL_UART_Transmit(DEBUG_UART, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);

			if(system_status != HAL_OK){
				return DEBUG_UART_ERROR;
			}
			break;

		case COMMS_UART_NUM:
//TODO
//			system_status = HAL_UART_Transmit(COMMS_UART, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
//
//			if(system_status != HAL_OK){
//				return COMMS_ERROR;
//			}
			break;

		default:
			return ERROR_CODE;
			break;
	}
	return NO_ERROR;
}


//--------------------------------------------- TIMERs ----------------------------------------------------

//todo: use __HAL_TIM_GET... to calculate time frequency and add as a func parameter
uint8_t config_timer(uint8_t tim_num, uint32_t autoreload, uint32_t prescaler){

	switch(tim_num){
		case BUTTON_TIMER_NUM:
			__HAL_TIM_SET_AUTORELOAD(BUTTON_TIMER, autoreload);
			__HAL_TIM_SET_PRESCALER(BUTTON_TIMER, prescaler);

			__HAL_TIM_CLEAR_FLAG(BUTTON_TIMER, TIM_SR_UIF);
			break;


		default:
			return CONFIG_TIM_ERROR;
			break;
	}
	return NO_ERROR;
}

uint8_t start_timer_INT(TIM_HandleTypeDef *timer){

	system_status = HAL_TIM_Base_Start_IT(timer);
	if(system_status != HAL_OK){
	  return TIMER_ERROR;
	}

	return NO_ERROR;
}

uint8_t stop_timer(TIM_HandleTypeDef *timer){

	system_status = HAL_TIM_Base_Stop(timer);
	if(system_status != HAL_OK){
	  return TIMER_ERROR;
	}

	return NO_ERROR;
}


//-------------------------------------------------------------- RTC ----------------------------------------------------------
uint8_t config_rtc(rtc_calendar date_time){

	RTC_DateTypeDef sysDate;
	RTC_TimeTypeDef sysTime;

	sysTime.Hours = date_time.hour;
	sysTime.Minutes = date_time.minute;
	sysTime.Seconds = date_time.second;

	sysDate.Date = date_time.day;
	sysDate.Month = date_time.month;
	sysDate.Year = date_time.year;


	system_status = HAL_RTC_SetTime(&hrtc, &sysTime, RTC_FORMAT_BIN);
	if(system_status != HAL_OK){
	  return CONFIG_RTC_ERROR;
	}

	system_status = HAL_RTC_SetDate(&hrtc, &sysDate, RTC_FORMAT_BIN);
	if(system_status != HAL_OK){
	  return CONFIG_RTC_ERROR;
	}

	return NO_ERROR;
}


rtc_calendar get_sys_time(){

	rtc_calendar current_time;
	RTC_DateTypeDef sysDate;
	RTC_TimeTypeDef sysTime;

	system_status = HAL_RTC_GetTime(&hrtc, &sysTime, RTC_FORMAT_BIN);
	if(system_status != HAL_OK){
		current_time.day = RTC_RETURN_ERR;
		return current_time;
	}

	system_status = HAL_RTC_GetDate(&hrtc, &sysDate, RTC_FORMAT_BIN);
	if(system_status != HAL_OK){
		current_time.day = RTC_RETURN_ERR;
		return current_time;
	}

	current_time.hour = sysTime.Hours;
	current_time.minute = sysTime.Minutes;
	current_time.second = sysTime.Seconds;

	current_time.day = sysDate.Date;
	current_time.month = sysDate.Month;
	current_time.year = sysDate.Year;

	return current_time;
}
//-------------------------------------------------------------- I2C - SENSORS ---------------------------------------------------------


uint8_t read_i2c_sensor(uint16_t addr, uint8_t *pData, uint16_t size){

	system_status = HAL_I2C_Master_Receive(SENSOR_I2C, addr, pData, size, ERROR_DELAY_MS);

	if(system_status != HAL_OK){
		return I2C_ERROR;
	}

	return 0;
}


uint8_t write_i2c_sensor(uint16_t addr, uint8_t *pData, uint16_t size){

	system_status = HAL_I2C_Master_Transmit(SENSOR_I2C, addr, pData, size, ERROR_DELAY_MS);

	if(system_status != HAL_OK){
		return I2C_ERROR;
	}

	return NO_ERROR;
}

//----------------------------------------------------------- POWER ------------------------------------------------------

uint8_t power_manage(uint8_t power_mode){

	switch (power_mode) {
		case run_mode:

			break;

		case stop_mode_RTC:

			break;

		default:
			return PWR_MANAGE_ERROR;
			break;
	}
	return NO_ERROR;
}


//----------------------------------------- EEPROM ----------------------------------

// uint8_t mem_read(addr, data, size);
// uint8_t mem_write(addr, data, size);
// wipe_memory

//----------------------------------------- INTERFACES ------------------------------

//uint8_t config_buzzer(uint16_t autoreload, uint16_t prescaler, uint16_t pulse){
//
////	TIM_OC_InitTypeDef sConfigOC = {0};
//
//	__HAL_TIM_SET_AUTORELOAD(BUTTON_TIMER, BUZZER_TIMER_ARR);
//	__HAL_TIM_SET_PRESCALER(BUTTON_TIMER, BUZZER_TIMER_PSC);
//
////	  sConfigOC.OCMode = TIM_OCMODE_PWM1;
////	  sConfigOC.Pulse = pulse;
////	  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
////	  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
////	  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
////	  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
////	  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
////	  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
////	  {
////	    Error_Handler();
////		return CONFIG_BUZZ_ERROR;
////	  }
//
//	return NO_ERROR;
//}
//
//uint8_t start_buzzer(){
////	system_status = HAL_TIM_PWM_Start(htim, Channel);
//
//	if(system_status != HAL_OK){
//		return GPIO_BUZZ_ERROR;
//	}
//
//	return NO_ERROR;
//}
//
//uint8_t stop_buzzer(){
////	system_status = HAL_TIM_PWM_Stop(htim, Channel);
//
//	if(system_status != HAL_OK){
//		return GPIO_BUZZ_ERROR;
//	}
//
//	return NO_ERROR;
//}
//
//uint8_t control_LED(uint8_t led_num, uint8_t mode){
//
//	GPIO_TypeDef* port_GPIO;
//	uint16_t pin_GPIO_num;
//
//	switch(led_num){
//		case sys_LED:
////			port_GPIO = GPIOA;
////			pin_GPIO_num = GPIO_PIN_12;
//			break;
//
//		case user_LED:
////			port_GPIO = GPIOA;
////			pin_GPIO_num = GPIO_PIN_13;
//			break;
//
//		default:
//			return GPIO_LED_ERROR;
//			break;
//	}
//
//	switch(mode){
//		case turn_LED_ON:
//			HAL_GPIO_WritePin(port_GPIO, pin_GPIO_num, GPIO_PIN_SET);
//			break;
//
//		case turn_LED_OFF:
//			HAL_GPIO_WritePin(port_GPIO, pin_GPIO_num, GPIO_PIN_RESET);
//			break;
//
//		case toggle_LED:
//			HAL_GPIO_TogglePin(port_GPIO, pin_GPIO_num);
//			break;
//
//		default:
//			return GPIO_LED_ERROR;
//			break;
//	}
//	return NO_ERROR;
//}
