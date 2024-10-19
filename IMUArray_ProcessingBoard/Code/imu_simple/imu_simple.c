#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"

#include "main_bitmap.bin.h"
#include "ICM42688.h"
#include "inputCapture.pio.h"


#define IMU_INT 0
#define IMU_CS 1
#define FPGA_RST 2
#define FPGA_CDONE 3
#define IMU_CLK 5
#define FPGA_RX 8
#define FPGA_CS 9
#define FPGA_SCK 10
#define FPGA_TX 11

#define GPS_RX 12
#define GPS_TX 13
#define GPS_PPS 14

#define VC 17

#define LED_IMU 18
#define LED_LOCK 19
#define LED_PPS 20
#define LED_WIFI 21

#define SYS_CLOCK_KHz 240 * 1000 

#define FPGA_READ 0x55

#define IMU_50Hz 0x01
#define IMU_100Hz 0x02
#define IMU_200Hz 0x03
#define IMU_250Hz 0x04
#define IMU_500Hz 0x05


volatile bool LED_IMU_STATE = 0;
volatile bool LED_PPS_STATE = 0;

// Indicate when IMU data is being printed to make sure we are not stepping over with pps stats output
volatile bool imu_printing = false;

uint8_t dummy[17] = {0};
uint8_t IMU_data[512] = {0};

// Simple GPSDO 
#define WINDOW_SIZE 10

PIO pio_inputCapture = pio0;
uint lastPPSTimestamp = 0;
const float Kp = 0.1f;    // Proportional gain, adjust as needed
const float alpha = 0.5f;   // Low-pass filter constant (smoothing factor)
const uint TARGET_FREQ = SYS_CLOCK_KHz/2; // Target frequency to match PPS
const uint MAX_PWM = 8192;  // Maximum PWM value for VCXO control
const uint MIN_PWM = 0;     // Minimum PWM value for VCXO control
float pwm_value = 3846.0f;  // Initial VCXO control value (midpoint)
float filteredFreqError = 0.0f;  // Variable to hold the filtered error
int measurementCount = 0;   // Counter to skip initial measurements
const int SKIP_MEASUREMENTS = 2; // Number of measurements to skip for startup
int freqWindow[WINDOW_SIZE] = {0};  // Circular buffer to store frequency samples
int sampleIndex = 0;  // Index to keep track of the current position in the window
int validSamples = 0;  // Counter to keep track of how many valid samples we have

// Structure to buffer PPS data
typedef struct {
    uint32_t delta;
    float freqError;
    uint32_t pwm_value;
    bool valid;
} pps_data_t;

volatile pps_data_t pps_buffer = {0};



// Function to combine two bytes into a 16-bit integer
int16_t combine_bytes(uint8_t low, uint8_t high) {
    return (int16_t)((high << 8) | low);
}

uint16_t combine_bytes_uint(uint8_t low, uint8_t high) {
    return (uint16_t)((high << 8) | low);
}

// Write to register function
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

// Reading function for IMU
void readIMU() {
    imu_printing = true;  // Set the flag to indicate IMU data is being printed
    LED_IMU_STATE = !LED_IMU_STATE;
    gpio_put(LED_IMU, LED_IMU_STATE);

    //First: "Read" the IMU data, but the data is actually sent to FPGA
    spi_set_baudrate(spi1,10*1000*1000);
    spi_set_format(spi1,8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_put(FPGA_CS, 0);
    gpio_put(IMU_CS, 0);
    
    dummy[0] = ICM42688_FIFO_DATA | 0x80;
    spi_write_blocking(spi1, dummy, sizeof(dummy));
    gpio_put(IMU_CS, 1);
    gpio_put(FPGA_CS, 1);

    //Second: Actually read the data from FPGA
    spi_set_baudrate(spi1,20*1000*1000);
    gpio_put(FPGA_CS, 0);
    dummy[0] = 0x55;
    spi_write_blocking(spi1, dummy, 1); //Sending header
    spi_set_format(spi1,8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    spi_read_blocking(spi1, 0x00, IMU_data, 512);
    gpio_put(FPGA_CS, 1);
    
    //Print the data out in csv format, note that this is kinda CPU heavy for the receiver side also, but this is still done like this for readability reasons
    for(int i=0;i<32;i++){
        int idx = i * 16;
        uint8_t header = IMU_data[idx]; // 1 byte for header
        int16_t acc_x = combine_bytes(IMU_data[idx + 2], IMU_data[idx + 1]);
        int16_t acc_y = combine_bytes(IMU_data[idx + 4], IMU_data[idx + 3]);
        int16_t acc_z = combine_bytes(IMU_data[idx + 6], IMU_data[idx + 5]);
        int16_t gyro_x = combine_bytes(IMU_data[idx + 8], IMU_data[idx + 7]);
        int16_t gyro_y = combine_bytes(IMU_data[idx + 10], IMU_data[idx + 9]);
        int16_t gyro_z = combine_bytes(IMU_data[idx + 12], IMU_data[idx + 11]);
        uint8_t temp = IMU_data[idx + 13];
        uint16_t timestamp = combine_bytes_uint(IMU_data[idx + 15], IMU_data[idx + 14]);
        printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,",header,timestamp,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,temp);
    }
    printf("\n");
    imu_printing = false;  // Clear the flag after printing
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
    write_reg_imu(ICM42688_FIFO_CONFIG1, 0x07);  // FIFO_TEMP_EN + FIFO_GYRO_EN + FIFO_ACCEL_EN
    write_reg_imu(ICM42688_FIFO_CONFIG,  0x40);  // FIFO Enable

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
    printf("Initializing FPGA Binary\n");
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
    printf("Initializing FPGA Binary Done\n");
}


void gpio_callback() {
    readIMU();  // Read IMU data and store in FIFO
}


static void pio_irq_func(void) {
    while(!pio_sm_is_rx_fifo_empty(pio_inputCapture, 0)) {
        uint currentPPSTimestamp = pio_inputCapture->rxf[0];
        if (currentPPSTimestamp == 4294967294){
            continue;
        }
        // Skip initial measurements to allow system to stabilize
        if (measurementCount < SKIP_MEASUREMENTS) {
            lastPPSTimestamp = currentPPSTimestamp;  // Initialize last timestamp
            measurementCount++;
            continue;  // Skip the current iteration
        }
        LED_PPS_STATE = !LED_PPS_STATE;
        gpio_put(LED_PPS, LED_PPS_STATE);
        // Calculate the current frequency using floating-point math

        uint32_t delta = lastPPSTimestamp - currentPPSTimestamp;

        int freqErrorRAW = (int)delta - (int)TARGET_FREQ*1000;

        if (abs(freqErrorRAW) > 1000000){
            //printf("[INT] %u - %u %u\n", freqErrorRAW, lastPPSTimestamp, currentPPSTimestamp);
            lastPPSTimestamp = currentPPSTimestamp;  // Initialize last timestamp
            continue;
        }


        // Update the circular buffer with the new frequency value
        freqWindow[sampleIndex] = freqErrorRAW;
        sampleIndex = (sampleIndex + 1) % WINDOW_SIZE;  // Move to the next position in the buffer
        // Increment valid sample count up to WINDOW_SIZE
        if (validSamples < WINDOW_SIZE) {
            validSamples++;
        }

        // Calculate the average frequency over the last valid samples
        float freqError = 0.0f;
        for (int i = 0; i < validSamples; i++) {
            freqError += freqWindow[i];
        }
        freqError /= validSamples;

        // Adjust the VCXO control voltage (proportional control)
        pwm_value -= Kp * freqError;  // Adjust PWM value based on filtered error

        // Clamp the PWM value within the valid range
        if (pwm_value > MAX_PWM) {
            pwm_value = MAX_PWM;
        } else if (pwm_value < MIN_PWM) {
            pwm_value = MIN_PWM;
        }

        // Send the adjusted voltage to the external VCXO via PWM
        pwm_set_gpio_level(VC, (uint)pwm_value);

        // Print the current frequency, error, and PWM value for debugging
        //printf("[INT] Raw counter: %u, Averaged error: %f, PWM Value: %d\n", delta, freqError, (uint)pwm_value);
        pps_buffer.delta = delta;
        pps_buffer.freqError = freqError;
        pps_buffer.pwm_value = (uint)pwm_value;
        pps_buffer.valid = true;

        // Update the last timestamp for the next PPS cycle
        lastPPSTimestamp = currentPPSTimestamp;
    }
}

void capture_program_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_gpio_init(pio, pin); 
    pio_sm_config c = capture_program_get_default_config(offset);  
    sm_config_set_jmp_pin (&c, pin) ;
    sm_config_set_fifo_join (&c, PIO_FIFO_JOIN_RX) ;

    irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_func);
    irq_set_enabled(PIO0_IRQ_0, true);

    pio_set_irqn_source_enabled(pio, 0, pis_sm0_rx_fifo_not_empty + sm, true);
    pio_sm_init(pio, sm, offset, &c);
}

int main() {

    set_sys_clock_khz(SYS_CLOCK_KHz, true);
    stdio_init_all();
    spi_init(spi1, 20 * 1000 * 1000);


    //Initialize GPIOs
    gpio_set_function(FPGA_RX, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_SCK, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_TX, GPIO_FUNC_SPI);

    gpio_init(FPGA_RST);
    gpio_set_dir(FPGA_RST, GPIO_OUT);
    gpio_put(FPGA_RST, 1);

    gpio_init(FPGA_CS);
    gpio_set_dir(FPGA_CS, GPIO_OUT);
    gpio_put(FPGA_CS, 1);

    gpio_init(IMU_CS);
    gpio_set_dir(IMU_CS, GPIO_OUT);
    gpio_put(IMU_CS, 1);

    gpio_init(IMU_INT);
    gpio_set_dir(IMU_INT, GPIO_IN);

    gpio_init(LED_IMU);
    gpio_set_dir(LED_IMU, GPIO_OUT);
    gpio_init(LED_LOCK);
    gpio_set_dir(LED_LOCK, GPIO_OUT);
    gpio_init(LED_PPS);
    gpio_set_dir(LED_PPS, GPIO_OUT);
    gpio_init(LED_WIFI);
    gpio_set_dir(LED_WIFI, GPIO_OUT);

    gpio_put(LED_IMU, 1);
    gpio_put(LED_LOCK, 1);
    gpio_put(LED_PPS, 1);
    gpio_put(LED_WIFI, 1);


    //Initialize VC for the VCTCXO for clock input to RP2040
    gpio_set_function(VC, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(VC);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config,8192);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(VC,3846);



    sleep_ms(5000);
    initFPGA();

    //Initialize IMU_CLK to output a 32Khz clock for the IMUs
    //240Mhz / 250 (clk_div) / 30 (wrap) = 32Khz 
    printf("Setting up IMU_CLK\n");
    gpio_set_function(IMU_CLK, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(IMU_CLK);

    config = pwm_get_default_config();
    pwm_config_set_wrap(&config,29);
    pwm_init(slice_num, &config, true);
    pwm_set_clkdiv(slice_num, 250);
    pwm_set_gpio_level(IMU_CLK,15);


    //Initialize PIO code to measure PPS duration for GPSDO
    uint offset = pio_add_program(pio_inputCapture, &capture_program);   
    capture_program_init(pio_inputCapture, 0, offset, GPS_PPS);
    pio_sm_set_enabled(pio_inputCapture, 0, true);



    printf("Init IMU\n"); 
    
    init_imu(IMU_100Hz);

    // Set up IMU interrupt handler
    gpio_set_irq_enabled_with_callback(IMU_INT, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Start measurements
    start_measure();

    while (1) {
        if (pps_buffer.valid && !imu_printing) {
            printf("[INT] Raw counter: %u, Averaged error: %f, PWM Value: %d\n",
                   pps_buffer.delta, pps_buffer.freqError, pps_buffer.pwm_value);
            pps_buffer.valid = false;
        }
    }

    return 0;
}
