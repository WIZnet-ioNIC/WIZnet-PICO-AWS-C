# How to Test HTTP & HTTPS Example



## Step 1: Prepare software

The following serial terminal program is required for HTTP & HTTPS example test, download and install from below link.

- [**Tera Term**][link-tera_term]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 


## Step 3: Setup HTTP & HTTPS Example

To test the HTTP & HTTPS example, minor settings shall be done in code.

1. Setup SPI port and pin in 'wizchip_spi.h' in 'WIZnet-PICO-AWS-C/port/ioLibrary_Driver' directory.

Setup the SPI interface you use.

### For **W55RP20-EVB-PICO**:
If you are using the **W55RP20-EVB-PICO**, enable `USE_PIO` and configure as follows:

```cpp
#if (DEVICE_BOARD_NAME == W55RP20_EVB_PICO)

#define USE_PIO

#define PIN_SCK   21
#define PIN_MOSI  23
#define PIN_MISO  22
#define PIN_CS    20
#define PIN_RST   25
#define PIN_IRQ   24

```

---

### For **W6300-EVB-PICO** or **W6300-EVB-PICO2**:
If you are using the **W6300-EVB-PICO** or **W6300-EVB-PICO2**, use the following pinout and SPI clock divider configuration:

```cpp
#elif (DEVICE_BOARD_NAME == W6300_EVB_PICO || DEVICE_BOARD_NAME == W6300_EVB_PICO2)
#define USE_PIO

#define PIO_IRQ_PIN             15
#define PIO_SPI_SCK_PIN         17
#define PIO_SPI_DATA_IO0_PIN    18
#define PIO_SPI_DATA_IO1_PIN    19
#define PIO_SPI_DATA_IO2_PIN    20
#define PIO_SPI_DATA_IO3_PIN    21
#define PIN_CS                  16
#define PIN_RST                 22


```

---

### For other generic SPI boards
If you are not using any of the above boards, you can fall back to a default SPI configuration:

```cpp
#else

#define SPI_PORT spi0

#define SPI_SCK_PIN  18
#define SPI_MOSI_PIN 19
#define SPI_MISO_PIN 16
#define SPI_CS_PIN   17
#define RST_PIN      20

#endif
```

Make sure you are **not defining `USE_PIO`** in your setup when using DMA:

```cpp
// #define USE_PIO
```



2. Setup network configuration such as IP in 'aws_iot_http.c' , which is the HTTP & HTTPS example in 'WIZnet-PICO-AWS-C/examples/aws_iot_http/' directory.

Setup IP and other network settings to suit your network environment.

```cpp
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
#if _WIZCHIP_ > W5500
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
#else
        .dhcp = NETINFO_STATIC        
#endif
};
```

3. Setup URL you want to connect to.

```cpp
/* HTTP */
#define HTTP_GET_URL "URL"
```



## Step 4: Build

1. After completing the HTTP & HTTPS example configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'aws_iot_http.uf2' is generated in 'WIZnet-PICO-AWS-C/build/examples/aws_iot_http/'.



## Step 5: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'aws_iot_http.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the HTTP & HTTPS example works normally, You can see the IP got from the URL and the cipher suite applied when connecting.

![][link-dns_server_ip_and_ciphersuite]

6. Also, you can see HTTP body information.

![][link-http_body_information]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_http/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_http/connect_to_serial_com_port.png
[link-dns_server_ip_and_ciphersuite]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_http/dns_server_ip_and_ciphersuite.png
[link-http_body_information]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_http/http_body_information.png
