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

is_trial = False

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
        bat_id = str(input("Bat {} ID: ".format(i)))
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
    global is_trial
    while (True):
        if(not is_trial):
            input_str = ""
            print('\n')
            while input_str not in exp_config['bats'].keys()  and not input_str.lower() == 'exit':
                print('\nInput Next bat id {}:'.format(exp_config['bats']))
                input_str = input()
                if input_str not in exp_config['bats'].keys() and not input_str.lower() == 'exit':
                    print("Invalid bat ID!")
            if(not input_str.lower() == 'exit'):
                print("Bat selection complete. Please present bat {}".format(input_str))
                print("Target Feeder: {}".format(exp_config['bats'][input_str].upper()))
            inputQueue.put(input_str)
            is_trial = True

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
    # Bool flag true during trial, false between trials
    global is_trial
    is_trial = False

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
            # arduino.write(bytes(input_str, 'utf-8'))
        # The rest of your program goes here.
        exp_logs.write(str(i)+'\n')
        i+=1
        if(i % 300 == 0):
            is_trial = False
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

