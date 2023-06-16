#include <stdio.h>
#include <fcntl.h>
#include "ctrl_config.h"
#include "steering.h"
#include "motor.h"
#include "logger.h"

static Steering_InputState state;
static ControllerConfig config;

void log_input(int input_status) {
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

int main() {
	config_load("ctrlconfig.toml", &config);

	int js = open(config.joy_path, O_RDONLY);
	int motor = open(config.motor_port, O_RDWR);
	
	if (motor_init(motor, config.motor_baudrate) == -1) {
		log_error("Failed to initialize motor serial port!");
		return 0;
	}

	printf("Connected to controller (Baudrate: %i)\n", config.motor_baudrate);
	Motor_Feedback feedback;

	while(true) {
		int input_status = steering_check_event(js, &config, &state);
		log_input(input_status);

		int velocity = state.brake < config.brake_threshold ? 0 : state.throttle;
		motor_send_command(motor, state.steering, velocity);

		if (motor_receive(motor, &feedback)) {
			printf("Received motor data:\n");
			printf("SpeedR: %i\nSpeedL: %i\nBatVoltage: %i\n",
				feedback.speedR_meas, feedback.speedL_meas, feedback.batVoltage
			);
		}
	}

	return 0;
}