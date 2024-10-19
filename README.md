# IMU sensor Array   

## Matlab
Here are the dataset collected with 100Hz on 32 IMUs, with data recording + analyzer code based on Matlab.

![Accelerometer_Allan_Deviation](/Matlab/Accelerometer_Allan_Deviation.jpg)
![Gyroscope_Allan_Deviation](/Matlab/Gyroscope_Allan_Deviation.jpg)


## Matlab Calibtaion
Based on the paper [Aligning the Forces—Eliminating the Misalignments in IMU Arrays](https://ieeexplore.ieee.org/document/6879287) - by John-Olof Nilsson et. al and their [code](https://www.openshoe.org/?m=201408).It will generate a imu_cal.h for the C code for the IMU processing board and theta.mat for other matlab code to do post analyze.