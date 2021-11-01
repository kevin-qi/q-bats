import numpy as np
import matplotlib.pyplot as plt
import pyaudio
import pathlib
from queue import Queue
import threading
import time
import sys
import signal
import os

frame_queue = Queue()

args = sys.argv[1:]
exp_name = args[0]
sess_name = args[1]
print("Recording audio for experiment: {}, session: {}".format(exp_name, sess_name))
pathlib.Path('experiments/{}/{}/{}'.format(exp_name, sess_name,'motu')).mkdir(parents=True,exist_ok=True)
sys.stdout.flush()

def main():
    global frame_queue

    RATE = 44100
    CHUNK_TIME = 10 # sec
    CHUNK = RATE*CHUNK_TIME
    FORMAT = pyaudio.paFloat32
    CHANNELS = 2

    global p
    p = pyaudio.PyAudio()
    global stream_1_2
    global stream_3_4

    stream_1_2 = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    input_device_index=4,
                    frames_per_buffer=CHUNK)

    stream_3_4 = p.open(format=FORMAT,
                    channels=CHANNELS,
                    rate=RATE,
                    input=True,
                    input_device_index=3,
                    frames_per_buffer=CHUNK)

    frames_1_2 = []
    frames_3_4 = []

    while True:
        # read is blocking, but the stream records continuously once it is opened. 
        # The first read will take ~RECORD_SECONDS, the 2nd read will only take <1 sec because
        # the buffer is already populated while the previous read was processing.

        # Record CHUNK from mic 1-2
        data_1_2 = stream_1_2.read(CHUNK)
        # Record CHUNK from mic 3-4
        data_3_4 = stream_3_4.read(CHUNK)

        # Convert buffer to numpy
        data_1_2 = np.frombuffer(data_1_2, np.float32)
        data_3_4 = np.frombuffer(data_3_4, np.float32)

        # Reshape data
        data_1_2 = np.reshape(data_1_2, (-1,2))
        data_3_4 = np.reshape(data_3_4, (-1,2))

        # Stack channels and place in queue for writing to disk on separate thread
        frame_queue.put(np.vstack([data_1_2[:,0], data_1_2[:,1], data_3_4[:,0], data_3_4[:,1]]))

def exit_func():
    global stream_1_2
    global stream_3_4
    global p

    print("Closed streams and pyAudio instance")
    sys.stdout.flush()
    stream_1_2.stop_stream()
    stream_1_2.close()
    stream_3_4.stop_stream()
    stream_3_4.close()
    p.terminate()
    
    print("Program terminated gracefully")
    sys.stdout.flush()
    os._exit(0)

def exit_handler(signal_received, frame):
    print("CTRL-C detected, waiting for current buffer to finish recording...")
    sys.stdout.flush()
    threading.Timer(10, exit_func).start()
    

def save_frames(frame_queue):
    frame_index = 0
    while True:
        if(frame_queue.qsize() > 0):
            data = frame_queue.get()
            print(data.shape)
            sys.stdout.flush()
            np.save('experiments/{}/{}/{}/audio_{}.npy'.format(exp_name, sess_name,'motu', frame_index), data)
            frame_index += 1
        time.sleep(0.01)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, exit_handler)

    saveThread = threading.Thread(target=save_frames, args=(frame_queue,), daemon=True)
    saveThread.start()

    print('Recording from MOTU channels 1_2 and 3_4')
    sys.stdout.flush()
    main()


