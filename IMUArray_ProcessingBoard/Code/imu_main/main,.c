// main.c

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "pico/unique_id.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// Include custom headers
#include "hardware_pins.h"
#include "imu_fifo.h"
#include "imu_processing.h"
#include "ntp_sync.h"
#include "tcp_server.h"
#include "gpsdo.h"
#include "inputCapture.pio.h" // For the PIO program
#include "tinygps.h"          // For GPS handling
#include "config.h"


// Global Variables
imu_fifo_t imu_fifo;
TCP_SERVER_T *tcp_state;
NTP_T ntp_state;

// Forward Declarations
void core1_entry();
void imu_gpio_callback(uint gpio, uint32_t events);

// Main Function
int main() {
    // Set system clock frequency
    set_sys_clock_khz(SYS_CLOCK_KHZ, true);

    // Initialize standard I/O
    stdio_init_all();

    printf("[INIT] Starting up...\n");

    // Initialize IMU FIFO
    imu_fifo_init(&imu_fifo);

    // Initialize GPIO pins
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

    // Initialize SPI for FPGA and IMU
    spi_init(spi1, 20 * 1000 * 1000);

    gpio_set_function(FPGA_RX, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_SCK, GPIO_FUNC_SPI);
    gpio_set_function(FPGA_TX, GPIO_FUNC_SPI);

    // Initialize UART for GPS
    uart_init(uart0, 115200);
    gpio_set_function(GPS_RX, GPIO_FUNC_UART);
    gpio_set_function(GPS_TX, GPIO_FUNC_UART);

    // Initialize PWM for VCXO control
    gpio_set_function(VC, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(VC);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, 8192);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(VC, 3846); // Initial PWM value

    // Initialize PIO for PPS input capture
    PIO pio_inputCapture = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio_inputCapture, &capture_program);
    gpsdo_init(pio_inputCapture, sm, offset, GPS_PPS);
    pio_sm_set_enabled(pio_inputCapture, sm, true);

    // Initialize FSYNC signal
    printf("[INIT] Setting up FSYNC\n");
    gpio_set_function(FSYNC, GPIO_FUNC_PWM);
    uint fsync_slice_num = pwm_gpio_to_slice_num(FSYNC);

    pwm_config fsync_config = pwm_get_default_config();
    pwm_config_set_wrap(&fsync_config, 29);
    pwm_init(fsync_slice_num, &fsync_config, true);
    pwm_set_clkdiv(fsync_slice_num, 250);
    pwm_set_gpio_level(FSYNC, 15);

    // Initialize Wi-Fi
    if (cyw43_arch_init()) {
        printf("[INIT] Failed to initialize Wi-Fi\n");
        return 1;
    }
    cyw43_arch_enable_sta_mode();

    printf("[INIT] Connecting to Wi-Fi...\n");
    while (true) {
        if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
            printf("[INIT] Failed to connect to Wi-Fi.\n");
        } else {
            printf("[INIT] Connected to Wi-Fi.\n");
            gpio_put(LED_WIFI, 0);
            break;
        }
    }

    // Initialize TCP server
    tcp_state = tcp_server_init();
    if (!tcp_state) {
        printf("[INIT] Failed to initialize TCP server\n");
        return 1;
    }
    if (!tcp_server_open(tcp_state)) {
        printf("[INIT] Failed to open TCP server\n");
        return 1;
    }

    // Initialize NTP synchronization
    ntp_init(&ntp_state);
    ntp_sync_time(&ntp_state);

    // Initialize FPGA
    initFPGA();

    // Launch core 1 for IMU and GPS handling
    printf("[INIT] Launching core 1\n");
    multicore_launch_core1(core1_entry);

    // Main loop variables
    static uint64_t last_ntp_sync_us = 0;

    // Main Loop
    while (1) {
        // Poll the Wi-Fi driver
        cyw43_arch_poll();

        // Schedule NTP synchronization every hour
        uint64_t now_us = time_us_64();
        if ((now_us - last_ntp_sync_us) > 3600ULL * 1000000ULL) { // Every hour
            ntp_sync_time(&ntp_state);
            last_ntp_sync_us = now_us;
        }

        // Check if a client is connected
        if (tcp_state->client_pcb && tcp_state->client_connected) {
            imu_data_t imu_data;
            // Pop data from the IMU FIFO and send over TCP
            while (imu_fifo_pop(&imu_fifo, &imu_data)) {
                err_t err = tcp_server_send_data(tcp_state, tcp_state->client_pcb, (uint8_t*)&imu_data, sizeof(imu_data_t));
                if (err != ERR_OK) {
                    // Handle error
                    printf("[TCP] Error sending data: %d\n", err);
                    break;
                }
            }
        }
    }

    // Cleanup (if the main loop ever exits)
    tcp_server_deinit(tcp_state);
    cyw43_arch_deinit();
    return 0;
}

// Core 1 entry function
void core1_entry() {
    // Initialize IMU
    init_imu(IMU_200Hz);

    // Set up IMU interrupt handler
    gpio_set_irq_enabled_with_callback(IMU_INT, GPIO_IRQ_EDGE_FALL, true, &imu_gpio_callback);

    // Start IMU measurements
    start_measure();

    // Core 1 main loop
    while (1) {
        // Handle GPS data
        if (uart_is_readable(uart0)) {
            uint8_t ch = uart_getc(uart0);
            if (gps_encode(ch)) {
                int year;
                uint8_t month, day, hour, minute, second;
                gps_crack_datetime(&year, &month, &day, &hour, &minute, &second, NULL, NULL);

                // Convert GPS time to UNIX time
                struct tm t = {
                    .tm_year = year - 1900,
                    .tm_mon  = month - 1,
                    .tm_mday = day,
                    .tm_hour = hour,
                    .tm_min  = minute,
                    .tm_sec  = second
                };
                time_t gps_unixtime = mktime(&t);

                // Store the GPS time to be used at the next PPS interrupt
                gps_last_received_unixtime = gps_unixtime;
                gps_time_valid = true;

                // Optional: Print the time for debugging
                char timeStr[100] = {0};
                strftime(timeStr, sizeof(timeStr), "%Y/%m/%d %H:%M:%S", &t);

                printf("[GPS] GPS Time received: %s, GPS sat: %d\n", timeStr, gps_satellites());

                if (gps_satellites() > 4) {
                    gpio_put(LED_LOCK, 0);
                } else {
                    gpio_put(LED_LOCK, 1);
                }
            }
        }
    }
}

// IMU INT interrupt callback
void imu_gpio_callback(uint gpio, uint32_t events) {
    readIMU();  // Read IMU data and store in FIFO
}
