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
import soundcard as sc
import serial
import serial.tools.list_ports
import pdb

is_trial = False

def controlled_input(msg, valid_inputs):
    input_str = ""
    while not input_str in valid_inputs:
        input_str = str(input(msg)).lower()
        if not input_str in valid_inputs:
            print("Please input valid option: {}".format(valid_inputs))
    return input_str




"""
Arduino Serial Connnection
"""
used_ports = serial.tools.list_ports.comports(include_links=False)
if len(used_ports)>1:
    print("More than one used port, manually inspect for arduino port")
    arduino_port='COM9' 
else:
    arduino_port= used_ports[0].device; 
arduino = serial.Serial(port=arduino_port, baudrate=115200, timeout=.1)
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
    num_bats = int(controlled_input("Number of stimuli bats: ", ["1", "2", "3", "4", "5"]))
    for i in range(num_bats): # For each bat, input bat ID and corresponding feeder
        bat_id = str(input("Bat {} ID: ".format(i)))
        feeder = controlled_input("Select feeder (p1, q1): ", ["p1", "q1"])
        exp_config['bats'][bat_id] = feeder
    exp_config['num_bats'] = num_bats
    exp_config['exp_name'] = exp_name
    exp_config['exp_dir'] = os.path.abspath(exp_dir)
    with open(os.path.join(exp_dir, "config.json"), 'w') as f: # Save config file to config.json
        json.dump(exp_config, f)
    
sess_name = input("Session Name: ") # Enter session name
sess_dir = os.path.join(exp_dir, sess_name)
Path(sess_dir).mkdir(parents=True, exist_ok=True) # Make directory if does not exist
record_audio_flag = input("Record audio? (y/n): ")
if record_audio_flag.lower() == 'y':
    os.system("start cmd /c python record_MOTU.py {} {}".format(exp_name, sess_name)) 
exp_logs = open('{}/{}/{}_logs.txt'.format(exp_dir, sess_name, sess_name), 'a+') # Load session logs (append)
sess_type = controlled_input("Session type (train, test): ", ["train", "test"])
exp_logs.write(sess_type + '\n')
def read_kbd_input(inputQueue):
    """
    Read key board inputs in between trials
    Run this on a seperate thread, not on the main thread!
    """
    global is_trial
    while (True):
        if(sess_type == 'train'):
            input_str = ""
            input_str = input()
            if input_str == "deliver P1":
                #print("Manual deliver reward P1")
                inputQueue.put(input_str)
            elif input_str == "deliver Q1":
                #print("Manual deliver reward Q1")
                inputQueue.put(input_str)
            if input_str == "bait P1":
                #print("Manual deliver reward P1")
                inputQueue.put(input_str)
            elif input_str == "bait Q1":
                #print("Manual deliver reward Q1")
                inputQueue.put(input_str)
            elif input_str == "retract P1":
                print("Retract feeder P1")
                inputQueue.put(input_str)
            elif input_str == "retract Q1":
                print("Retract feeder Q1")
                inputQueue.put(input_str)
        if(sess_type == 'test'):
            if(not is_trial):
                input_str = ""
                print('\n')
                while input_str not in exp_config['bats'].keys()  and not input_str.lower() == 'exit' and not input_str == "*":
                    print('\nInput Next bat id {}:'.format(exp_config['bats']))
                    input_str = input()
                    if input_str not in exp_config['bats'].keys() and not input_str.lower() == 'exit' and not input_str == "*":
                        print("Invalid bat ID!")
                if(not input_str.lower() == 'exit'):
                    if(input_str == "*"):
                        print("Bat selection complete. Please present * bat.")
                        print("Target Feeder: *")
                    else:
                        print("Bat selection complete. Please present bat {}".format(input_str))
                        print("Target Feeder: {}".format(exp_config['bats'][input_str].upper()))
                        inputQueue.put(exp_config['bats'][input_str].upper())
                if(input_str == "*"):
                    inputQueue.put(input_str)
                elif(not input_str == "exit"):
                    inputQueue.put(exp_config['bats'][input_str])
                elif input_str == "retract":
                    print("Retract feeders")
                    inputQueue.put(input_str)
                elif input_str == "deliver P1":
                    print("Manual deliver reward P1")
                    inputQueue.put(input_str)
                elif input_str == "deliver Q1":
                    print("Manual deliver reward Q1")
                    inputQueue.put(input_str)
                elif input_str == "door":
                    print("Opening Door")
                    inputQueue.put(input_str)
                is_trial = True
            elif(is_trial):
                input_str = input()
                if input_str == "next":
                    print("Manual triggerring beam break. Advance to next state")
                    inputQueue.put(input_str)
                elif input_str == "reset":
                    #print("Manual reset command")
                    inputQueue.put(input_str)
                elif input_str == "deliver P1":
                    print("Manual deliver reward P1")
                    inputQueue.put(input_str)
                elif input_str == "deliver Q1":
                    print("Manual deliver reward Q1")
                    inputQueue.put(input_str)
                elif input_str == "door":
                    print("Opening Door")
                    inputQueue.put(input_str)
                elif input_str == "retract P1":
                    print("Retract feeder P1")
                    inputQueue.put(input_str)
                elif input_str == "retract Q1":
                    print("Retract feeder Q1")
                    inputQueue.put(input_str)
        time.sleep(0.5)

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
            exp_logs.write('Command: ['+input_str+']\n') # Log to session log file

            if (input_str == EXIT_COMMAND): # Safe quit
                print("Exiting serial terminal.")
                exp_logs.close()
                break

            # Insert your code here to do whatever you want with the input_str.
            arduino.write(bytes(input_str+'\n', 'utf-8')) # Send bat ID

        # The rest of your program goes here.
        if(arduino.inWaiting() > 0):
            inByte = arduino.read(1).decode('utf-8')
            line += inByte
            if(inByte == '!'): # Received complete TTL timestamp
                exp_logs.write(line)
                #print(line)
                #sys.stdout.flush()
                line = ''
            elif inByte == "|":
                exp_logs.write(line+'\n')

                if('RESET' in line):
                    print('Trial Reset')
                    is_trial = False

                print(line)
                sys.stdout.flush()
                line = ''
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

