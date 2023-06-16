#include "steering.h"

void init_state(Steering_InputState* state) {
	state->steering = 0;
	state->brake = 0;
	state->throttle = 0;
	state->dpad_horiz = 0;
	state->dpad_vert = 0;

	state->cross = 0;
	state->square = 0;
	state->circle = 0;
	state->triangle = 0;
	state->paddle_right = 0;
	state->paddle_left = 0;
	state->r2 = 0;
	state->l2 = 0;
	state->select = 0;
	state->start = 0;
	state->r3 = 0;
	state->l3 = 0;
	state->shift_down = 0;
	state->shift_up = 0;
}

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

int linmap_axis(short axis_value, int min, int max) {
	int axis_int = (int)axis_value;
	int axis_min = -32767;
	int axis_max = 32767;
	float axis_fraction = (float)(axis_int - axis_min) / (axis_max - axis_min);
	int mapped = axis_fraction * (max - min) + min;

	return mapped;
}

int qbezier_axis(int axis_value, Config_Axis* axis) {
	float map_range = axis->lin_map_max - axis->lin_map_min;
	int val = axis_value;

	if (axis->lin_map_min < 0 && axis->lin_map_max > 0) {
		map_range = axis_value < 0 ? -axis->lin_map_min : axis->lin_map_max;
		val = val < 0 ? -val : val;
	}

	float t = val / map_range;
	float inv_t = 1 - t;
	float p0x = 0;
	float p0y = 0;
	float p1x = axis->qbezier_x;
	float p1y = axis->qbezier_y;
	float p2x = 1;
	float p2y = 1;

	// Out = P1 + (1 - t)^2(P0 - P1) + t^2(P2 - P1)
	// float out_x = p1x + inv_t * inv_t * (p0x - p1x) + t * t * (p2x - p1x);
	float out_bezier_y = p1y + inv_t * inv_t * (p0y - p1y) + t * t * (p2y - p1y);
	float out_y = out_bezier_y * map_range;

	if (axis->lin_map_min < 0 && axis->lin_map_max > 0 && val < 0) {
		out_y = -out_y;
	}

	return out_y;
}

void handle_axis(
	ControllerConfig* config,
	Steering_InputState* state,
	struct js_event* ev
) {
	static JoyAxisState axes[3] = {0};
	int axis = joy_get_axis_state(ev, axes);

	// Honestly, this is even worse than the button function
	if (axis == config->axes.throttle.axis_index) {
		state->throttle = linmap_axis(
			config->axes.throttle.value_index ? axes[axis].y : axes[axis].x,
			config->axes.throttle.lin_map_min,
			config->axes.throttle.lin_map_max
		);

		if (config->axes.throttle.reversed) {
			state->throttle = config->axes.throttle.lin_map_max - state->throttle;
		}

		if (config->axes.throttle.curved) {
			state->throttle = qbezier_axis(state->throttle, &config->axes.throttle);
		}
	}

	if (axis == config->axes.steering.axis_index) {
		state->steering = linmap_axis(
			config->axes.steering.value_index ? axes[axis].y : axes[axis].x,
			config->axes.steering.lin_map_min,
			config->axes.steering.lin_map_max
		);

		if (config->axes.steering.reversed) {
			state->steering = config->axes.steering.lin_map_max - state->steering;
		}

		if (config->axes.steering.curved) {
			state->steering = qbezier_axis(state->steering, &config->axes.steering);
		}
	}

	if (axis == config->axes.brake.axis_index) {
		state->brake = linmap_axis(
			config->axes.brake.value_index ? axes[axis].y : axes[axis].x,
			config->axes.brake.lin_map_min,
			config->axes.brake.lin_map_max
		);

		if (config->axes.brake.reversed) {
			state->brake = config->axes.brake.lin_map_max - state->brake;
		}

		if (config->axes.brake.curved) {
			state->brake = qbezier_axis(state->brake, &config->axes.brake);
		}
	}

	if (axis == config->axes.dpad_horiz.axis_index) {
		state->dpad_horiz = linmap_axis(
			config->axes.dpad_horiz.value_index ? axes[axis].y : axes[axis].x,
			config->axes.dpad_horiz.lin_map_min,
			config->axes.dpad_horiz.lin_map_max
		);

		if (config->axes.dpad_horiz.reversed) {
			state->dpad_horiz = config->axes.dpad_horiz.lin_map_max - state->dpad_horiz;
		}
	}

	if (axis == config->axes.dpad_vert.axis_index) {
		state->dpad_vert = linmap_axis(
			config->axes.dpad_vert.value_index ? axes[axis].y : axes[axis].x,
			config->axes.dpad_vert.lin_map_min,
			config->axes.dpad_vert.lin_map_max
		);

		if (config->axes.dpad_vert.reversed) {
			state->dpad_vert = config->axes.dpad_vert.lin_map_max - state->dpad_vert;
		}
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
