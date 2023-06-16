/// @file motor.h
/// @brief Handles interaction with the hoverboard motor controller

#ifndef __MOTOR_H
#define __MOTOR_H

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include "logger.h"

#define MOTOR_START_FRAME 0xABCD

typedef struct {
	uint16_t start;
	int16_t  steer;
	int16_t  speed;
	uint16_t checksum;
} Motor_Command;

typedef struct{
	 uint16_t start;
	 int16_t  cmd1;
	 int16_t  cmd2;
	 int16_t  speedR_meas;
	 int16_t  speedL_meas;
	 int16_t  batVoltage;
	 int16_t  boardTemp;
	 uint16_t cmdLed;
	 uint16_t checksum;
} Motor_Feedback;

int motor_init(int serial_port_fd, int baudrate);
void motor_send_command(int serial_port_fd, int16_t uSteer, int16_t uSpeed);
bool motor_receive(int serial_port_fd, Motor_Feedback* feedback);

#endif