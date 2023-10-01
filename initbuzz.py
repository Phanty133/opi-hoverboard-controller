GPIO_PATH = "/sys/class/gpio/gpio13"

def write_gpio(val: int):
	with open(f"{GPIO_PATH}/value", "w") as f:
		f.write(str(val))

def beep(time_ms: int):
	write_gpio(1)
	sleep(time_ms)
	write_gpio(0)	

def main():
	beep(100)
	sleep(100)
	beep(100)

if __name__ == "__main__":
	main()