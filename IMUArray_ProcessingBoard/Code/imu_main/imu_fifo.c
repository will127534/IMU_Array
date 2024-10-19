#include "imu_fifo.h"

void imu_fifo_init(imu_fifo_t *fifo) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    mutex_init(&fifo->mutex);
}

bool imu_fifo_push(imu_fifo_t *fifo, const imu_data_t *data) {
    bool result = false;
    mutex_enter_blocking(&fifo->mutex);
    if (fifo->count < IMU_FIFO_SIZE) {
        fifo->buffer[fifo->head] = *data;
        fifo->head = (fifo->head + 1) % IMU_FIFO_SIZE;
        fifo->count++;
        result = true;
    }
    mutex_exit(&fifo->mutex);
    return result;
}

bool imu_fifo_pop(imu_fifo_t *fifo, imu_data_t *data) {
    bool result = false;
    mutex_enter_blocking(&fifo->mutex);
    if (fifo->count > 0) {
        *data = fifo->buffer[fifo->tail];
        fifo->tail = (fifo->tail + 1) % IMU_FIFO_SIZE;
        fifo->count--;
        result = true;
    }
    mutex_exit(&fifo->mutex);
    return result;
}

int imu_fifo_count(imu_fifo_t *fifo) {
    int count;
    mutex_enter_blocking(&fifo->mutex);
    count = fifo->count;
    mutex_exit(&fifo->mutex);
    return count;
}
