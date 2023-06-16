/// @file steering.h
/// @brief Parses raw joystick input

#ifndef __STEERING_H
#define __STEERING_H

#include <stdbool.h>
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include "ctrl_config.h"
#include "joystick.h"

typedef struct {
	short steering;
	short brake;
	short throttle;
	short dpad_horiz;
	short dpad_vert;

	bool cross;
	bool square;
	bool circle;
	bool triangle;
	bool paddle_right;
	bool paddle_left;
	bool r2;
	bool l2;
	bool select;
	bool start;
	bool r3;
	bool l3;
	bool shift_down;
	bool shift_up;
} Steering_InputState;

void init_state(Steering_InputState* state);

int steering_check_event(
	int input_fd,
	ControllerConfig* config,
	Steering_InputState* state
);

#endif