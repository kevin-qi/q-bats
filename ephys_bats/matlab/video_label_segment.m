function [output] = video_label_segment(processed_annotation, input_dir, fps)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

% inter-frame-time (s)
T = 1/fps;

activity = processed_annotation.activity_names;
chunk = processed_annotation.chunk_names;

%% Create output folders
for i=1:length(chunk)
    for j=1:length(activity)
        if(~isfolder(fullfile(input_dir,'output',chunk{i},activity{j})))
            mkdir(fullfile(input_dir,'output',chunk{i},activity{j}));
        end
    end
end

%% Get chunks
fn = fieldnames(processed_annotation.chunks);
for i=1:numel(fn)
    chunk = processed_annotation.chunks.(fn{i});
    for j=1:length(chunk)
        label = chunk{j}.label;
        s = chunk{j}.frame_start;
        e = chunk{j}.frame_end;
        
        folder_name = fullfile(input_dir, 'output',fn{i},label);
        video_name = fullfile(folder_name, strcat(num2str(frame_start),'_',num2str(frame_end),'.avi'));
        disp(video_name);
        v = VideoWriter(video_name);
        v.FrameRate = 10;
        open(v);
        for frame=frame_start:frame_end
            %disp(frame);

            src = fullfile(input_dir,'bottom',strcat(num2str(frame),'.png'));
            des = fullfile(folder_name, strcat(num2str(frame),'.png'));
            writeVideo(v, imread(src));
            copyfile(src,des);
        end
        close(v);
    end
end

output = chunk_indices;    
for i=1:height(processed_annotation)
    temp_coord = jsondecode(processed_annotation.temporal_coordinates{i});
    if(length(temp_coord)==2)
        %disp(temp_coord);
        s = round(temp_coord(1)/T);
        e = round(temp_coord(2)/T);
        for k=1:length(chunk)
            chunk_name = chunk{k};
            indices = chunk_indices.(chunk_name);
            for j=1:size(indices,2)
                if(s>indices(1,j) & s<indices(2,j))
                    disp(chunk_name);
                    frame_start = s;
                    frame_end = min(e, indices(2,j));
                elseif(e>indices(1,j) & e<indices(2,j))
                    disp(chunk_name);
                    frame_start = max(s, indices(1,j));
                    frame_end = e;
                else
                    frame_start = 0;
                    frame_end = 0;
                end
                disp(frame_start);
                disp(frame_end);
                
                if(frame_start ~= frame_end)
                    label = processed_annotation.label{i};
                    folder_name = fullfile(input_dir, 'output',chunk_name,label);
                    folder_name = fullfile(folder_name, strcat(num2str(frame_start),'_',num2str(frame_end)));
                    if(~isfolder(folder_name))
                        mkdir(folder_name)
                    end
                    video_name = fullfile(folder_name, strcat(num2str(frame_start),'_',num2str(frame_end),'.avi'));
                    disp(video_name);
                    v = VideoWriter(video_name);
                    v.FrameRate = 10;
                    open(v);
                    for frame=frame_start:frame_end
                        %disp(frame);
                       
                        src = fullfile(input_dir,'bottom',strcat(num2str(frame),'.png'));
                        des = fullfile(folder_name, strcat(num2str(frame),'.png'));
                        writeVideo(v, imread(src));
                        copyfile(src,des);
                    end
                    close(v);
                end
                
            end
        end
        
    end
end

%outputArg1 = inputArg1;
%outputArg2 = inputArg2;
end

