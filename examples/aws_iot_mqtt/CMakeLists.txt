set(TARGET_NAME aws_iot_mqtt)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
	    )

include_directories(${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include)

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_dma
        pico_aon_timer
        ETHERNET_FILES
        IOLIBRARY_FILES
        DHCP_FILES
        DNS_FILES
        SNTP_FILES
        AWS_SDK_PORT_FILES
        TIMER_FILES
        mbedcrypto
        mbedx509
        mbedtls
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
