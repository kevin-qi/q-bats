%data = load_logger_command_data('../../box_experiments/210617/ephys/210617_Le_eph_1_14');
%data.csc = downsample(data.csc,10);
csc = mean(data.csc(9:12),2);

N = 8000;
Fs = 3125;
figure;
subplot(2,1,1);
[pxx, f] = pwelch(csc, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

Wo = 60/(Fs/2);  BW = Wo/70;
[num,den] = iirnotch(Wo,BW);
lfp = filtfilt(num,den,csc);
line_comb = linspace(120,120+60*13,14);
%bw_comb = [100, 300, 300, 400, 500, 600,700,800,900,1000,1000,1000]
for i=1:length(line_comb)
    fc = line_comb(i);
    Wo = fc/(Fs/2);  BW = Wo/70;
    [num,den] = iirnotch(Wo,BW);
    lfp = filtfilt(num,den,lfp);
end
lfp = filtfilt(num,den,lfp);

subplot(2,1,2);
[pxx, f] = pwelch(lfp, hanning(N),0,N,Fs);
plot(f, 10*log10(pxx))

t0 = data.timestamps_first_samples_logger_usec{1}(1);
delta_t = 1e6/Fs;
csc_timestamps = linspace(t0, t0+delta_t*(length(csc)-1), length(csc)).';

processed_annotations = parse_via_annotation('../../box_experiments/210617/cameras/annotation_v1_export.csv', 10, 'ACTIVITY_KEY', '1', 'CHUNK_KEY', '2');
chunks = processed_annotations.chunks;
ttl = medfilt1(data.ttl_timestamps_usec);
figure;
tiledlayout(5,5);
axs = [];
for bout = chunks.nm_1
    axs = [axs nexttile];
    disp(bout);
    s = bout.frame_start;
    e = bout.frame_end;
    label = bout.label;
    
    s_time = ttl(s);
    e_time = ttl(e);
    
    [minVal, csc_start] = min(abs(csc_timestamps - s_time));
    [minVal, csc_end] = min(abs(csc_timestamps - e_time));
    disp(sprintf('%d_%d',csc_timestamps(csc_start), csc_timestamps(csc_end)));
    disp(sprintf('%s_%s',s_time, e_time));

    [wt, f] = cwt(csc(csc_start:csc_end), 'amor', Fs, 'FrequencyLimits', [0 300]);
    pcolor(1:numel(csc(csc_start:csc_end)),f,abs(wt));
    shading interp;
    caxis([0 30])
    title(label);
end
linkaxes(axs,'y');


figure;
tiledlayout(5,5);
axs = [];
for bout = chunks.nm_1
    axs = [axs nexttile];
    disp(bout);
    s = bout.frame_start;
    e = bout.frame_end;
    label = bout.label;
    
    s_time = ttl(s);
    e_time = ttl(e);
    
    [minVal, csc_start] = min(abs(csc_timestamps - s_time));
    [minVal, csc_end] = min(abs(csc_timestamps - e_time));
    disp(sprintf('%d_%d',csc_timestamps(csc_start), csc_timestamps(csc_end)));
    disp(sprintf('%s_%s',s_time, e_time));
    
    plot(abs(hilbert(bandpass(csc(csc_start:csc_end), [1,19], Fs))));
    title(label);
end
linkaxes(axs,'xy');

lpf = [1, 21, 41,61,81,101,121,141,161,181, 201];
for i = 1:11
    figure;
    tiledlayout(5,5);
    axs = [];
    for bout = chunks.nm_1
        axs = [axs nexttile];
        disp(bout);
        s = bout.frame_start;
        e = bout.frame_end;
        label = bout.label;

        s_time = ttl(s);
        e_time = ttl(e);

        [minVal, csc_start] = min(abs(csc_timestamps - s_time));
        [minVal, csc_end] = min(abs(csc_timestamps - e_time));
        disp(sprintf('%d_%d',csc_timestamps(csc_start), csc_timestamps(csc_end)));
        disp(sprintf('%s_%s',s_time, e_time));

        plot(abs(hilbert(bandpass(csc(csc_start:csc_end), [lpf(i),lpf(i)+18], Fs))));
        title(label);
    end
    linkaxes(axs,'xy');
sgtitle(sprintf('%s_%s',lpf(i),lpf(i)+18));
end
