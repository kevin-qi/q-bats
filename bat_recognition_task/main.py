# Importing Libraries
import serial
import time
import sys
import json
import threading
import queue
from pathlib import Path
import os
"""
arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)
line = ''
"""

def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    data = arduino.readline()
    return data

exp_config = {}
exp_name = input("Experiment Name: ")
exp_dir = "experiments/{}".format(exp_name)
Path(exp_dir).mkdir(parents=True, exist_ok=True)
if(Path(os.path.join(exp_dir, "config.json")).is_file()):
    with open(os.path.join(exp_dir, "config.json"), 'r') as f:
        exp_config = json.loads(f.read())
else:
    exp_config['bats'] = {}
    num_bats = int(input("Number of stimuli bats: "))
    for i in range(num_bats):
        bat_id = input("Bat {} ID: ".format(i))
        feeder = None
        while not feeder in set(['p','q']):
            feeder = str(input("Set corresponding feeder (P, Q): ")).lower()
            if not feeder in set(['p','q']):
                print("Please select either P or Q!")
        exp_config['bats'][bat_id] = feeder
    exp_config['num_bats'] = num_bats
    exp_config['exp_name'] = exp_name
    exp_config['exp_dir'] = os.path.abspath(exp_dir)
    with open(os.path.join(exp_dir, "config.json"), 'w') as f:
        json.dump(exp_config, f)
    
sess_name = input("Session Name: ")
exp_logs = open('{}/{}_logs.txt'.format(exp_dir, sess_name), 'a+')

def read_kbd_input(inputQueue):
    print('Ready for keyboard input:')
    while (True):
        input_str = input()
        inputQueue.put(input_str)

"""
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
        arduino.write(bytes(bat_id, 'utf-8'))"""



def main():
    EXIT_COMMAND = "exit"
    inputQueue = queue.Queue()

    inputThread = threading.Thread(target=read_kbd_input, args=(inputQueue,), daemon=True)
    inputThread.start()
    i = 0
    while (True):
        if (inputQueue.qsize() > 0):
            input_str = inputQueue.get()
            print("input_str = {}".format(input_str))
            exp_logs.write(input_str+'\n')
            if (input_str == EXIT_COMMAND):
                print("Exiting serial terminal.")
                exp_logs.close()
                break
            # Insert your code here to do whatever you want with the input_str.

        # The rest of your program goes here.
        exp_logs.write(str(i)+'\n')
        i+=1
        time.sleep(0.01) 
    print("End.")

if (__name__ == '__main__'): 
    try:
        main()
    except KeyboardInterrupt:
        exp_logs.close()


    #num = input("Enter a number: ") # Taking input from user
    #value = write_read(num)
    #print(value) # printing the value

