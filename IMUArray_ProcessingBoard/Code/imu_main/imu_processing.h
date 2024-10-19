#ifndef IMU_PROCESSING_H
#define IMU_PROCESSING_H

#include <stdint.h>
#include "imu_fifo.h"

#define IMU_50Hz 0x01
#define IMU_100Hz 0x02
#define IMU_200Hz 0x03
#define IMU_250Hz 0x04
#define IMU_500Hz 0x05


void init_imu(int rate);
void readIMU();
void processData(uint8_t *data, int data_length, imu_data_t* outputData);
void start_measure();
void initFPGA();

extern volatile bool imu_printing;

#endif // IMU_PROCESSING_H
