% Close all figures and clear the workspace
close all;
clear all;

% Define the serial port and baud rate
device = serialport("COM8", 9600);

% Define the number of IMUs and data types
numIMUs = 32;
numDataTypes = 9; % Including header and timestamp

% Initialize a 3D array to store data
maxSamples = 100 * 1200; % maximum number of samples to store
data = zeros(numIMUs, numDataTypes, maxSamples);

pause(10)
display('Start Measuring')
% Initialize a counter for the number of samples
sampleCount = 0;
tic;
% Read and process data in a loop
while sampleCount < maxSamples
    line = readline(device);
    
    if mod(sampleCount,1000) == 0
        display(sampleCount)
    end
    
    % Split the line into individual data points
    dataPoints = strsplit(line, ',');
    
    if length(dataPoints) == numIMUs * numDataTypes + 1
        sampleCount = sampleCount + 1;
        
        % Extract and store the data for each IMU
        for i = 1:numIMUs
            imuData = dataPoints((i-1)*numDataTypes + 1:i*numDataTypes);
            imuDataNum = str2double(imuData);
            
            % Apply conversions
            imuDataNum(3:5) = imuDataNum(3:5) * 0.061; % Accelerometer
            imuDataNum(6:8) = imuDataNum(6:8) * (31.25 / 65535.0); % Gyroscope
            imuDataNum(9) = imuDataNum(9) / 2.07 + 25; % Temperature
            
            data(i, :, sampleCount) = imuDataNum;
        end
    end
end
elapsedTime = toc;
% Close the serial port
clear device;

% Save the data to a .mat file
save('imu_data_2.mat', 'data', 'maxSamples');
% Prepare data for CSV
dataForCSV = reshape(permute(data, [3, 1, 2]), maxSamples, numIMUs * numDataTypes);

% Create headers for the CSV file
headers = cell(1, numIMUs * numDataTypes);
dataTypes = {'Header', 'Timestamp', 'Acc_X (mg)', 'Acc_Y (mg)', 'Acc_Z (mg)', 'Gyro_X (dps)', 'Gyro_Y (dps)', 'Gyro_Z (dps)', 'Temp (C)'};

for j = 1:numDataTypes
    for i = 1:numIMUs 
        headers{(j-1)*numIMUs + i} = ['IMU' num2str(i) '_' dataTypes{j}];
    end
end

% Write the data to a CSV file
csvFile = 'imu_data_2.csv';
fid = fopen(csvFile, 'w');
fprintf(fid, '%s,', headers{1:end-1});
fprintf(fid, '%s\n', headers{end});
fclose(fid);

% Use dlmwrite to append the data
dlmwrite(csvFile, dataForCSV, '-append');

% Display the stored data
fprintf('Time taken to read all data: %.2f seconds\n', elapsedTime);
