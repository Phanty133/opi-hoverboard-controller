#!/bin/zsh

# I couldn't be bothered to write a makefile

mkdir build
gcc -Iinc -c -o build/toml.o src/toml.c
g++ -Iinc -pthread -std=c++14 -o opi-controller.out \
	build/toml.o \
	src/logger.c \
	src/motor.c \
	src/joystick.c \
	src/steering.c \
	src/ctrl_config.c \
	src/main.cpp

rm -r build