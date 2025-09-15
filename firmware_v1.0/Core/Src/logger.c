/*
 * logger.c
 *
 *  Created on: May 5, 2025
 *      Author: dst2001055
 */

#include "logger.h"

extern enum errorTypes ERROR_CODE;

color const reg_colors[8] = {
    {0, "BLK", "\033[0;30m"},
    {1, "RED", "\033[1;31m"},
    {2, "GRN", "\033[0;32m"},
    {3, "YEL", "\033[0;33m"},
    {4, "BLU", "\033[0;34m"},
    {5, "MAG", "\033[0;35m"},
    {6, "CYN", "\033[0;36m"},
    {7, "WHT", "\033[0;37m"},
};

log_struct const log_list[8] = {
	{ERROR_LOG,"ERROR", reg_colors[1]},
    {INFO_LOG, "INFO ", reg_colors[2]},
    {DEBUG_LOG,"DEBUG", reg_colors[6]},
	{WARNING_LOG,"WARNING", reg_colors[3]},
};

uint8_t log_write(uint8_t log_type, const char* log_msg, ...){

	char systemTime_Date 	[BUFFER_SIZE1] = {0};
	char log_msg_buffer  	[BUFFER_SIZE1] = {0};
	char debug_msg_buffer	[BUFFER_SIZE2] = {0};
	char formatted_log_msg	[BUFFER_SIZE2] = {0};

	//DEBUG TODO: REMOVE
	//rtc_calendar timestamp;

//	TODO: Fix - Add RTC functionality
	rtc_calendar timestamp = get_sys_time();
	if(timestamp.day == RTC_RETURN_ERR){
		return RTC_TIME_ERROR;
	}

	const char* color_code = log_list[log_type].color_info.code;

	va_list args;
	va_start(args, log_msg);

	vsnprintf(log_msg_buffer, sizeof(log_msg_buffer), log_msg, args);


	//TODO: if its a log for BLE module -> send info in a specific format


//------------------------------------------ Debug UART log print ------------------------------------------

	// Message with colors for PuTTY terminal
	// Log Type + Log message formated

	snprintf(formatted_log_msg, sizeof(formatted_log_msg), "%s[%s] %s%s", color_code, log_list[log_type].name_type, log_msg_buffer, RESET_COLOR);

	//DEBUG TODO: REMOVE
//	timestamp.hour = 0;
//	timestamp.minute = 0;
//	timestamp.second = 0;
//	timestamp.day = 19;
//	timestamp.month = 8;
//	timestamp.year = 25;

//	// Timestamp formated
	snprintf(systemTime_Date, sizeof(systemTime_Date), " @ %02d:%02d:%02d - %02d/%02d/%02d\r\n",
			timestamp.hour, timestamp.minute, timestamp.second,
			timestamp.day, timestamp.month, timestamp.year + YEAR_COEF);

	// Debug message concatenated
	snprintf(debug_msg_buffer, sizeof(debug_msg_buffer), "%s%s", formatted_log_msg, systemTime_Date);

	// Send message to desired UART
	const char* debug_msg = debug_msg_buffer;
	ERROR_CODE = send_UART_msg(DEBUG_UART_NUM, debug_msg);
	if(ERROR_CODE != NO_ERROR){
		return ERROR_CODE;
	}

// ------------------------------- Store log in EEPROM -> only if INFO or ERROR log ------------------------------------------

	if(log_type == ERROR_LOG || log_type == INFO_LOG){
		//TODO: log_eeprom()
	}

	va_end(args);

	return ERROR_CODE;
}
