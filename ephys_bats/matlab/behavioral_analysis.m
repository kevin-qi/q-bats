function [outputArg1, outputArg2] = behavioral_analysis(processed_annotation)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

data = processed_annotation.frames;

sniff_counts = struct();
for i=1:length(data)
    if(data(i).chunk)
        if(strcmp(data(i).activity,'sniff'))
            disp(data(i).chunk)
            if(isfield(sniff_counts, data(i).chunk))
                sniff_counts.(data(i).chunk) = [sniff_counts.(data(i).chunk) i];
            else
                sniff_counts.(data(i).chunk) = [];
            end
        end
    end
end
disp(sniff_counts);

figure;
i = 1;
for trial_pair={'fm', 'nm', 'nf'}
    subplot(1,3,i);
    t1 = length(sniff_counts.(sprintf('%s_1',trial_pair{1})));
    t2 = length(sniff_counts.(sprintf('%s_2',trial_pair{1})));
    disp(t1);
    X = categorical({'Trial 1', 'Trial 2'});
    X = reordercats(X,{'Trial 1', 'Trial 2'});
    Y = [t1 t2];
    bar(X,Y/10);
    title(trial_pair{1});
    ylabel('sniff time (s)');
    i = i + 1;
    ylim([0 200]);
end

outputArg1 = data;
outputArg2 = sniff_counts;
end

