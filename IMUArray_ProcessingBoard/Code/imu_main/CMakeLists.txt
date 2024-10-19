file(GLOB_RECURSE tinygps tinygps/*.c)
include_directories(tinygps)

add_executable(imu_main
        imu_main.c
        imu_processing.c
        tcp_server.c
        imu_fifo.c
        ntp_sync.c
        gpsdo.c
        ${tinygps}
        )
target_compile_definitions(imu_main PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(imu_main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(imu_main
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        hardware_spi
        hardware_pwm
        hardware_pio
        pico_multicore
        )
pico_add_extra_outputs(imu_main)
pico_enable_stdio_usb(imu_main 1)
pico_enable_stdio_uart(imu_main 0)