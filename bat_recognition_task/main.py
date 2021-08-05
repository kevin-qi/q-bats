# Importing Libraries
import serial
import time
import sys
arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)
line = ''



def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    data = arduino.readline()
    return data

exp_name = input("Experiment Name:")
exp_logs = open('{}_logs.txt'.format(exp_name), 'a')

while True:
    msg = arduino.readline().decode('utf-8')
    if(msg != ''):
        line += msg
        msg = ''
    if('\n' in line):
        print(line)
        exp_logs.write(line)
        sys.stdout.flush()
        line = ''

        bat_id = input("Enter bat index")
        arduino.write(bytes(bat_id, 'utf-8'))


    #num = input("Enter a number: ") # Taking input from user
    #value = write_read(num)
    #print(value) # printing the value

exp_logs.close()