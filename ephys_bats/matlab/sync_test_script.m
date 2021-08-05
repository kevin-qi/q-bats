%Brief script for loading data from Mouse_Logger16
%Load event data
TTL_idx = find(contains(event_types_and_details,'Digital in'));
ref = ones(size(TTL_idx));
TTL_times = event_timestamps_usec(TTL_idx)/1e6;
%Load logger data
signal = AD_count_int16;
%%
edges = [-70000 2000:1:7000 70000];
histogram(diff(TTL_times),edges);
Fs = Estimated_channelFS_Transceiver(1);
time = Timestamps_of_first_samples_usec(1)/1e6+[0:1/Fs:(length(AD_count_int16)-1)/Fs];
ax(1) = subplot(211);   stem(TTL_times,ones(size(TTL_times)));
ax(2) = subplot(212);   plot(time,signal);
linkaxes(ax,'x');
stem(TTL_times,ref);    hold on;
plot(time,normalize(signal,'range'));
%%
time2 = interp1(Indices_of_first_and_last_samples(:,1)',Timestamps_of_first_samples_usec,1:length(AD_count_int16),'linear','extrap')/1e6;
figure()
for i = 1:30:length(TTL_times)
    [~,idx] = min(abs(time2-TTL_times(i)));
    y = signal(:,idx-100:idx+200);
    t = [0:1/Fs:(length(y)-1)/Fs]*1e3;
    plot(t,y);  hold on;
    if i==1 line([100/Fs,100/Fs]*1e3,[-150 150]), end
    drawnow();
end
hold off;