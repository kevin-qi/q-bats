function [output] = parse_via_annotation(filepath, fps, varargin)
%parse_via_annotation Parse annotation file from VIA into behavioral trials
%   Parameters
%   -----------
%   filepath | string
%       path to exported VIA annotation file
%   fps | float
%       video framerate
%   ACTIVITY_KEY | Optional, KEY-VALUE, 'default': '1'
%       Key/ID of "Activity" attribute as set in VIA. This should be the
%       attribute that labels the behavior/actions.
%   CHUNK_KEY | Optional, KEY-VALUE, 'default': '2'
%       Key/ID of "Chunk" attribute as set in VIA. This would be the
%       attribute that labels the chunks/segments of a behavioral
%       experiment between which behavior is compared. If null, just label
%       the entire video as one big chunk.
%
%   Output
%   -------
%   out = struct()
%       out.T
%           1/fps
%       out.frames
%           Chunk & Activity of each frame

%% Parse inputs
defaultActivityKey = '1';
defaultChunkKey = '2';

p = inputParser;
addRequired(p, 'filepath');
addRequired(p, 'fps', @isnumeric);
addParameter(p, 'ACTIVITY_KEY', defaultActivityKey);
addParameter(p, 'CHUNK_KEY', defaultChunkKey);
parse(p, filepath, fps, varargin{:});
activity_key = p.Results.ACTIVITY_KEY;
chunk_key = p.Results.CHUNK_KEY;

%% Parse VIA annotation metadata
fid = fopen(filepath);
tline = fgetl(fid);
for i=1:10
    if(i==7) % SHAPE_ID
        %disp(tline)
        match = regexp(tline, '{.*}', 'match');
        shape_id = jsondecode(match{1});
    elseif(i==8) % FLAG_ID
        match = regexp(tline, '{.*}', 'match');
        flag_id = jsondecode(match{1});
    elseif(i==9) % Annotations
        match = regexp(tline, '{.*}', 'match');
        temp = jsondecode(match{1});
        output.temp = temp;
        attribute{1} = struct2cell(temp.(['x' activity_key]).options);
        attribute{2} = struct2cell(temp.(['x' chunk_key]).options);
    elseif(i==10) % CSV_HEADER
        match = regexp(tline, '(\w*)', 'match');
        csv_header = match(2:end);
    end
        
    tline = fgetl(fid);
end
fclose(fid);

%% Parse VIA annotations
T = 1/fps;
annotations = readtable(filepath,'NumHeaderLines',10);
annotations.Properties.VariableNames = csv_header;
%annotations.activity = metadata{1};
%disp(annotations);

get_attribute_1 = @(x) regexp(x{1},'(\d)','match');
get_attribute_2 = @(x) attribute{str2num(x{1}{1})}{str2num(x{1}{2})+1};
t = arrayfun(get_attribute_1, annotations.metadata, 'UniformOutput', false);
annotations.label = arrayfun(get_attribute_2, t, 'UniformOutput', false);

%activity = struct2cell(attribute.activity.options);
%disp(activity);

activity = attribute{1};
chunk = attribute{2};

chunk_indices = struct();
chunk_key = struct();
for i=1:length(chunk)
    chunk_indices.(chunk{i}) = [];
    chunk_key.(chunk{i}) = i;
end

for i=1:height(annotations)
    label = annotations.label{i};
    if(regexp(cell2mat(chunk.'), label))
        disp(label);
        disp(annotations.temporal_coordinates{i});
        indices = jsondecode(annotations.temporal_coordinates{i});
        s = uint32(indices(1)/T);
        e = round(indices(2)/T);
        chunk_indices.(label) = [chunk_indices.(label) [s;e]];
    end
end

%% Get first and last annotated frames
res = arrayfun(@(x) regexp(x{1},'([.\d]*)','match'), annotations.temporal_coordinates, 'UniformOutput', false);
frame_indices = [];
disp(length(res));
for i=1:length(res)
    if(size(res{i},2) == 2)
        frame_indices = [frame_indices str2double(res{i}{1})];
        frame_indices = [frame_indices str2double(res{i}{2})];
    end
    disp(i);
end
first_annotated_frame = round(min(frame_indices)/T);
last_annotated_frame = round(max(frame_indices)/T);

for frame=first_annotated_frame:last_annotated_frame
    data(frame).chunk = 'null';
    data(frame).activity = 'null';
end
chunks = struct();
for i=1:height(annotations)
    temp_coord = jsondecode(annotations.temporal_coordinates{i});
    if(length(temp_coord)==2) % Temporal segments should have start and end
        %disp(temp_coord);
        s = round(temp_coord(1)/T); % start frame
        e = round(temp_coord(2)/T); % end frame
        
        % The following for-block searches for the chunk(s) that this temporal
        % segment belongs to. Temporal segments that span 2 chunks will be
        % counted as 2 segments in their corresponding chunk.
        for k=1:length(chunk) % For each chunk_name in experiment
            chunk_name = chunk{k};
            indices = chunk_indices.(chunk_name); % Start/end frames of the chunk
            % chunk_indices.(chunk_name) has shape (2,N), where N is the number of
            % chunks of chunk_name and rows are start/end frames.
            for j=1:size(indices,2) % For each chunk
                if(s>indices(1,j) & s<indices(2,j)) % Temporal segment starts in this chunk
                    disp(chunk_name);
                    frame_start = s;
                    frame_end = min(e, indices(2,j)); % Cutoff temporal segment if it extends beyond the chunk
                elseif(e>indices(1,j) & e<indices(2,j)) % If temporal segment ends in this chunk
                    disp(chunk_name);
                    frame_start = max(s, indices(1,j)); %Cut off temporal segment if it extends beyond the chunk
                    frame_end = e;
                elseif(s<indices(1,j) & e>indices(2,j)) %If temporal segment spans the entire chunk
                    disp(chunk_name);
                    frame_start = indices(1,j); 
                    frame_end = indices(2,j);
                else % If temporal segment not in this chunk, null.
                    frame_start = 0;
                    frame_end = 0;
                end
                if(frame_start ~= frame_end) % If temporal segment belongs to this chunk
                    disp(frame_start);
                    disp(frame_end);
                    label = annotations.label{i};
                    act.label = label;
                    act.frame_start = frame_start;
                    act.frame_end = frame_end;
                    ch_name = sprintf('%s_%d', chunk_name, j);
                    if(~ismember(ch_name, fieldnames(chunks)))
                        chunks.(ch_name) = [act];
                    else
                        chunks.(ch_name) = [chunks.(ch_name) act]; 
                    end
                    for frame=frame_start:frame_end
                        data(frame).chunk = sprintf('%s_%d', chunk_name, j);
                        data(frame).activity = label;
                    end
                end
            end
        end
    end
end


output.annotations = annotations;
output.chunk_indices = chunk_indices;
output.chunk_key = chunk_key;
output.chunk_names = chunk;
output.activity_names = activity;
output.T = 1/fps;
output.fps = fps;
output.frames = data;
output.chunks = chunks;


[dir, name, ext] = fileparts(filepath);
if(~isfolder(fullfile(dir,'output')))
    mkdir(fullfile(dir,'output'));
end
save(fullfile(dir,'output','processed_annotation.mat'), 'output');
%outputArg2 = inputArg2;
end

