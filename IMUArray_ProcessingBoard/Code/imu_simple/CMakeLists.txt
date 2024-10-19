add_executable(imu_simple
        imu_simple.c
        )

# pull in common dependencies
target_link_libraries(
    imu_simple 
    pico_stdlib 
    hardware_spi 
    hardware_pwm 
    hardware_pio
)

# enable usb output, disable uart output
pico_enable_stdio_usb(imu_simple 1)
pico_enable_stdio_uart(imu_simple 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(imu_simple)

