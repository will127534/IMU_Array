# IMU Array Data Processing Board

Here is the board and code for the processing board I'm designing mainly for my geophone replacement project.
![Board photo](/IMUArray_ProcessingBoard/Images/_DSC6747-2.jpg)

## System Block Diagram
![System Block Diagram](/IMUArray_ProcessingBoard/Images/IMG_0009.JPG)

## Dictionary Structure

* IMUArray_ProcessingBoard: KiCad files for the board
* Code: C Code for the board

## Hardware Notes

So, long story short, this is essentially a custom-built Pico W using the same RP2040 + CYW43439 (Type 1YN) combo. Additionally, the crystal for the RP2040 was replaced with a VCTCXO, allowing me to tune the onboard clock to match the GPS clock (i.e., GPSDO). The code supports both NTP and GPS+PPS as time sync sources. The main reason for this is that I’m using the same crystal to send a stable 32kHz signal to the IMU array.

You might ask, why on earth do I have a full bridge rectifier along with a high-voltage DC-DC step-down converter? This was mainly because I initially intended to use my doorbell system (24V AC) to power the system. I kind of regret this decision, so it was removed in v2.0 (I’m waiting for the board to arrive).

Furthermore, I originally planned to rely on Wi-Fi to transmit the data, so the USB connector (JST-SH) wasn’t designed for external connection but only for programming. See more on the blog post. However, I ended up deciding to make the USB accessible from the case in the future v2.0 design.


## Software Notes

In the code folder, you’ll find two files. The goal for `imu_simple` is to have a much simpler codebase, so two years later, I can still understand what’s going on or easily test new ideas. `imu_main` is the main code I’m currently using, which follows the structure outlined below:

The data can be sent via either TCP or USB. If a client connects to the TCP server, it will switch to sending data through TCP. During startup, and once per hour, the system performs an NTP time sync. If GPS is available, it also uses it to calculate the UNIX timestamp offset from the internal system clock. As a result, each IMU sample contains not only the timestamp from the IMU itself but also a UNIX timestamp. However, note that these two timestamps are not strongly related (since the GPSDO requires time to adjust the error).

For IMU data processing, the code uses the method described in Reference 6 to calibrate the IMU array. Refer to the MATLAB code section for generating imu_cal.h. The data processing applies the calibration and merges the 32 IMU data into one dataset. The RP2040 is surprisingly fast enough to handle all of this using floating-point operations, even without hardware FPU, and can run at up to 500Hz. A side note here is that to measure the processing time, I placed the data processing code along with the SPI read function. By measuring the CS pin, I can determine the total time spent on both data acquisition and processing. The 500Hz rate accounts for both tasks in sequence, not in parallel (i.e., I’m not using both CPU cores for reading and processing, but instead for reading/processing and Wi-Fi/Bluetooth data processing).

Lastly, I’m not entirely sure how to share the code I wrote under the RP2040 SDK’s pico-example folder, so you may need to figure out how to set up the build environment outside of the SDK. Also, please let me know how to share it if you have suggestions.


## Footnotes

[Interactive BOM](https://htmlpreview.github.io/?https://github.com/will127534/IMU_Array/blob/main/IMUArray_ProcessingBoard/IMUArray_ProcessingBoard/bom/ibom.html) [(provided by InteractiveHtmlBom)
](https://github.com/openscopeproject/InteractiveHtmlBom) 


PIO input capture code from: https://people.ece.cornell.edu/land/courses/ece4760/RP2040/C_SDK_PIO_control/Input_capture/index_pio_control.html

