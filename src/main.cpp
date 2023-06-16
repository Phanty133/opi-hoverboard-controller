#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <mutex>
#include <thread>
#include "ctrl_config.h"
#include "steering.h"
#include "motor.h"
#include "logger.h"

typedef enum {
	HOLD,
	RELEASE,
	DONE
} InitState;

typedef struct {
	InitState input_init_state;
	int last_input_status;
	Steering_InputState state;
} ProgState;

static ProgState prog_state;
static std::mutex state_mutex;

void log_input(int input_status, Steering_InputState* state) {
	if (input_status != 0) printf("-------------------------------------\n");

	if (input_status == 1) {
		printf("Paddle left: %i\n", state->paddle_left);
		printf("Paddle right: %i\n", state->paddle_right);
	} else if (input_status == 2) {
		printf("Steering state: %i\n", state->steering);
		printf("Throttle state: %i\n", state->throttle);
		printf("Brake state: %i\n", state->brake);
		printf("DPad-L/R: %i\n", state->dpad_horiz);
		printf("DPad-D/U: %i\n", state->dpad_vert);
	}
}

void log_state(int velocity, int steering) {
	printf("-------------------------\n");
	printf("Target velocity: %i\n", velocity);
	printf("Steering: %i\n", steering);
}

unsigned long long get_cur_millis() {
	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long millis =
		(unsigned long long)(tv.tv_sec) * 1000 +
		(unsigned long long)(tv.tv_usec) / 1000;

	return millis;
}

void fork_input_read(int js_fd, ControllerConfig config) {
	bool logged_init_info = false;
	ProgState cur_prog_state;

	while (true) {
		{
			std::lock_guard<std::mutex> lock(state_mutex);
			cur_prog_state = prog_state;
		}

		int input_status = steering_check_event(js_fd, &config, &cur_prog_state.state);

		if (cur_prog_state.input_init_state != DONE) {
			if (cur_prog_state.input_init_state == HOLD) {
				if (!logged_init_info) {
					printf("Hold down both throttle and brakes simultaneously!\n");
					logged_init_info = true;
				}

				if (cur_prog_state.state.throttle != 0 && cur_prog_state.state.brake != 0) {
					cur_prog_state.input_init_state = RELEASE;
					logged_init_info = false;
				}
			} else if (cur_prog_state.input_init_state == RELEASE) {
				if (!logged_init_info) {
					printf("Release both pedals!\n");
					logged_init_info = true;
				}

				if (cur_prog_state.state.throttle == 0 && cur_prog_state.state.brake == 0) {
					cur_prog_state.input_init_state = DONE;
					printf("Initialization done!\n");
				}
			}
		}

		{
			std::lock_guard<std::mutex> lock(state_mutex);
			prog_state = cur_prog_state;
		}
	}
}

void fork_send_cmd(int motor_fd, ControllerConfig config) {
	int refresh_millis = 100;
	int refresh_start = 0;

	ProgState cur_prog_state;
	printf("Brake threshold: %i\n", config.brake_threshold);

	while (true) {
		{
			std::lock_guard<std::mutex> lock(state_mutex);
			cur_prog_state = prog_state;
		}

		if (cur_prog_state.input_init_state != DONE) continue;

		if (
			get_cur_millis() - refresh_start > refresh_millis
			|| cur_prog_state.last_input_status > 0
		) {
			int velocity = cur_prog_state.state.brake < config.brake_threshold
				? 0 : cur_prog_state.state.throttle;
			log_state(velocity, cur_prog_state.state.steering);
			motor_send_command(motor_fd, cur_prog_state.state.steering, velocity);

			refresh_start = get_cur_millis();
		}
	}
}

int main() {
	ControllerConfig config;
	config_load("ctrlconfig.toml", &config);

	int js = open(config.joy_path, O_RDONLY);
	int motor = open(config.motor_port, O_RDWR);
	
	if (motor_init(motor, config.motor_baudrate) == -1) {
		log_error("Failed to initialize motor serial port!");
		return 0;
	}

	printf("Connected to controller (Baudrate: %i)\n", config.motor_baudrate);
	Motor_Feedback feedback;
	// init_state(&state);

	std::thread inputThread(fork_input_read, js, config);
	std::thread cmdThread(fork_send_cmd, motor, config);

	inputThread.join();
	cmdThread.join();

	return 0;
}