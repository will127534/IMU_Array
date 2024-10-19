function generateCalibrationHeader(theta, filename)
% generateCalibrationHeader generates a C header file with calibration matrices and biases.
%
% Inputs:
%   - theta: Calibration parameters for all IMUs, a [32 x 12] matrix.
%   - filename: Name of the header file to be generated (e.g., 'calibration.h').

% Number of IMUs
M = size(theta, 1);

% Initialize arrays to hold calibration matrices and biases
C_acc_all = zeros(M, 9); % Each calibration matrix is 3x3, stored as a row of 9 elements
b_acc_all = zeros(M, 3); % Accelerometer biases
b_gyro_all = zeros(M, 3); % Gyroscope biases

% Loop over each IMU
for m = 1:M
    % Extract calibration parameters for IMU m
    theta_m = theta(m, :);
    
    % Scale factors
    K = diag(theta_m(1:3)); % 3x3 diagonal matrix
    % Accelerometer biases
    b_acc = theta_m(4:6)'; % 3x1 vector
    % Misalignment matrix
    S = eye(3);
    S(1,2) = -theta_m(7);
    S(1,3) = theta_m(8);
    S(2,3) = -theta_m(9);
    % Gyroscope biases
    b_gyro = theta_m(10:12)'; % 3x1 vector
    
    % Combined calibration matrix for accelerometer
    C_acc = inv(K * S); % 3x3 matrix
    
    % Store calibration matrix and biases
    C_acc_all(m, :) = C_acc(:)'; % Store as row vector
    b_acc_all(m, :) = b_acc';
    b_gyro_all(m, :) = b_gyro';
end

% Open the header file for writing
fid = fopen(filename, 'w');

% Write header guards
fprintf(fid, '#ifndef CALIBRATION_H\n');
fprintf(fid, '#define CALIBRATION_H\n\n');

% Include necessary headers
fprintf(fid, '#include <stdint.h>\n\n');

% Define number of IMUs
fprintf(fid, '#define NUM_IMUS %d\n\n', M);

% Write calibration matrices
fprintf(fid, 'const float C_acc[NUM_IMUS][9] = {\n');
for m = 1:M
    fprintf(fid, '    {');
    fprintf(fid, '%.10ff, ', C_acc_all(m, 1:end-1));
    fprintf(fid, '%.10ff}', C_acc_all(m, end));
    if m ~= M
        fprintf(fid, ',\n');
    else
        fprintf(fid, '\n');
    end
end
fprintf(fid, '};\n\n');

% Write accelerometer biases
fprintf(fid, 'const float b_acc[NUM_IMUS][3] = {\n');
for m = 1:M
    fprintf(fid, '    {');
    fprintf(fid, '%.10ff, ', b_acc_all(m, 1:end-1));
    fprintf(fid, '%.10ff}', b_acc_all(m, end));
    if m ~= M
        fprintf(fid, ',\n');
    else
        fprintf(fid, '\n');
    end
end
fprintf(fid, '};\n\n');

% Write gyroscope biases
fprintf(fid, 'const float b_gyro[NUM_IMUS][3] = {\n');
for m = 1:M
    fprintf(fid, '    {');
    fprintf(fid, '%.10ff, ', b_gyro_all(m, 1:end-1));
    fprintf(fid, '%.10ff}', b_gyro_all(m, end));
    if m ~= M
        fprintf(fid, ',\n');
    else
        fprintf(fid, '\n');
    end
end
fprintf(fid, '};\n\n');

% Close header guards
fprintf(fid, '#endif // CALIBRATION_H\n');

% Close the file
fclose(fid);

end
