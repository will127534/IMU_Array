#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "ICM42688.h"
#include "imu_cal.h"
#include "main_bitmap.bin.h"
#include "hardware_pins.h"
#include "gpsdo.h"
#include "time_sync.h"
#include "tcp_server.h"
#include "imu_fifo.h"
#include "imu_processing.h"

static bool LED_IMU_STATE = false;
volatile bool imu_printing = false;
extern imu_fifo_t imu_fifo;

static uint8_t rawIMUData[512] = {0};
uint8_t dummy[17] = {0};

// Function to combine two bytes into a 16-bit integer
int16_t combine_bytes(uint8_t low, uint8_t high) {
    return (int16_t)((high << 8) | low);
}

uint16_t combine_bytes_uint(uint8_t low, uint8_t high) {
    return (uint16_t)((high << 8) | low);
}

// Write to register function (updated to take uint8_t value)
void write_reg_imu(uint8_t reg, uint8_t data) {
    gpio_put(IMU_CS, 0);
    uint8_t packet[] = {reg, data};
    spi_write_blocking(spi1, packet, sizeof(packet));
    gpio_put(IMU_CS, 1);
}

// Read from register function
uint8_t read_reg_imu(uint8_t reg) {
    gpio_put(IMU_CS, 0);
    uint8_t packet_out[] = {reg | 0x80, 0x00};
    uint8_t packet_in[2] = {0};
    spi_write_read_blocking(spi1, packet_out, packet_in, 2);
    gpio_put(IMU_CS, 1);
    return packet_in[1];
}

void processData(uint8_t *data, int data_length, imu_data_t* outputData) {
    const int num_imus = NUM_IMUS; // 32
    const int imu_data_size = 16; // bytes per IMU

    // Arrays to hold calibrated data
    float calibrated_acc[NUM_IMUS][3];
    float calibrated_gyro[NUM_IMUS][3];

    // Scaling factors
    float acc_scale = (2 * 9.81f) / 32768.0f; // ±2g range
    float gyro_scale = (15.625f / 32768.0f); // ±15.625 deg/s range

    float avg_temp = 0;
    uint16_t timestamp = combine_bytes_uint(data[15], data[14]);
    uint8_t header = data[0]; 

    // Loop over each IMU
    for (int no = 0; no < num_imus; no++) {
        int idx = no * imu_data_size;

        //uint8_t header = data[idx]; // 1 byte for header
        int16_t acc_x = combine_bytes(data[idx + 2], data[idx + 1]);
        int16_t acc_y = combine_bytes(data[idx + 4], data[idx + 3]);
        int16_t acc_z = combine_bytes(data[idx + 6], data[idx + 5]);
        int16_t gyro_x = combine_bytes(data[idx + 8], data[idx + 7]);
        int16_t gyro_y = combine_bytes(data[idx + 10], data[idx + 9]);
        int16_t gyro_z = combine_bytes(data[idx + 12], data[idx + 11]);
        uint8_t temp = data[idx + 13];
        //uint16_t timestamp = combine_bytes_uint(data[idx + 15], data[idx + 14]);

        avg_temp += temp;

        // Convert raw data to physical units
        float acc_raw[3];
        float gyro_raw[3];

        acc_raw[0] = acc_x * acc_scale;
        acc_raw[1] = acc_y * acc_scale;
        acc_raw[2] = acc_z * acc_scale;

        gyro_raw[0] = gyro_x * gyro_scale;
        gyro_raw[1] = gyro_y * gyro_scale;
        gyro_raw[2] = gyro_z * gyro_scale;

        // Apply orientation correction based on IMU group
        // Define IMU groups
        int group;
        if (no >= 0 && no <= 7) {
            group = 1; // Group 1: IMUs 1-8 (indices 0-7)
        } else if (no >= 8 && no <= 15) {
            group = 2; // Group 2: IMUs 9-16 (indices 8-15)
        } else if (no >= 16 && no <= 23) {
            group = 3; // Group 3: IMUs 17-24 (indices 16-23)
        } else {
            group = 4; // Group 4: IMUs 25-32 (indices 24-31)
        }

        // Orientation correction
        float acc_corrected[3];
        float gyro_corrected[3];

        switch (group) {
            case 1:
                // Group 1: No changes
                acc_corrected[0] = acc_raw[0];
                acc_corrected[1] = acc_raw[1];
                acc_corrected[2] = acc_raw[2];

                gyro_corrected[0] = gyro_raw[0];
                gyro_corrected[1] = gyro_raw[1];
                gyro_corrected[2] = gyro_raw[2];
                break;
            case 2:
                // Group 2: Swap Acc_X and Acc_Y, negate new Acc_X
                acc_corrected[0] = -acc_raw[1]; // Acc_X = -Acc_Y
                acc_corrected[1] = acc_raw[0];  // Acc_Y = Acc_X
                acc_corrected[2] = acc_raw[2];  // Acc_Z remains the same

                // Gyro: Swap Gyro_X and Gyro_Y, negate new Gyro_X
                gyro_corrected[0] = -gyro_raw[1]; // Gyro_X = -Gyro_Y
                gyro_corrected[1] = gyro_raw[0];  // Gyro_Y = Gyro_X
                gyro_corrected[2] = gyro_raw[2];  // Gyro_Z remains the same
                break;
            case 3:
                // Group 3: Negate Acc_X and Acc_Y
                acc_corrected[0] = -acc_raw[0]; // Acc_X = -Acc_X
                acc_corrected[1] = -acc_raw[1]; // Acc_Y = -Acc_Y
                acc_corrected[2] = acc_raw[2];  // Acc_Z remains the same

                // Gyro: Negate Gyro_X and Gyro_Y
                gyro_corrected[0] = -gyro_raw[0]; // Gyro_X = -Gyro_X
                gyro_corrected[1] = -gyro_raw[1]; // Gyro_Y = -Gyro_Y
                gyro_corrected[2] = gyro_raw[2];  // Gyro_Z remains the same
                break;
            case 4:
                // Group 4: Swap Acc_X and Acc_Y, negate new Acc_Y
                acc_corrected[0] = acc_raw[1];   // Acc_X = Acc_Y
                acc_corrected[1] = -acc_raw[0];  // Acc_Y = -Acc_X
                acc_corrected[2] = acc_raw[2];   // Acc_Z remains the same

                // Gyro: Swap Gyro_X and Gyro_Y, negate new Gyro_Y
                gyro_corrected[0] = gyro_raw[1];   // Gyro_X = Gyro_Y
                gyro_corrected[1] = -gyro_raw[0];  // Gyro_Y = -Gyro_X
                gyro_corrected[2] = gyro_raw[2];   // Gyro_Z remains the same
                break;
            default:
                // Should not reach here
                acc_corrected[0] = acc_raw[0];
                acc_corrected[1] = acc_raw[1];
                acc_corrected[2] = acc_raw[2];

                gyro_corrected[0] = gyro_raw[0];
                gyro_corrected[1] = gyro_raw[1];
                gyro_corrected[2] = gyro_raw[2];
                break;
        }

        // Apply calibration
        // Calibration matrix C_acc[no][9], biases b_acc[no][3], gyroscope biases b_gyro[no][3]
        float acc_calibrated[3];
        float gyro_calibrated[3];

        const float *C = C_acc[no]; // Calibration matrix for IMU no
        const float *b = b_acc[no]; // Accelerometer biases for IMU no
        const float *b_g = b_gyro[no]; // Gyroscope biases for IMU no

        // Calibrate accelerometer data: acc_calibrated = C * (acc_corrected - b)
        acc_calibrated[0] = C[0]* (acc_corrected[0] - b[0]) + C[1]* (acc_corrected[1] - b[1]) + C[2]* (acc_corrected[2] - b[2]);
        acc_calibrated[1] = C[3]* (acc_corrected[0] - b[0]) + C[4]* (acc_corrected[1] - b[1]) + C[5]* (acc_corrected[2] - b[2]);
        acc_calibrated[2] = C[6]* (acc_corrected[0] - b[0]) + C[7]* (acc_corrected[1] - b[1]) + C[8]* (acc_corrected[2] - b[2]);

        // Calibrate gyroscope data
        gyro_calibrated[0] = gyro_corrected[0] - b_g[0];
        gyro_calibrated[1] = gyro_corrected[1] - b_g[1];
        gyro_calibrated[2] = gyro_corrected[2] - b_g[2];

        // Store calibrated data
        calibrated_acc[no][0] = acc_calibrated[0];
        calibrated_acc[no][1] = acc_calibrated[1];
        calibrated_acc[no][2] = acc_calibrated[2];

        calibrated_gyro[no][0] = gyro_calibrated[0];
        calibrated_gyro[no][1] = gyro_calibrated[1];
        calibrated_gyro[no][2] = gyro_calibrated[2];
    }

    // Perform data fusion (averaging)
    float avg_acc[3] = {0, 0, 0};
    float avg_gyro[3] = {0, 0, 0};

    for (int no = 0; no < num_imus; no++) {
        avg_acc[0] += calibrated_acc[no][0];
        avg_acc[1] += calibrated_acc[no][1];
        avg_acc[2] += calibrated_acc[no][2];

        avg_gyro[0] += calibrated_gyro[no][0];
        avg_gyro[1] += calibrated_gyro[no][1];
        avg_gyro[2] += calibrated_gyro[no][2];

        //printf("%d,%f,%f,%f,%f,%f,%f\n", no, calibrated_acc[no][0],calibrated_acc[no][1],calibrated_acc[no][2],calibrated_gyro[no][0],calibrated_gyro[no][1],calibrated_gyro[no][2]);
    }

    avg_acc[0] /= num_imus;
    avg_acc[1] /= num_imus;
    avg_acc[2] /= num_imus;

    avg_gyro[0] /= num_imus;
    avg_gyro[1] /= num_imus;
    avg_gyro[2] /= num_imus;

    avg_temp /= num_imus;
    avg_temp = avg_temp / 2.07 + 25.0;

    // Output the fused data
    //printf("Averaged Data: (%f, %f, %f),(%f, %f, %f)\n", avg_acc[0], avg_acc[1], avg_acc[2],avg_gyro[0], avg_gyro[1], avg_gyro[2]);

    outputData->header = header;
    outputData->timestamp = timestamp;
    outputData->temp = avg_temp;
    outputData->data[0] = avg_acc[0];
    outputData->data[1] = avg_acc[1];
    outputData->data[2] = avg_acc[2];
    outputData->data[3] = avg_gyro[0];
    outputData->data[4] = avg_gyro[1];
    outputData->data[5] = avg_gyro[2];
}



void readIMU() {
    // Get the current system clock time
    uint64_t imu_sysclock_us = time_us_64();

    LED_IMU_STATE = !LED_IMU_STATE;
    gpio_put(LED_IMU, LED_IMU_STATE);
    spi_set_baudrate(spi1,10*1000*1000);
    spi_set_format(spi1,8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_put(FPGA_CS, 0);
    gpio_put(IMU_CS, 0);
    
    dummy[0] = ICM42688_FIFO_DATA | 0x80;
    spi_write_blocking(spi1, dummy, sizeof(dummy));
    gpio_put(IMU_CS, 1);
    gpio_put(FPGA_CS, 1);

    spi_set_baudrate(spi1,20*1000*1000);
    gpio_put(FPGA_CS, 0);
    dummy[0] = 0x55;
    spi_write_blocking(spi1, dummy, 1); //Sending header
    spi_set_format(spi1,8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_read_blocking(spi1, 0x00, rawIMUData, 512);

    // Compute the real timestamp
    uint64_t real_timestamp = 0;
    if (pps_received || ntp_time_valid) {
        real_timestamp = imu_sysclock_us + sysclock_to_unixtime_offset_us;
    }

    if (tcp_connected) {
        // Prepare to write into the FIFO
        mutex_enter_blocking(&imu_fifo.mutex);
        if (imu_fifo.count < IMU_FIFO_SIZE) {
            int index = imu_fifo.head;
            processData(rawIMUData,512,&imu_fifo.buffer[index]);
            imu_fifo.buffer[index].real_timestamp = real_timestamp;

            // Update FIFO indices
            imu_fifo.head = (imu_fifo.head + 1) % IMU_FIFO_SIZE;
            imu_fifo.count++;

        } else {
            // FIFO is full, data will be lost
            printf("[IMU] !!!!!!!!!!!IMU FIFO is full, data lost!!!!!!!!!!!!!!!\n");
        }
        mutex_exit(&imu_fifo.mutex);
    }
    else{
        //print the data via USB CDC in binary:
        imu_printing = true;  // Set the flag to indicate IMU data is being printed

        #ifdef PRINTRAW
            printf("[IMU]%" PRIu64 ",", real_timestamp);
            //fwrite(data, 1, 512, stdout);
            for (size_t i = 0; i < 512; ++i) {
                printf("%02X", rawIMUData[i]);
            }
            printf("\n");
        #else
            imu_data_t output;
            processData(rawIMUData,512,&output);
            printf("[IMU]%" PRIu64 ",%d,%d,%f,%f,%f,%f,%f,%f,%.1f\n",real_timestamp,output.timestamp,output.header,output.data[0],output.data[1],output.data[2],output.data[3],output.data[4],output.data[5],output.temp);
        #endif 

        imu_printing = false;  // Clear the flag after printing    
    }

    gpio_put(FPGA_CS, 1);
}

// Start measurement function
void start_measure() {
    write_reg_imu(ICM42688_REG_BANK_SEL, 0x00);   // Select register bank 0
    write_reg_imu(ICM42688_PWR_MGMT0, 0x0F);      // Enable measurements
}

// Initialization function for IMU
void init_imu(int rate) {
    spi_set_baudrate(spi1,10*1000*1000);
    spi_set_format(spi1,8, SPI_CPOL_0, SPI_CPHA_0,SPI_MSB_FIRST);
    // Soft reset
    write_reg_imu(ICM42688_REG_BANK_SEL, 0x00);   // Select register bank 0
    write_reg_imu(ICM42688_DEVICE_CONFIG, 0x01);  // Soft reset
    sleep_ms(100);  // 100ms delay for soft reset

    // Configure gyro and accel based on rate
    switch (rate) {
      case IMU_500Hz:
        write_reg_imu(ICM42688_GYRO_CONFIG0,  0xEF); // 15.625dps + 500Hz
        write_reg_imu(ICM42688_ACCEL_CONFIG0, 0x6F); // 2g + 500Hz
        break;
      case IMU_200Hz:
        write_reg_imu(ICM42688_GYRO_CONFIG0,  0xE7); // 15.625dps + 200Hz
        write_reg_imu(ICM42688_ACCEL_CONFIG0, 0x67); // 2g + 200Hz
        break;
      case IMU_100Hz:
        write_reg_imu(ICM42688_GYRO_CONFIG0,  0xE8); // 15.625dps + 100Hz
        write_reg_imu(ICM42688_ACCEL_CONFIG0, 0x68); // 2g + 100Hz
        break;
      case IMU_50Hz:
        write_reg_imu(ICM42688_GYRO_CONFIG0,  0xE9); // 15.625dps + 50Hz
        write_reg_imu(ICM42688_ACCEL_CONFIG0, 0x69); // 2g + 50Hz
        break;
      default:
        write_reg_imu(ICM42688_GYRO_CONFIG0,  0xE9); // 15.625dps + 50Hz
        write_reg_imu(ICM42688_ACCEL_CONFIG0, 0x69); // 2g + 50Hz
    }

    // Enable FIFO
    write_reg_imu(ICM42688_FIFO_CONFIG1, 0x07);   // FIFO_TEMP_EN + FIFO_GYRO_EN + FIFO_ACCEL_EN
    write_reg_imu(ICM42688_FIFO_CONFIG,  0x40);   // FIFO Enable

    // Configure interface and interrupts
    write_reg_imu(ICM42688_INTF_CONFIG1, 0x95);  // RTC_MODE

    // Select register bank 1
    write_reg_imu(ICM42688_REG_BANK_SEL, 0x01);  // Select register bank 1

    // Enable CLKIN
    write_reg_imu(ICM42688_INTF_CONFIG5, 0x04);  // CLKIN Enable

    // Select register bank 0
    write_reg_imu(ICM42688_REG_BANK_SEL, 0x00);  // Select register bank 0

    // Timestamp configuration and interrupt routing
    write_reg_imu(ICM42688_TMST_CONFIG, 0x2B);    // Timestamp configuration
    write_reg_imu(ICM42688_INT_CONFIG,  0x02);    // INT1 drive circuit = Push pull
    write_reg_imu(ICM42688_INT_SOURCE0, 0x08);    // UI data ready interrupt routed to INT1
    write_reg_imu(ICM42688_INT_CONFIG1, 0x00);    // Disable INT pulse mode


}

void initFPGA(){
    printf("[INIT] Initializing FPGA Binary\n");
    uint8_t dummy[4] = {0};
    gpio_put(FPGA_RST, 0);
    sleep_ms(100);
    gpio_put(FPGA_RST, 1);
    gpio_put(FPGA_CS, 0);
    sleep_ms(100);
    gpio_put(FPGA_CS, 1);
    spi_set_format(spi1,8, SPI_CPOL_1, SPI_CPHA_1,SPI_MSB_FIRST);
    //8 byte empty data
    for (int i=0;i<2;i++){
        spi_write_blocking(spi1, dummy, sizeof(dummy));
    }
    spi_write_blocking(spi1, FPGA_BINARY, sizeof(FPGA_BINARY));
    //100 byte empty data
    for (int i=0;i<25;i++){
        spi_write_blocking(spi1, dummy, sizeof(dummy));
    }
    printf("[INIT] Initializing FPGA Binary Done\n");
}