/// @file ctrl_config.h
/// @brief Reads and parses the controller config file

#ifndef __CTRL_CONFIG_H
#define __CTRL_CONFIG_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include "toml.h"
#include "logger.h"

/// @brief Joystick axis mapping
typedef struct {
	/// @brief Joystick axis index, 0-indexed
	uint8_t axis_index;

	/// @brief Axis value index, Possible values: 0, 1
	uint8_t value_index;

	/// @brief Minimum value of the joystick value's linear mapping
	int lin_map_min;

	/// @brief Maximum value of the joystick value's linear mapping
	int lin_map_max;

	/// @brief If true, output is lin_map_max - mapped(value)
	bool reversed;

	bool curved;
	float qbezier_x;
	float qbezier_y;
} Config_Axis;

/// @brief Maps joystick axes to event axes indices
typedef struct {
	Config_Axis throttle;
	Config_Axis brake;
	Config_Axis steering;
	Config_Axis dpad_horiz;
	Config_Axis dpad_vert;
} Config_AxisMap;

/// @brief Maps joystick buttons to event indices
typedef struct {
	uint8_t paddle_left;
	uint8_t paddle_right;
	uint8_t l2;
	uint8_t r2;
	uint8_t l3;
	uint8_t r3;
	uint8_t select;
	uint8_t start;
	uint8_t triangle;
	uint8_t square;
	uint8_t circle;
	uint8_t cross;
	uint8_t shift_up;
	uint8_t shift_down;
} Config_ButtonMap;

/// @brief Parsed configuration file data
typedef struct {
	/// @brief Path to the joystick's file. Format: /dev/input/jsX
	char joy_path[32];

	/// @brief Steering wheel axis map
	Config_AxisMap axes;

	/// @brief Steering wheel button map
	Config_ButtonMap buttons;

	/// @brief Path to the motor serial port. Format: /dev/input/ttySX
	char motor_port[32];

	unsigned int motor_baudrate;

	int brake_threshold;

	int reverse_speed_cap;

	float throttle_boost_mult;

	bool debug;
} ControllerConfig;

/// @brief Loads and parses config file
/// @param config_path Path to the config file
/// @param config Output config reference
/// @return 0 if successful, -1 otherwise
int config_load(const char* config_path, ControllerConfig* config);

#endif