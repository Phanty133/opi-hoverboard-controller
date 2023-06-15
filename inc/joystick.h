/// @file joystick.h
/// @brief Handles steering wheel and pedal inputs

#ifndef __JOY_H
#define __JOY_H

#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

/// @brief X and Y values of a joystick axis
typedef struct {
	short x;
	short y;
} JoyAxisState;

/// @brief Reads an event from a joystick device.
/// @param fd Joystick device file descriptor
/// @param event Output event reference
/// @return 0 If successful, -1 otherwise
int joy_read_event(int fd, struct js_event *event);

/// @brief Reads the state of an axis. Assumes that axes are numbered starting from 0,
/// @brief and that the X axis is an even number, and the Y axis is an odd number.
/// @param event Joystick event
/// @param axes Output axis data reference
/// @return Read axis index
size_t joy_get_axis_state(struct js_event *event, JoyAxisState axes[3]);

#endif