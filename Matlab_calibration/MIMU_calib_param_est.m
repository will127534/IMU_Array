function [theta, y_cal_mean] = MIMU_calib_param_est(meas, M, scale_acc, scale_gyro, L, internalMisalignment)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%>
%>  Function that runs the IMU calibration with raw data from multiple positions.
%>  The function does the following:
%>  - Loads the averaged data from multiple positions.
%>  - Estimates the initial roll and pitch angles.
%>  - Processes the calibration algorithm.
%>
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Settings and memory allocation

global y_meas;
global cholQinv;

% Initialize variables
y_meas = zeros(M, 3, L);  % Accelerometer measurements for M IMUs, 3 axes, L positions
cholQinv = zeros(M, 3, 3, L);  % Noise covariance matrices
theta = zeros(12, M);
y_cal_mean = zeros(M, 3, L);
gyro_mean = zeros(M, 3);

%% Load data and preprocess the data

% (Data loading and preprocessing code remains the same as in your script)

% Example: (Adjust according to your data loading code)
% Loop through each position/orientation
for l = 1:L
    % Assume meas{l} contains the averaged data for all IMUs at position l
    inertial_data = meas{l};
    
    for m = 1:M
        % Extract accelerometer and gyroscope data for the m-th IMU
        idx_start = 6*(m-1)+1;
        acc_data = scale_acc * inertial_data(idx_start:idx_start+2);
        y_meas(m, :, l) = acc_data';  % Store accelerometer data (1 x 3)
        
        gyro_data = scale_gyro * inertial_data(idx_start+3:idx_start+5);
        gyro_mean(m, :) = gyro_mean(m, :) + gyro_data'/L;  % Accumulate for averaging
        
        % Since we have only mean data, we can't estimate noise covariance
        % So we set cholQinv to identity matrices
        cholQinv(m, :, :, l) = eye(3);
    end
end

%% Calculate the initial estimates for the roll and pitch

tmp = zeros(2, L);
for l = 1:L
    % Calculate the mean over all IMUs at this position
    f = mean(y_meas(:, :, l), 1)';  % f is a 3x1 vector
    % Roll
    tmp(1, l) = atan2(f(2), f(3));
    % Pitch
    tmp(2, l) = atan2(-f(1), sqrt(f(2)^2 + f(3)^2));
end

%% Run estimation of the accelerometer scale factors, biases, and IMU misalignment

if internalMisalignment
    % Define the initial state estimate
    
    % Orientation parameters (roll and pitch estimates)
    theta0_orientation = reshape(tmp, 2*L, 1);
    
    % Calibration parameters for IMUs 1 to M-1 (with internal misalignment)
    cal_params_IMUs = repmat([1;1;1; 0;0;0; 0;0;0; 0;0;0], M-1, 1);
    
    % Calibration parameters for the last IMU (without internal misalignment)
    cal_params_last_IMU = [1;1;1; 0;0;0; 0;0;0]; % 9 elements (no internal misalignment)
    
    % Combine into theta0
    theta0 = [theta0_orientation; cal_params_IMUs; cal_params_last_IMU];
    
    % Run the optimization
    tmp_theta = lsqnonlin(@costFunctionArrayInternalMisalignment, theta0);
    
    % Extract calibration parameters from tmp_theta
    num_orientation_params = 2*L;
    calibration_params = tmp_theta(num_orientation_params+1:end);
    
    % Reconstruct theta (12 x M) matrix
    theta = zeros(12, M);
    
    % For IMUs 1 to M-1
    for m = 1:M-1
        idx_start = (m-1)*12 + 1;
        idx_end = m*12;
        theta(:, m) = calibration_params(idx_start:idx_end);
    end
    
    % For the last IMU (IMU M)
    theta_m_last = calibration_params((M-1)*12 +1:end); % Should be 9 elements
    if length(theta_m_last) ~= 9
        error('Incorrect number of calibration parameters for the last IMU.');
    end
    % Pad with zeros for internal misalignment parameters
    theta(1:9, M) = theta_m_last;
    theta(10:12, M) = zeros(3,1);
else
    % Define the initial state estimate
    theta0 = [reshape(tmp, 2*L, 1); repmat([1;1;1; 0;0;0; 0;0;0], M, 1)];
    % Remove alignment parameters of last IMU
    theta0 = theta0(1:end-3);
    % Run the optimization
    tmp_theta = lsqnonlin(@costFunctionArray, theta0);
    % Reshape parameters into original form
    tmp_theta = reshape([tmp_theta(2*L+1:end); zeros(3,1)], 9, M);
    % Insert zeros for internal misalignment
    theta(:, :) = [tmp_theta(1:6,:); zeros(3,M); tmp_theta(7:9,:)];
end

%% Calibrate the mean value of the data at each orientation

for m = 1:M
    theta_tmp = theta(:, m);
    
    % Set the calibration parameters
    K = diag(theta_tmp(1:3));  % Scale factors
    S = eye(3);  % Misalignment matrix
    S(1,2) = -theta_tmp(7);
    S(1,3) = theta_tmp(8);
    S(2,3) = -theta_tmp(9);
    b = theta_tmp(4:6);  % Biases
    E = eye(3);  % Internal misalignment matrix (if applicable)
    if internalMisalignment && m < M
        % Internal misalignment parameters are only for IMUs 1 to M-1
        E(1,2) = theta_tmp(12);
        E(1,3) = -theta_tmp(11);
        E(2,1) = -theta_tmp(12);
        E(2,3) = theta_tmp(10);
        E(3,1) = theta_tmp(11);
        E(3,2) = -theta_tmp(10);
    else
        % No internal misalignment for the last IMU
        E = eye(3);
    end
    
    % Calibrate the accelerometer measurements at each orientation
    for l = 1:L
        tmp_data = y_meas(m, :, l)' - b;
        y_cal_mean(m, :, l) = (K * S * E) \ tmp_data;
    end
end

% Append gyro mean (biases) values to the calibration parameters
theta = [theta' gyro_mean];

end
