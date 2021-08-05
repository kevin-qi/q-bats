if true
load('../BoxExperiments/210622/ephys/210622_Le_eph_1_14/extracted_data/00000_20210622_CSC15.mat');
lfp = double(downsample(AD_count_int16,10));
end

N = 10000
Fs = 3125
figure;
subplot(1,1,1)
[pxx, f] = pwelch(lfp, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))
xline(202);
xline(212);
xline(222);
xline(232);
xline(242);
xline(252);
xline(263);

%Wo = 60/(Fs/2);  BW = Wo/50;
%[num,den] = iirnotch(Wo,BW);
%lfp = filtfilt(num,den,lfp);
%line_comb = linspace(120,120+60*11,12)
%bw_comb = [120, 180, 300, 300, 400, 600,700,800,900,1000,1000,1000]
%for i=1:length(line_comb)
%    fc = line_comb(i)
%    Wo = fc/(Fs/2);  BW = Wo/(bw_comb(i));
%    [num,den] = iirnotch(Wo,BW);
%    lfp = filtfilt(num,den,lfp);
%end
%subplot(1,2,2)
%[pxx, f] = pwelch(lfp, hanning(N),0,N,Fs);
%plot(f, 10*log10(pxx))