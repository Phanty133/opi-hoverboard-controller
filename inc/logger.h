/// @file logger.h
/// @brief Unified logging interface

#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>

typedef enum {
	INFO = 0,
	WARNING,
	ERROR
} LogLevel;

void log_message(LogLevel level, const char* msg);
void log_error(const char* msg);
void log_warning(const char* msg);
void log_info(const char* msg);

#endif