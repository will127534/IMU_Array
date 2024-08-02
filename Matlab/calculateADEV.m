% Close all figures and clear the workspace
close all;
clear all;

load("imu_data_2.mat")

% Define the number of IMUs and data types
numIMUs = 32;
numDataTypes = 9; % Including header and timestamp
fs = 100; % Data rate is 100 Hz
maxSamples = 100 * 300; % maximum number of samples to store

% Structure for Allan Deviation calculation
dataStruct.rate = fs; % Assuming the data rate is 1 Hz for this example

% Allan Deviation calculation and plot
% Define tau values for Allan Deviation calculation
tau = floor(logspace(0, log10(maxSamples), 1000)) *0.01;

figure;

% Calculate the average gyroscope values
avgGyroX = (mean(squeeze(data(1:8, 6, :)), 1) + mean(squeeze(data(9:16, 6, :)), 1) - mean(squeeze(data(17:27, 6, :)), 1) - mean(squeeze(data(25:32, 6, :)), 1)) ./4; 
avgGyroY = (mean(squeeze(data(1:8, 7, :)), 1) - mean(squeeze(data(9:16, 7, :)), 1) - mean(squeeze(data(17:27, 7, :)), 1) + mean(squeeze(data(25:32, 7, :)), 1)) ./4; 
avgGyroZ = mean(squeeze(data(:, 8, :)), 1);


dataStruct.freq = avgGyroX * 3600;
[adevGyroX, ~, ~, tauUsed] = allan(dataStruct, tau, 'Average Gyro_X', 0);

dataStruct.freq = avgGyroY * 3600;
[adevGyroY, ~, ~, ~] = allan(dataStruct, tau, 'Average Gyro_Y', 0);

dataStruct.freq = avgGyroZ * 3600;
[adevGyroZ, ~, ~, ~] = allan(dataStruct, tau, 'Average Gyro_Z', 0);

loglog(tauUsed, sqrt(adevGyroX), 'r', 'DisplayName', 'Avg Gyro_X (dps)','LineWidth',1);
hold on;
loglog(tauUsed, sqrt(adevGyroY), 'b', 'DisplayName', 'Avg Gyro_Y (dps)','LineWidth',1);
loglog(tauUsed, sqrt(adevGyroZ), 'k', 'DisplayName', 'Avg Gyro_Z (dps)','LineWidth',1);

for imuIndex = 1:numIMUs
    % Prepare data structure for root Allan deviation
    dataStruct.rate = fs;
    
    dataStruct.freq = squeeze(data(imuIndex, 6, :)) * 3600; % Gyro_X
    [adevGyroX, ~, ~, ~] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Gyro_X'], 0);
    
    dataStruct.freq = squeeze(data(imuIndex, 7, :)) * 3600; % Gyro_Y
    [adevGyroY, ~, ~, ~] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Gyro_Y'], 0);
    
    dataStruct.freq = squeeze(data(imuIndex, 8, :)) * 3600; % Gyro_Z
    [adevGyroZ, ~, ~, tauUsed] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Gyro_Z'], 0);
    
    % Plot Allan Deviation
    loglog(tauUsed, sqrt(adevGyroX), 'DisplayName', 'Gyro_X (dps)','Color',"#D95319");
    loglog(tauUsed, sqrt(adevGyroY), 'DisplayName', 'Gyro_Y (dps)','Color',"#4DBEEE");
    loglog(tauUsed, sqrt(adevGyroZ)', 'DisplayName', 'Gyro_Z (dps)','Color',"#A6A6A6");
    
    xlabel('Averaging Time (s)');
    ylabel('Root Allan Deviation (dph)');
    title(['32x IMU Gryo Root Allan Deviation (degrees per hour)']);
    %legend('show');
end

xlim([tauUsed(1),tauUsed(end)])
print(gcf, 'Gyroscope_Allan_Deviation.jpg', '-djpeg', '-r150');


figure;
% Calculate the average accelerometer values
avgAccX = (mean(squeeze(data(1:8, 3, :)), 1) + mean(squeeze(data(9:16, 3, :)), 1) - mean(squeeze(data(17:24, 3, :)), 1) - mean(squeeze(data(25:32, 3, :)), 1)) ./4; 
avgAccY = (mean(squeeze(data(1:8, 4, :)), 1) - mean(squeeze(data(9:16, 4, :)), 1) - mean(squeeze(data(17:24, 4, :)), 1) + mean(squeeze(data(25:32, 4, :)), 1)) ./4; 
avgAccZ = mean(squeeze(data(:, 5, :)), 1);




dataStruct.freq = avgAccX;
[adevAccX, ~, ~, tauUsed] = allan(dataStruct, tau, 'Average Acc_X ', 0);

dataStruct.freq = avgAccY;
[adevAccY, ~, ~, ~] = allan(dataStruct, tau, 'Average Acc_Y', 0);

dataStruct.freq = avgAccZ;
[adevAccZ, ~, ~, ~] = allan(dataStruct, tau, 'Average Acc_Z', 0);

loglog(tauUsed, (adevAccX), 'r', 'DisplayName', 'Avg Acc_X  (dps)','LineWidth',1);
hold on;
loglog(tauUsed, (adevAccY), 'b', 'DisplayName', 'Avg Acc_Y  (dps)','LineWidth',1);
loglog(tauUsed, (adevAccZ), 'k', 'DisplayName', 'Avg Acc_Z  (dps)','LineWidth',1);

for imuIndex = 1:numIMUs
    % Prepare data structure for root Allan deviation
    dataStruct.rate = fs;
    
    dataStruct.freq = squeeze(data(imuIndex, 3, :)); % Example for Acc_X
    [adevAccX, ~, ~, ~] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Acc_X'], 0);
    
    dataStruct.freq = squeeze(data(imuIndex, 4, :)); % Acc_Y
    [adevAccY, ~, ~, ~] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Acc_Y'], 0);
    
    dataStruct.freq = squeeze(data(imuIndex, 5, :)); % Acc_Z
    [adevAccZ, ~, ~, tauUsed] = allan(dataStruct, tau, ['IMU ' num2str(imuIndex) ' Acc_Z'], 0);
    
    % Plot Allan Deviation
    loglog(tauUsed, (adevAccX), 'DisplayName', 'Acc_X (mg)','Color',"#D95319");
    loglog(tauUsed, (adevAccY), 'DisplayName', 'Acc_Y (mg)','Color',"#4DBEEE");
    loglog(tauUsed, (adevAccZ)', 'DisplayName', 'Acc_Z (mg)','Color',"#A6A6A6");
    
    xlabel('Averaging Time (s)');
    ylabel('Allan Deviation  (mg)');
    title(['32x IMU Acc Allan Deviation (mg)']);
    %legend('show');
end
xlim([tauUsed(1),tauUsed(end)])
print(gcf, 'Accelerometer_Allan_Deviation.jpg', '-djpeg', '-r150');
