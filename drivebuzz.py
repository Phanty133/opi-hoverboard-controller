import time

GPIO_PATH = "/sys/class/gpio/gpio13"

def write_gpio(val: int):
	with open(f"{GPIO_PATH}/value", "w") as f:
		f.write(str(val))

def beep(time_ms: int):
	write_gpio(1)
	time.sleep(time_ms / 1000)
	write_gpio(0)	

def main():
	beep(100)
	time.sleep(0.1)

if __name__ == "__main__":
	main()