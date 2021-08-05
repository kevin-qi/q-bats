load('../BoxExperiments/clean_psu_test_2_14/extracted_data/00000_20210618_CSC13.mat');
lfp = double(downsample(AD_count_int16,10));
lfp_25 = lfp(1:int32(length(lfp)/4));
lfp_50 = lfp(int32(length(lfp)/4):int32(length(lfp)/2));
lfp_75 = lfp(int32(length(lfp)/2):3*int32(length(lfp)/4));
lfp_100 = lfp(3*int32(length(lfp)/4):end);
figure;
subplot(2,2,1);

N = 512
Fs = 3125
[pxx, f] = pwelch(lfp_25, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,2);
[pxx, f] = pwelch(lfp_50, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,3);
[pxx, f] = pwelch(lfp_75, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,4);
[pxx, f] = pwelch(lfp_100, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))


Wo = 60/(Fs/2);  BW = Wo/35;
[num,den] = iirnotch(Wo,BW);
lfp = filtfilt(num,den,lfp);
line_comb = linspace(120,120+60*11,12)
bw_comb = [100, 300, 300, 400, 500, 600,700,800,900,1000,1000,1000]
for i=1:length(line_comb)
    fc = line_comb(i)
    Wo = fc/(Fs/2);  BW = Wo/(bw_comb(i));
    [num,den] = iirnotch(Wo,BW);
    lfp = filtfilt(num,den,lfp);
end
lfp = filtfilt(num,den,lfp);
%plot(lfp(1:10000));
%hold on;
%plot(lfp_nth(1:10000));
lfp_25 = lfp(1:int32(length(lfp)/4));
lfp_50 = lfp(int32(length(lfp)/4):int32(length(lfp)/2));
lfp_75 = lfp(int32(length(lfp)/2):3*int32(length(lfp)/4));
lfp_100 = lfp(3*int32(length(lfp)/4):end);
figure;
subplot(2,2,1);

N = 512
Fs = 3125
[pxx, f] = pwelch(lfp_25, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,2);
[pxx, f] = pwelch(lfp_50, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,3);
[pxx, f] = pwelch(lfp_75, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

subplot(2,2,4);
[pxx, f] = pwelch(lfp_100, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))