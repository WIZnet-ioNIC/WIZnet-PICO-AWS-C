# How to Test Connect to AWS IoT through MQTT Example



## Step 1: Prepare software

The following serial terminal program is required for Connect AWS IoT through MQTT example test, download and install from below link.

- [**Tera Term**][link-tera_term]



## Step 2: Prepare hardware

If you are using WIZnet's PICO board, you can skip '1. Combine...'

1. If you are using WIZnet Ethernet HAT, Combine it with Raspberry Pi Pico.

2. Connect ethernet cable to your PICO board ethernet port.

3. Connect your PICO board to desktop or laptop using USB cable. 


## Step 3: Setup AWS IoT

In order to connect to AWS IoT through MQTT, the development environment must be configured to use AWS IoT.

This example was tested by configuring AWS IoT Core. Please refer to the 'Create AWS IoT resources' section of document below and configure accordingly.

- [**Create AWS IoT resources**][link-create_aws_iot_resources]

For more information on AWS IoT, refer to the document below.

- [**What is AWS IoT?**][link-what_is_aws_iot]



## Step 4: Setup Connect AWS IoT through MQTT Example

To test the Connect AWS IoT through MQTT example, minor settings shall be done in code.

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



2. Setup network configuration such as IP in 'aws_iot_mqtt.c' which is the Connect AWS IoT through MQTT example in 'WIZnet-PICO-AWS-C/examples/aws_iot_mqtt/' directory.

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

3. Setup AWS IoT configuration.

MQTT_DOMAIN should be setup as AWS IoT data endpoint, and MQTT_USERNAME and MQTT_PASSWORD do not need to be setup. Setup MQTT_CLIENT_ID same as thing created during AWS IoT Core setup.

```cpp
/* AWS IoT */
#define MQTT_DOMAIN "account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com"
#define MQTT_PUB_TOPIC "$aws/things/my_rp2040_thing/shadow/update"
#define MQTT_SUB_TOPIC "$aws/things/my_rp2040_thing/shadow/update/accepted"
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL
#define MQTT_CLIENT_ID "my_rp2040_thing"
```

4. Setup device certificate and key.

You must enter the root certificate, client certificate and private key that were downloaded in Step 3.

Root certificate uses the RSA 2048 bit key, Amazon Root CA 1, and does not use the public key.

Device certificate and key can be set in 'mqtt_certificate.h' in 'WIZnet-PICO-AWS-C/examples/aws_iot_mqtt/' directory.

```cpp
uint8_t mqtt_root_ca[] =
"-----BEGIN CERTIFICATE-----\r\n"
"...\r\n"
"-----END CERTIFICATE-----\r\n";

uint8_t mqtt_client_cert[] =
"-----BEGIN CERTIFICATE-----\r\n"
"...\r\n"
"-----END CERTIFICATE-----\r\n";

uint8_t mqtt_private_key[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"...\r\n"
"-----END RSA PRIVATE KEY-----\r\n";
```



## Step 5: Build

1. After completing the configuration, click 'build' in the status bar at the bottom of Visual Studio Code or press the 'F7' button on the keyboard to build.

2. When the build is completed, 'aws_iot_mqtt.uf2' is generated in 'WIZnet-PICO-AWS-C/build/examples/aws_iot_mqtt/' directory.



## Step 6: Upload and Run

1. While pressing the BOOTSEL button of the Pico power on the board, the USB mass storage 'RPI-RP2' or 'RP2350' is automatically mounted.

![][link-raspberry_pi_pico_usb_mass_storage]

2. Drag and drop 'aws_iot_mqtt.uf2' onto the USB mass storage device 'RPI-RP2' or 'RP2350'.

3. Connect to the serial COM port of the pico with Tera Term.

![][link-connect_to_serial_com_port]

4. Reset your board.

5. If the Connect AWS IoT through MQTT example works normally on the pico, you can see the network information of the pico and connecting to the AWS IoT and publishing the message.

![][link-see_network_information_of_raspberry_pi_pico_connecting_to_aws_iot_and_publishing_message]

![][link-subscribe_to_publish_topic_and_receive_publish_message_through_test_function]

6. If you publish the message through the test function in AWS IoT Core to the subcribe topic was configured in Step 4, you can see that the pico receive the message about the subcribe topic.

![][link-publish_message_through_test_function]

![][link-receive_message_about_subcribe_topic]



<!--
Link
-->

[link-tera_term]: https://osdn.net/projects/ttssh2/releases/
[link-create_aws_iot_resources]: https://docs.aws.amazon.com/iot/latest/developerguide/create-iot-resources.html
[link-what_is_aws_iot]: https://docs.aws.amazon.com/iot/latest/developerguide/what-is-aws-iot.html
[link-raspberry_pi_pico_usb_mass_storage]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/raspberry_pi_pico_usb_mass_storage.png
[link-connect_to_serial_com_port]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/connect_to_serial_com_port.png
[link-see_network_information_of_raspberry_pi_pico_connecting_to_aws_iot_and_publishing_message]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/see_network_information_of_raspberry_pi_pico_connecting_to_aws_iot_and_publishing_message.png
[link-subscribe_to_publish_topic_and_receive_publish_message_through_test_function]: https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/subscribe_to_publish_topic_and_receive_publish_message_through_test_function.png
[link-publish_message_through_test_function]:https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/publish_message_through_test_function.png
[link-receive_message_about_subcribe_topic]:https://github.com/WIZnet-ioNIC/WIZnet-PICO-AWS-C/blob/main/static/images/aws_iot_mqtt/receive_message_about_subscribe_topic.png
