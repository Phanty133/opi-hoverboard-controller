#include "logger.h"

void log_message(LogLevel level, const char* msg) {
	switch (level) {
		case INFO:
			printf("INFO: %s\n", msg);
			break;
		case WARNING:
			printf("WARNING: %s\n", msg);
			break;
		case ERROR:
			printf("ERROR: %s\n", msg);
			break;
	}
}

void log_error(const char* msg) {
	log_message(ERROR, msg);
}

void log_warning(const char* msg) {
	log_message(WARNING, msg);
}

void log_info(const char* msg) {
	log_message(INFO, msg);
}
