# ioLibrary_Driver
add_library(IOLIBRARY_FILES STATIC)

target_sources(IOLIBRARY_FILES PUBLIC
        ${PORT_DIR}/ioLibrary_Driver/src/wizchip_spi.c
        ${PORT_DIR}/ioLibrary_Driver/src/wizchip_gpio_irq.c
        )

if(${BOARD_NAME} STREQUAL W55RP20_EVB_PICO OR ${WIZNET_CHIP} STREQUAL W6300)
pico_generate_pio_header(IOLIBRARY_FILES ${PORT_DIR}/ioLibrary_Driver/src/wizchip_qspi_pio.pio)

target_sources(IOLIBRARY_FILES PUBLIC 
        ${PORT_DIR}/ioLibrary_Driver/src/wizchip_qspi_pio.c
)

target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
)
endif()

target_include_directories(IOLIBRARY_FILES PUBLIC
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/ioLibrary_Driver/inc
        ${PORT_DIR}
        )

target_link_libraries(IOLIBRARY_FILES PUBLIC
        pico_stdlib
        hardware_pio
        hardware_spi
        hardware_dma
        hardware_clocks
        )

# timer
add_library(TIMER_FILES STATIC)

target_sources(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer/timer.c
        )

target_include_directories(TIMER_FILES PUBLIC
        ${PORT_DIR}/timer
        )

target_link_libraries(TIMER_FILES PRIVATE
        pico_stdlib      
        )

# aws-iot-device-sdk-embedded-C
add_library(AWS_SDK_PORT_FILES STATIC)

target_sources(AWS_SDK_PORT_FILES PUBLIC
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/dns_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/http_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/mqtt_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/ssl_transport_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/timer_interface.c
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/src/util.c
        )

target_include_directories(AWS_SDK_PORT_FILES PUBLIC
        ${PORT_DIR}/aws-iot-device-sdk-embedded-C/inc
        ${MBEDTLS_DIR}/include
        )

target_link_libraries(AWS_SDK_PORT_FILES PUBLIC
        pico_stdlib
        ETHERNET_FILES
        DHCP_FILES
        DNS_FILES
        AWS_SDK_FILES
        mbedcrypto
        mbedx509
        mbedtls
        )