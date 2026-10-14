%% Convert an ADS8688 raw .bin frame to the hexadecimal .txt format used by
% frame_fix_convert_plot.m.
%
% The output is a textual byte dump: every original byte is written as two
% hexadecimal characters.  It does not alter headers, sample bytes, or #END.

inputFile = fullfile(pwd, 'input_frame.bin');  % 改成待转换的原始帧路径
[inputDir, inputName, ~] = fileparts(inputFile);
outputFile = fullfile(inputDir, [inputName '.txt']);

fidIn = fopen(inputFile, 'rb');
if fidIn < 0
    error('Cannot open input file: %s', inputFile);
end
cleanupIn = onCleanup(@() fclose(fidIn));

fidOut = fopen(outputFile, 'wt');
if fidOut < 0
    error('Cannot create output file: %s', outputFile);
end
cleanupOut = onCleanup(@() fclose(fidOut));

total = 0;
chunkBytes = 8192;
while true
    bytes = fread(fidIn, chunkBytes, '*uint8');
    if isempty(bytes)
        break;
    end
    fprintf(fidOut, '%02X ', bytes);
    fprintf(fidOut, '\n');
    total = total + numel(bytes);
end

fprintf('Converted %d bytes without modification:\n  %s\n', total, outputFile);
fprintf('Set filePath in frame_fix_convert_plot.m to this TXT file and run it.\n');
