import numpy as np
import soundcard as sc
import sys


# Record microphone data from MOTU channels 1 % 2
# MOTU_data stores [N x 2] matrix of the two channels (channel 1 is mic, channel 2 is TTL)
args = sys.argv[1:]
exp_name = args[0]
sess_name = args[1]
print("Recording audio for experiment: {}, session: {}".format(exp_name, sess_name))
mic1 = sc.get_microphone('1-2')
# Start recording
audio = np.zeros([1024,2])
with mic1.recorder(samplerate=48000) as mic:

    for _ in range(100):
        MOTU_data = mic.record(numframes=262144)
        audio = np.concatenate((audio,MOTU_data), axis=0)
        np.save('experiments/{}/{}/audio_{}.npy'.format(exp_name, sess_name, _), audio)
print(audio.shape)

# Playback commands:
default_speaker = sc.get_speaker('Main');
default_speaker.play(audio[:,0]/np.max(audio[:,0]),samplerate=48000)
input()