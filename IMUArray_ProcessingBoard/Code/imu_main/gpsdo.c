#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "pico/sem.h"
#include "pico/multicore.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "inputCapture.pio.h"
#include "time_sync.h"
#include "imu_processing.h"
#include "gpsdo.h"
#include "config.h"

// Pin Definitions (adjust as needed)
#define GPS_PPS 14
#define VC 17
#define LED_PPS 20

// Global Variables
volatile uint64_t last_pps_sysclock_us = 0;           // System clock time when last PPS interrupt occurred
volatile bool pps_received = false;                   // Flag indicating PPS received
volatile bool gps_time_valid = false;                 // Flag indicating if GPS time is valid
volatile time_t gps_last_received_unixtime = 0; 


static uint lastPPSTimestamp = 0;
static const float Kp = 0.1f;    // Proportional gain, adjust as needed
static const uint TARGET_FREQ = SYS_CLOCK_KHz / 2; // Target frequency to match PPS
static const uint MAX_PWM = 8192;  // Maximum PWM value for VCXO control
static const uint MIN_PWM = 0;     // Minimum PWM value for VCXO control
static float pwm_value = 3846.0f;  // Initial VCXO control value (midpoint)
static int measurementCount = 0;   // Counter to skip initial measurements
static const int SKIP_MEASUREMENTS = 2; // Number of measurements to skip for startup
static int freqWindow[WINDOW_SIZE] = {0};  // Circular buffer to store frequency samples
static int sampleIndex = 0;  // Index to keep track of the current position in the window
static int validSamples = 0;  // Counter to keep track of how many valid samples we have

// PPS Interrupt Handler
void gpsdo_pio_irq_handler(void) {
    // Get current system clock time in microseconds
    uint64_t current_sysclock_us = time_us_64();

    while(!pio_sm_is_rx_fifo_empty(pio0, 0)) {
        uint currentPPSTimestamp = pio0->rxf[0];
        if (currentPPSTimestamp == 4294967294){
            continue;
        }
        // Skip initial measurements to allow system to stabilize
        if (measurementCount < SKIP_MEASUREMENTS) {
            lastPPSTimestamp = currentPPSTimestamp;  // Initialize last timestamp
            measurementCount++;
            continue;  // Skip the current iteration
        }
        // Toggle PPS LED
        gpio_put(LED_PPS, !gpio_get(LED_PPS));
        // Calculate the current frequency using floating-point math

        uint32_t delta = lastPPSTimestamp - currentPPSTimestamp;

        int freqErrorRAW = (int)delta - (int)TARGET_FREQ * 1000;

        if (abs(freqErrorRAW) > 1000000){
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

        while(imu_printing);
        // Print the current frequency, error, and PWM value for debugging
        printf("[PPS] Raw counter: %u, Averaged error: %f, PWM Value: %d\n", delta, freqError, (uint)pwm_value);

        // Update the last timestamp for the next PPS cycle
        lastPPSTimestamp = currentPPSTimestamp;

        if (gps_time_valid) {
            // We have a new GPS time to synchronize, note the timestamp we receive is from the last second
            uint64_t gps_unixtime_us = ((uint64_t)gps_last_received_unixtime + 1) * 1000000ULL;
            sysclock_to_unixtime_offset_us = gps_unixtime_us - current_sysclock_us;
            printf("[PPS] Time synchronized to GPS: UNIX time %llu us\n", gps_unixtime_us);
        } else {
            // Neither GPS nor NTP time is available
            printf("[PPS] No valid GPS time available\n");
        }

        last_pps_sysclock_us = current_sysclock_us;
        pps_received = true;
    }
}

// Initialize GPSDO
void gpsdo_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_gpio_init(pio, pin); 
    pio_sm_config c = capture_program_get_default_config(offset);  
    sm_config_set_jmp_pin (&c, pin) ;
    sm_config_set_fifo_join (&c, PIO_FIFO_JOIN_RX) ;

    irq_set_exclusive_handler(PIO0_IRQ_0, gpsdo_pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);

    pio_set_irqn_source_enabled(pio, 0, pis_sm0_rx_fifo_not_empty + sm, true);
    pio_sm_init(pio, sm, offset, &c);
}

// Handle PPS Signal (if needed)
void gpsdo_handle_pps(void) {
    // Implementation if additional handling is required
}
