#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <atomic>
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

static std::atomic<Steering_InputState> state;
static std::atomic<ControllerConfig> config;
static std::atomic<InitState> input_init_state(HOLD);
static std::atomic<int> lastInputStatus(0);

void log_input(int input_status) {
	if (input_status != 0) printf("-------------------------------------\n");

	Steering_InputState state_data = state.load();

	if (input_status == 1) {
		printf("Paddle left: %i\n", state_data.paddle_left);
		printf("Paddle right: %i\n", state_data.paddle_right);
	} else if (input_status == 2) {
		printf("Steering state: %i\n", state_data.steering);
		printf("Throttle state: %i\n", state_data.throttle);
		printf("Brake state: %i\n", state_data.brake);
		printf("DPad-L/R: %i\n", state_data.dpad_horiz);
		printf("DPad-D/U: %i\n", state_data.dpad_vert);
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

void fork_input_read(int js_fd) {
	bool logged_init_info = false;
	ControllerConfig config_data = config.load();

	while (true) {
		Steering_InputState curState = state.load();
		int input_status = steering_check_event(js_fd, &config_data, &curState);
		InitState cur_init_state = input_init_state.load();

		if (cur_init_state != DONE) {
			if (cur_init_state == HOLD) {
				if (!logged_init_info) {
					printf("Hold down both throttle and brakes simultaneously!\n");
					logged_init_info = true;
				}

				if (curState.throttle != 0 && curState.brake != 0) {
					input_init_state = RELEASE;
					logged_init_info = false;
				}
			} else if (cur_init_state == RELEASE) {
				if (!logged_init_info) {
					printf("Release both pedals!\n");
					logged_init_info = true;
				}

				if (curState.throttle == 0 && curState.brake == 0) {
					input_init_state = DONE;
					printf("Initialization done!\n");
				}
			}

			continue;
		}

		state = curState;
		lastInputStatus = input_status;
	}
}

void fork_send_cmd(int motor_fd) {
	int refresh_millis = 100;
	int refresh_start = 0;
	ControllerConfig config_data = config.load();

	while (true) {
		if (input_init_state.load() != DONE) continue;

		if (
			get_cur_millis() - refresh_start > refresh_millis
			|| lastInputStatus.load() > 0
		) {
			Steering_InputState curState = state.load();
			int velocity = curState.brake < config_data.brake_threshold ? 0 : curState.throttle;
			log_state(velocity, curState.steering);
			motor_send_command(motor_fd, curState.steering, velocity);

			refresh_start = get_cur_millis();
		}
	}
}

int main() {
	ControllerConfig config_val = config.load();

	config_load("ctrlconfig.toml", &config_val);

	int js = open(config_val.joy_path, O_RDONLY);
	int motor = open(config_val.motor_port, O_RDWR);
	
	if (motor_init(motor, config_val.motor_baudrate) == -1) {
		log_error("Failed to initialize motor serial port!");
		return 0;
	}

	printf("Connected to controller (Baudrate: %i)\n", config_val.motor_baudrate);
	Motor_Feedback feedback;
	// init_state(&state);

	std::thread inputThread(fork_input_read, js);
	std::thread cmdThread(fork_send_cmd, motor);

	inputThread.join();
	cmdThread.join();

	return 0;
}