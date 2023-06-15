#include "steering.h"

void handle_button(
	ControllerConfig* config,
	Steering_InputState* state,
	struct js_event* ev
) {
	// This is absolutely fucking awful
	if (ev->number == config->buttons.circle) {
		state->circle = ev->value;
	} else if (ev->number == config->buttons.cross) {
		state->cross = ev->value;
	} else if (ev->number == config->buttons.triangle) {
		state->triangle = ev->value;
	} else if (ev->number == config->buttons.square) {
		state->square = ev->value;
	} else if (ev->number == config->buttons.cross) {
		state->cross = ev->value;
	} else if (ev->number == config->buttons.paddle_left) {
		state->paddle_left = ev->value;
	} else if (ev->number == config->buttons.paddle_right) {
		state->paddle_right = ev->value;
	} else if (ev->number == config->buttons.l2) {
		state->l2 = ev->value;
	} else if (ev->number == config->buttons.r2) {
		state->r2 = ev->value;
	} else if (ev->number == config->buttons.l3) {
		state->l3 = ev->value;
	} else if (ev->number == config->buttons.r3) {
		state->r3 = ev->value;
	} else if (ev->number == config->buttons.shift_up) {
		state->shift_up = ev->value;
	} else if (ev->number == config->buttons.shift_down) {
		state->shift_down = ev->value;
	} else if (ev->number == config->buttons.select) {
		state->select = ev->value;
	} else if (ev->number == config->buttons.start) {
		state->start = ev->value;
	}
}

int map_axis_value(short axis_value, int min, int max) {
	int axis_int = (int)axis_value;
	int axis_min = -32767;
	int axis_max = 32767;
	float axis_fraction = (float)(axis_int - axis_min) / (axis_max - axis_min);
	int mapped = axis_fraction * (max - min) + min;

	return mapped;
}

void handle_axis(
	ControllerConfig* config,
	Steering_InputState* state,
	struct js_event* ev
) {
	JoyAxisState axes[3];
	int axis = joy_get_axis_state(ev, axes);

	// Honestly, this is even worse than the button function
	if (axis == config->axes.throttle.axis_index) {
		state->throttle = map_axis_value(
			config->axes.throttle.value_index ? axes[axis].y : axes[axis].x,
			config->axes.throttle.lin_map_min,
			config->axes.throttle.lin_map_max
		);
	}

	if (axis == config->axes.steering.axis_index) {
		state->steering = map_axis_value(
			config->axes.steering.value_index ? axes[axis].y : axes[axis].x,
			config->axes.steering.lin_map_min,
			config->axes.steering.lin_map_max
		);
	}

	if (axis == config->axes.brake.axis_index) {
		state->brake = map_axis_value(
			config->axes.brake.value_index ? axes[axis].y : axes[axis].x,
			config->axes.brake.lin_map_min,
			config->axes.brake.lin_map_max
		);
	}

	if (axis == config->axes.dpad_horiz.axis_index) {
		state->dpad_horiz = map_axis_value(
			config->axes.dpad_horiz.value_index ? axes[axis].y : axes[axis].x,
			config->axes.dpad_horiz.lin_map_min,
			config->axes.dpad_horiz.lin_map_max
		);
	}

	if (axis == config->axes.dpad_vert.axis_index) {
		state->dpad_vert = map_axis_value(
			config->axes.dpad_vert.value_index ? axes[axis].y : axes[axis].x,
			config->axes.dpad_vert.lin_map_min,
			config->axes.dpad_vert.lin_map_max
		);
	}
}

int steering_check_event(
	int input_fd,
	ControllerConfig* config,
	Steering_InputState* state
) {
	struct js_event ev;
	int status = joy_read_event(input_fd, &ev);

	if (status != 0) return -1;

	switch(ev.type) {
		case JS_EVENT_BUTTON:
			handle_button(config, state, &ev);
			return 1;
		case JS_EVENT_AXIS:
			handle_axis(config, state, &ev);
			return 2;
		default:
			// Ignore other events
			break;
	}

	return 0;
}
