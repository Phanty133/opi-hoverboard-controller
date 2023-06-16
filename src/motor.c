#include "motor.h"

int motor_init(int serial_port_fd, int baudrate) {
	struct termios tty;

	if(tcgetattr(serial_port_fd, &tty) != 0) {
		// printf("ERROR: %i from tcgetattr: %s\n", errno, strerror(errno));
		log_error("tcgetattr failed");
		return -1;
	}

	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ECHO; // Disable echo
	tty.c_lflag &= ~ECHOE; // Disable erasure
	tty.c_lflag &= ~ECHONL; // Disable new-line echo
	tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

	tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;

	cfsetispeed(&tty, baudrate);
	cfsetospeed(&tty, baudrate);

	// Save tty settings, also checking for error
	if (tcsetattr(serial_port_fd, TCSANOW, &tty) != 0) {
		log_error("tcsetattr failed");
		return -1;
	}
}

void motor_send_command(int serial_port_fd, int16_t steer, int16_t speed) {
	Motor_Command command;

	command.start    = (uint16_t)MOTOR_START_FRAME;
	command.steer    = (int16_t)steer;
	command.speed    = (int16_t)speed;
	command.checksum = (uint16_t)(command.start ^ command.steer ^ command.speed);

	// unsigned const char data[] = "\xcd\xab\x00\x00\x64\x00\xa9\xab";
	write(serial_port_fd, (uint8_t*)&command, sizeof(command));
	tcdrain(serial_port_fd);
}

bool motor_receive(int serial_port_fd, Motor_Feedback* feedback) {
	static uint8_t incoming_byte_prev;
	static uint16_t buf_start_frame;
	static uint8_t data_index;
	int available;

	if (ioctl(serial_port_fd, FIONREAD, &available ) < 0 ) {
		log_error("Failed to get motor port available data");
		return false;
	}

	if (available == 0) return false;

	bool data_valid = false;
	bool start_frame_loaded = false;

	while (available-- > 0) {
		uint8_t incoming_byte;
	
		if (read(serial_port_fd, &incoming_byte, 1) == -1) {
			log_error("Failed to read motor serial port");
		}

		buf_start_frame = ((uint16_t)(incoming_byte) << 8) | incoming_byte_prev;

		if (start_frame_loaded) {
			((uint8_t*)feedback)[data_index++] = incoming_byte;

			if (data_index == sizeof(Motor_Feedback)) {
				data_valid = true;
				break;
			}
		} else if (buf_start_frame == MOTOR_START_FRAME) {
			data_index = 2;
			start_frame_loaded = true;
		}
	}

	if (data_valid) {
		uint16_t checksum = (uint16_t)(
			feedback->start ^ feedback->cmd1 ^ feedback->cmd2
			^ feedback->speedR_meas ^ feedback->speedL_meas
			^ feedback->batVoltage ^ feedback->boardTemp ^ feedback->cmdLed
		);

		if (!(feedback->start == MOTOR_START_FRAME && feedback->checksum == checksum)) {
			data_valid = false;
		}
	}

	return data_valid;
}
