# Importing Libraries
import serial
import time
import sys
import json
import threading
import queue
from pathlib import Path
import os
import re

is_trial = False

def write_read(x):
    arduino.write(bytes(x, 'utf-8'))
    data = arduino.readline()
    return data

"""
Arduino Serial Connnection
"""
arduino = serial.Serial(port='COM9', baudrate=115200, timeout=.1)
line = ''


"""
Experiment Configuration
"""
exp_config = {}
exp_name = input("Experiment Name: ") # Experiment name
exp_dir = "experiments/{}".format(exp_name) # Experiment directory
Path(exp_dir).mkdir(parents=True, exist_ok=True) # Make directory if does not exist
if(Path(os.path.join(exp_dir, "config.json")).is_file()): # Reload config file if experiment exists
    with open(os.path.join(exp_dir, "config.json"), 'r') as f:
        exp_config = json.loads(f.read())
else: # Configure new experiment if does not exist
    exp_config['bats'] = {}
    num_bats = int(input("Number of stimuli bats: "))
    for i in range(num_bats): # For each bat, input bat ID and corresponding feeder
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
    with open(os.path.join(exp_dir, "config.json"), 'w') as f: # Save config file to config.json
        json.dump(exp_config, f)
    
sess_name = input("Session Name: ") # Enter session name
exp_logs = open('{}/{}_logs.txt'.format(exp_dir, sess_name), 'a+') # Load session logs (append)

def read_kbd_input(inputQueue):
    """
    Read key board inputs in between trials
    Run this on a seperate thread, not on the main thread!
    """
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
        time.sleep(0.5)

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
    line = ""

    EXIT_COMMAND = "exit"

    # Initialize queue for inter-thread communication
    inputQueue = queue.Queue()
    # Read keyboard inputs on seperate thread so that it does not block the rest of the program
    inputThread = threading.Thread(target=read_kbd_input, args=(inputQueue,), daemon=True)
    inputThread.start()
    while (True):
        # Dequeue from inputQueue to get keyboard inputs
        if (inputQueue.qsize() > 0):
            input_str = inputQueue.get()
            print("input_str = {}".format(input_str))
            exp_logs.write(input_str+'\n') # Log to session log file



            if (input_str == EXIT_COMMAND): # Safe quit
                print("Exiting serial terminal.")
                exp_logs.close()
                break

            # Insert your code here to do whatever you want with the input_str.
            arduino.write(bytes(input_str+'\n', 'utf-8')) # Send bat ID
            print(input_str)
            #is_trial = True

        # The rest of your program goes here.
        msg = arduino.readline().decode('utf-8')
        line += msg
        if(msg != ''):
            print(msg)
            if('RESET' in msg):
                print('resetting')
                is_trial = False
            sys.stdout.flush()
            msg = ''
        time.sleep(0.01)

        if("\n" in line):
            print(line)
            TTL_timestamps = re.findall(r"(\|\d\d*\|)", line)
            if(TTL_timestamps != None):
                print(TTL_timestamps)
                for ts in TTL_timestamps:
                    exp_logs.write(ts+'\n')

            events = re.findall(r"(\w*:\d*\|)", line)
            if(events != None):
                exp_logs.write(''.join(events))
            line = ""
    print("End.")

if (__name__ == '__main__'): 
    try:
        main()
    except KeyboardInterrupt:
        exp_logs.close()


    #num = input("Enter a number: ") # Taking input from user
    #value = write_read(num)
    #print(value) # printing the value

