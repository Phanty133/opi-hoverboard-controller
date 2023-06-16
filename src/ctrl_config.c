#include "ctrl_config.h"

void read_axis(Config_Axis* config_axis, toml_table_t* axis_table, const char* axis_name) {
	toml_table_t* axis = toml_table_in(axis_table, axis_name);
	config_axis->axis_index = toml_int_in(axis, "axis").u.b;
	config_axis->value_index = toml_int_in(axis, "value").u.b;
	config_axis->lin_map_min = toml_int_in(axis, "lin_map_min").u.b;
	config_axis->lin_map_max = toml_int_in(axis, "lin_map_max").u.b;
	config_axis->reversed = toml_bool_in(axis, "reversed").u.b;
	config_axis->curved = toml_bool_in(axis, "curved").u.b;

	if (config_axis->curved) {
		config_axis->qbezier_x = toml_double_in(axis, "qbezier_x").u.d;
		config_axis->qbezier_y = toml_double_in(axis, "qbezier_y").u.d;
	}
}

int config_load(const char* config_path, ControllerConfig* config) {
	FILE* fp = fopen(config_path, "r");

	if (!fp) {
		log_error("Failed to open config file");
		return -1;
	}

	char errbuf[200];
	toml_table_t* conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
	fclose(fp);

	if (!conf) {
		log_error("Failed to parse config file");
		return -1;
	}

	toml_table_t* joystick_table = toml_table_in(conf, "joystick");
	toml_datum_t joy_path = toml_string_in(joystick_table, "path");
	strcpy(config->joy_path, joy_path.u.s);
	free(joy_path.u.s);

	toml_table_t* joymap = toml_table_in(conf, "joymap");
	toml_table_t* axis = toml_table_in(joymap, "axis");
	read_axis(&(config->axes.throttle), axis, "throttle");
	read_axis(&(config->axes.brake), axis, "brake");
	read_axis(&(config->axes.steering), axis, "steering");
	read_axis(&(config->axes.dpad_horiz), axis, "dpad_horiz");
	read_axis(&(config->axes.dpad_vert), axis, "dpad_vert");

	toml_table_t* buttons = toml_table_in(joymap, "buttons");
	config->buttons.cross = toml_int_in(buttons, "cross").u.b;
	config->buttons.square = toml_int_in(buttons, "square").u.b;
	config->buttons.circle = toml_int_in(buttons, "circle").u.b;
	config->buttons.triangle = toml_int_in(buttons, "triangle").u.b;
	config->buttons.paddle_right = toml_int_in(buttons, "paddle_right").u.b;
	config->buttons.paddle_left = toml_int_in(buttons, "paddle_left").u.b;
	config->buttons.r2 = toml_int_in(buttons, "r2").u.b;
	config->buttons.l2 = toml_int_in(buttons, "l2").u.b;
	config->buttons.select = toml_int_in(buttons, "select").u.b;
	config->buttons.start = toml_int_in(buttons, "start").u.b;
	config->buttons.r3 = toml_int_in(buttons, "r3").u.b;
	config->buttons.l3 = toml_int_in(buttons, "l3").u.b;
	config->buttons.shift_down = toml_int_in(buttons, "shift_down").u.b;
	config->buttons.shift_up = toml_int_in(buttons, "shift_up").u.b;

	toml_table_t* motor = toml_table_in(conf, "motor");
	config->motor_baudrate = toml_int_in(motor, "baudrate").u.b;
	
	toml_datum_t motor_serial_port = toml_string_in(motor, "serial_port");
	strcpy(config->motor_port, motor_serial_port.u.s);
	free(motor_serial_port.u.s);

	toml_free(conf);
	return 0;
}