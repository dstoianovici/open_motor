#! /usr/bin/python3

#Testing serial comms to teensy


# import serial as s
# from time import sleep



baudRate = 115200
timeout = 0.5
port = "/dev/ttyACM0"

import open_motor_serial
import json
import serial
import time

comms = open_motor_serial.serial_communicator(port,baudRate,timeout)

def main():
    while True:
        comms.send_pwm_goal(100,0,100,100)
        print("Response:" + comms.get_response())
        time.sleep(0.5)

        comms.send_pwm_goal(0,0,0,0)
        print("Response:" + comms.get_response())
        time.sleep(0.5)

        comms.send_pwm_goal(-100,0,-100,-100)
        print("Response:" + comms.get_response())
        time.sleep(0.5)

        comms.send_pwm_goal(0,0,0,0)
        print("Response:" + comms.get_response())
        time.sleep(0.5)

        


if __name__ == "__main__":
    main()

