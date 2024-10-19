#ifndef IMU_FIFO_H
#define IMU_FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include "pico/mutex.h"

#define IMU_FIFO_SIZE 1500

typedef struct {
    uint16_t timestamp;
    uint8_t header;
    float temp;
    float data[6];
    uint64_t real_timestamp; // UNIX timestamp in microseconds
} imu_data_t;

typedef struct {
    imu_data_t buffer[IMU_FIFO_SIZE];
    volatile int head;  // Write index
    volatile int tail;  // Read index
    volatile int count; // Number of items in the FIFO
    mutex_t mutex;      // Mutex for protecting the FIFO buffer
} imu_fifo_t;

void imu_fifo_init(imu_fifo_t *fifo);
bool imu_fifo_push(imu_fifo_t *fifo, const imu_data_t *data);
bool imu_fifo_pop(imu_fifo_t *fifo, imu_data_t *data);
int imu_fifo_count(imu_fifo_t *fifo);

#endif // IMU_FIFO_H
