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
