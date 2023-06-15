#include <stdio.h>
#include <fcntl.h>
#include "ctrl_config.h"
#include "steering.h"
#include "motor.h"
#include "logger.h"

static Steering_InputState state;
static ControllerConfig config;

int main() {
	config_load("ctrlconfig.toml", &config);

	int js = open(config.joy_path, O_RDONLY);
	int motor = open(config.motor_port, O_RDWR);
	
	if (motor_init(motor, config.motor_baudrate) == -1) {
		log_error("Failed to initialize motor serial port!");
		return 0;
	}

	while(true) {
		int input_status = steering_check_event(js, &config, &state);

		if (input_status == -1) break;
		
		if (input_status != 0) printf("-------------------------------------\n");

		if (input_status == 1) {
			printf("Paddle left: %i\n", state.paddle_left);
			printf("Paddle right: %i\n", state.paddle_right);
		} else if (input_status == 2) {
			printf("Steering state: %i\n", state.steering);
			printf("Throttle state: %i\n", state.throttle);
			printf("Brake state: %i\n", state.brake);
			printf("DPad-L/R: %i\n", state.dpad_horiz);
			printf("DPad-D/U: %i\n", state.dpad_vert);
		}
	}

	return 0;
}