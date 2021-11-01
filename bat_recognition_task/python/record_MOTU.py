import numpy as np
import soundcard as sc
import sys
import pathlib
from queue import Queue
import threading
import time

def save_audio_data(saveQueue):
    while True:
        if(saveQueue.qsize() > 0):
            data = saveQueue.get()
            print(data.shape)
            sys.stdout.flush()
            np.save('experiments/{}/{}/{}/audio_{}.npy'.format(exp_name, sess_name,'motu', _), data)
        time.sleep(0.01)

# Record microphone data from MOTU channels 1 % 2
# MOTU_data stores [N x 2] matrix of the two channels (channel 1 is mic, channel 2 is TTL)
args = sys.argv[1:]
exp_name = args[0]
sess_name = args[1]
print("Recording audio for experiment: {}, session: {}".format(exp_name, sess_name))
sys.stdout.flush()

saveQueue = Queue()

saveThread = threading.Thread(target=save_audio_data, args=(saveQueue,), daemon=True)
saveThread.start()

mic1 = sc.get_microphone('1-2')
mic2 = sc.get_microphone('3-4')
# Start recording
pathlib.Path('experiments/{}/{}/{}'.format(exp_name, sess_name,'motu')).mkdir(parents=True,exist_ok=True)
audio = np.zeros([1024,2])
with mic1.recorder(samplerate=48000) as mic1, mic2.recorder(samplerate=48000) as mic2:

    for _ in range(99999999999999999):
        MOTU_data_1 = mic1.record(numframes=480000)
        MOTU_data_2 = mic2.record(numframes=480000)
        #audio = np.concatenate((audio,MOTU_data), axis=0)
        #print(MOTU_data.shape)
        MOTU_data = np.hstack([MOTU_data_1, MOTU_data_2])
        saveQueue.put(MOTU_data)
        

# Playback commands:
#default_speaker = sc.get_speaker('Main');
#default_speaker.play(audio[:,0]/np.max(audio[:,0]),samplerate=48000)
#input()

