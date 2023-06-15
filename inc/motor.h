/// @file motor.h
/// @brief Handles interaction with the hoverboard motor controller

#ifndef __MOTOR_H
#define __MOTOR_H

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "logger.h"

int motor_init(int serial_port_fd, int baudrate);

#endif