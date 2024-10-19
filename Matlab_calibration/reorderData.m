function dataOutput = reorderData(data)
% Function to rotate different groups of the 32 IMUs in the data array
%
% Input:
%   data - a [192 x measurement_count] array
%          Each IMU contributes 6 rows: 3 for accelerometer, 3 for gyroscope
%          (32 IMUs * 6 measurements per IMU = 192 rows)
%
% Output:
%   dataOutput - the rotated data array of the same size as input

% Initialize the output data array
dataOutput = data;

% Number of IMUs
M = 32;

% Process each group
% Group 1: IMUs 1 to 8
for m = 1:8
    idx = (m - 1) * 6;
    % Accelerometer data
    dataOutput(:,idx + 1) = data(:,idx + 1); % Acc_X
    dataOutput(:,idx + 2) = data(:,idx + 2); % Acc_Y
    dataOutput(:,idx + 3) = data(:,idx + 3); % Acc_Z
    % Gyroscope data
    dataOutput(:,idx + 4) = data(:,idx + 4); % Gyro_X
    dataOutput(:,idx + 5) = data(:,idx + 5); % Gyro_Y
    dataOutput(:,idx + 6) = data(:,idx + 6); % Gyro_Z
end

% Group 2: IMUs 9 to 16
for m = 9:16
    idx = (m - 1) * 6;
    % Accelerometer data
    dataOutput(:,idx + 1) = -data(:,idx + 2); % Acc_X = -Acc_Y
    dataOutput(:,idx + 2) = data(:,idx + 1);  % Acc_Y = Acc_X
    dataOutput(:,idx + 3) = data(:,idx + 3);  % Acc_Z remains the same
    % Gyroscope data
    dataOutput(:,idx + 4) = -data(:,idx + 5); % Gyro_X = -Gyro_Y
    dataOutput(:,idx + 5) = data(:,idx + 4);  % Gyro_Y = Gyro_X
    dataOutput(:,idx + 6) = data(:,idx + 6);  % Gyro_Z remains the same
end

% Group 3: IMUs 17 to 24
for m = 17:24
    idx = (m - 1) * 6;
    % Accelerometer data
    dataOutput(:,idx + 1) = -data(:,idx + 1); % Acc_X = -Acc_X
    dataOutput(:,idx + 2) = -data(:,idx + 2); % Acc_Y = -Acc_Y
    dataOutput(:,idx + 3) = data(:,idx + 3);  % Acc_Z remains the same
    % Gyroscope data
    dataOutput(:,idx + 4) = -data(:,idx + 4); % Gyro_X = -Gyro_X
    dataOutput(:,idx + 5) = -data(:,idx + 5); % Gyro_Y = -Gyro_Y
    dataOutput(:,idx + 6) = data(:,idx + 6);  % Gyro_Z remains the same
end

% Group 4: IMUs 25 to 32
for m = 25:32
    idx = (m - 1) * 6;
    % Accelerometer data
    dataOutput(:,idx + 1) = data(:,idx + 2);  % Acc_X = Acc_Y
    dataOutput(:,idx + 2) = -data(:,idx + 1); % Acc_Y = -Acc_X
    dataOutput(:,idx + 3) = data(:,idx + 3);  % Acc_Z remains the same
    % Gyroscope data
    dataOutput(:,idx + 4) = data(:,idx + 5);  % Gyro_X = Gyro_Y
    dataOutput(:,idx + 5) = -data(:,idx + 4); % Gyro_Y = -Gyro_X
    dataOutput(:,idx + 6) = data(:,idx + 6);  % Gyro_Z remains the same
end

end
