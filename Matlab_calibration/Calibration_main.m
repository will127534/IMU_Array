close all
clear all

global g_local;
g_local = gravity(37.29,100);

%The CSV files are the measurement collected with 20 different orientations
%each with 90 seconds of data

% Define the number of positions/orientations (number of CSV files)
L = 20; 
% Number of IMUs
M = 32; 

% Scaling factors (set to 1 if data is already scaled)
scale_acc = 4 * g_local /65535; %+- 2g
scale_gyro =  (31.25 / 65535.0);%+-15.625 dps

% Initialize cell array to hold measurements for each position
meas = cell(L, 1);

% Loop over each position/orientation
for l = 1:L
    % Construct the filename
    filename = sprintf('./CalibrationDataset_1/%d.csv', l);
    
    % Read data from the CSV file using your readCSV function
    data = readCSV(filename);
    
    % Fix the rotation of each IMU bank
    data = reorderData(data);
    
    % Initialize a vector to hold averaged data for all IMUs at this position
    % Each IMU contributes 6 values (3 for accelerometer, 3 for gyroscope)
    meas_l = zeros(6*M, 1);
    
    % Loop over each IMU
    for m = 1:M
        % Calculate starting index for this IMU in the data array
        idx_start = (m-1)*6;
        
        % Indices for accelerometer data
        acc_cols = idx_start + [1, 2, 3]; % acc_x, acc_y, acc_z
        
        % Indices for gyroscope data
        gyro_cols = idx_start + [4, 5, 6]; % gyro_x, gyro_y, gyro_z
        
        % Extract accelerometer and gyroscope data for the m-th IMU
        acc_data = data(:, acc_cols);
        gyro_data = data(:, gyro_cols);
        
        % Compute the mean over time for this IMU at this position
        acc_mean = mean(acc_data, 1); % 1x3 vector
        gyro_mean = mean(gyro_data, 1); % 1x3 vector
        
        % Store the mean values in the meas_l vector
        idx_meas_start = 6*(m-1)+1;
        meas_l(idx_meas_start:idx_meas_start+2) = acc_mean';  % Accelerometer data
        meas_l(idx_meas_start+3:idx_meas_start+5) = gyro_mean';  % Gyroscope data
    end
    
    % Store the measurements for this position
    meas{l} = meas_l;
end

% Internal misalignment flag (set as needed)
internalMisalignment = true;

% Call the calibration function with the processed data
[theta, y_cal_mean] = MIMU_calib_param_est(meas, M, scale_acc, scale_gyro, L, internalMisalignment);

% Display the calibration parameters
disp('Calibration Parameters (theta):');
disp(theta);

% Display the calibrated accelerometer mean values
disp('Calibrated Accelerometer Mean Values (y_cal_mean):');
disp(y_cal_mean);

generateCalibrationHeader(theta, "imu_cal.h")

save('theta.mat','theta')