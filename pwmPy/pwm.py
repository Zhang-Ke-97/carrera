import RPi.GPIO as GPIO

L298N_IN1 = 24
L298N_IN2 = 23
L298N_ENA = 25

dutyCycle = 10

GPIO.setmode(GPIO.BCM)
GPIO.setup(L298N_IN1, GPIO.OUT)
GPIO.setup(L298N_IN2, GPIO.OUT)
GPIO.setup(L298N_ENA, GPIO.OUT)

GPIO.output(L298N_IN1, GPIO.LOW)
GPIO.output(L298N_IN2, GPIO.LOW)

p = GPIO.PWM(L298N_ENA, 10) # GPIO.PWM(channel, frequency)
p.start(dutyCycle) # set duty_cycle from 0 to 100

print("Enter \' run \' to start, \'+ \' to increase the speed, \'-\' to decrease the speed, \'stop\' to stop")

while(1):
    command = input("Enter your command: ")
    if command == 'run':
        GPIO.output(L298N_IN1, GPIO.HIGH)
        GPIO.output(L298N_IN2, GPIO.LOW)
    elif command == '+':
        dutyCycle = dutyCycle + 10
        p.ChangeDutyCycle(dutyCycle)
        print("duty cycle increased to " + dutyCycle)
    elif command == '-':
        dutyCycle = dutyCycle - 10
        p.ChangeDutyCycle(dutyCycle)   
        print("duty cycle decreased to " + dutyCycle)
    elif command == 'stop':
        GPIO.output(L298N_IN1, GPIO.LOW)
        GPIO.output(L298N_IN2, GPIO.LOW)
    else:
        print("invalid command")
