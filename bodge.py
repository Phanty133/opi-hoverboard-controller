import serial

port = serial.Serial("/dev/ttyS2", 115200)
port.write(b"\xcd\xab\x00\x00\x64\x00\xa9\xab")