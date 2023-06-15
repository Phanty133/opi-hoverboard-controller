#!/bin/zsh

# I couldn't be bothered to write a makefile

gcc -Iinc -o opi-controller.out \
	src/main.c \
	src/motor.c \
	src/joystick.c \
	src/steering.c \
	src/toml.c \
	src/ctrl_config.c \
	src/logger.c