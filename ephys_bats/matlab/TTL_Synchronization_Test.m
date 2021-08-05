%% TTL Synchronization Test for Ephys
% Using "Interferometer" setup where TTL signal is sent to logger and
% recorded by logger.
% For use with Deuteron Technology Loggers & LoggerCommand3

clear all; close all;

%% Load in TTL event and recorded TTL data
if true
data_dir = '../sync_test_19';
data = load_logger_command_data(data_dir);
end

t0 = data.timestamps_first_samples_logger_usec{1}(1);

ref_timings = data.ttl_timestamps_usec;

rec = mean(data.csc,2);
rec = rec(1:end-10000,:); % Remove end samples (artifact due to shutting off system)

figure
plot(rec);

hold on
for i=1:length(ref_timings);
    xline((ref_timings(i)-t0)/32);
end
shg;

% Get rising edges of recorded signal
dy = diff(rec);                % Take Derivative
[dypks,ix] = findpeaks(dy, 'MinPeakHeight', 5, 'MinPeakProminence', 2, 'MinPeakDistance', 31250*2);
ix = ix*32 + t0;
figure;
hold on
for i=1:length(ix);
    xline(ix(i));
end
shg;
rec_timings = ix;
%% Plot Recorded and Reference TTL


%% TTL counts
ref_ttl_count = length(rec_timings);
rec_ttl_count = length(ref_timings);

disp(ref_ttl_count);
disp(rec_ttl_count);
assert(abs(ref_ttl_count - rec_ttl_count)<=1, 'Recorded and Reference TTL counts do not match!');

% If counts do not match, slice them so that they match but throw a
% warning
if(abs(ref_ttl_count - rec_ttl_count) ~= 0)
    disp("WARNING: ref_ttl_count does not match rec_ttl_count");
    ref_timings = ref_timings(1:min(rec_ttl_count, ref_ttl_count));
    rec_timings = rec_timings(1:min(rec_ttl_count, ref_ttl_count));
end

%% TTL timings
figure;
tiledlayout(2,2);
nexttile;
histogram(diff(ref_timings)-3*10^6, 50);
title('TTL timing difference error (usec)');

nexttile;
scatter(ref_timings(1:end-1,1), cumsum(diff(ref_timings)-3*10^6))
title('Cumulative TTL timing difference error (usec)');

nexttile;
scatter(ref_timings, rec_timings);
title('Recorded TTL vs Reference TTL timings (usec)');

figure;
tiledlayout(2,2);

nexttile;
scatter(ref_timings, (rec_timings - ref_timings)/1000);
ylim([-100,100]);
title('Recorded - Reference TTL Difference (ms)');

nexttile;
histogram((rec_timings - ref_timings)/1000, 50);
title('Recorded - Reference TTL Diff Hist (ms)');

% Look at error relative to 0 or 50 (depending on if peak finding looked at
% the leading or falling edge due to imperfections in recording square
% waves
nexttile;
d = (rec_timings - ref_timings)/1000;
d2 = [];
for i=1:length(d)
    if d(i)>40
        d2(length(d2)+1) = d(i)-50;
    else
        d2(length(d2)+1) = d(i);
    end
end
scatter(ref_timings, d2);
ylim([-100,100]);
title('Recorded - Reference TTL Difference Modulo 50 (ms)');

nexttile;
histogram(d2, 50);
title('Recorded - Reference TTL Diff Hist Modulo 50 (ms)');


