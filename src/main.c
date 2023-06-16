#include <stdio.h>
#include <fcntl.h>
#include "ctrl_config.h"
#include "steering.h"
#include "motor.h"
#include "logger.h"

static Steering_InputState state;
static ControllerConfig config;

typedef enum {
	HOLD,
	RELEASE,
	DONE
} InitState;

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

void log_state(int velocity, int steering) {
	printf("-------------------------\n");
	printf("Target velocity: %i\n", velocity);
	printf("Steering: %i\n", steering);
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
	init_state(&state);
	InitState input_init_state = HOLD;
	bool logged_init_info = false;

	while(true) {
		int input_status = steering_check_event(js, &config, &state);
		// log_input(input_status);

		if (input_init_state != DONE) {
			if (input_init_state == HOLD) {
				if (!logged_init_info) {
					printf("Hold down both throttle and brakes simultaneously!\n");
					logged_init_info = true;
				}

				if (state.steering != 0 && state.brake != 0) {
					input_init_state = RELEASE;
					logged_init_info = false;
				}
			} else if (input_init_state == RELEASE) {
				if (!logged_init_info) {
					printf("Release both pedals!\n");
					logged_init_info = true;
				}

				if (state.steering == 0 && state.brake == 0) {
					input_init_state = DONE;
					printf("Initialization done!\n");
				}
			}

			continue;
		}

		int velocity = state.brake < config.brake_threshold ? 0 : state.throttle;
		log_state(velocity, state.steering);
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