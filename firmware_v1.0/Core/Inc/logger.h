/*
 * logger.h
 *
 *  Created on: May 5, 2025
 *      Author: dst2001055
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_

#include "mal.h"

#define ERROR_LOG	0
#define INFO_LOG	1
#define DEBUG_LOG	2
#define WARNING_LOG 3

#define BUFFER_SIZE1 0x80
#define BUFFER_SIZE2 0xFF

#define RESET_COLOR "\033[1;0m"


typedef struct{
    uint8_t ID;
    const char* name;
    const char* code;
}color;

typedef struct{
	uint8_t log_num;
	const char* name_type;
	color color_info;
}log_struct;

extern const color reg_colors[8];
extern const log_struct log_list[8];

uint8_t log_write(uint8_t log_type, const char* log_msg, ...);

#endif /* INC_LOGGER_H_ */
