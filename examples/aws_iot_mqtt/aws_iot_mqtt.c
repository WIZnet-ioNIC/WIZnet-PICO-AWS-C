/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "dhcp.h"
#include "sntp.h"

#if HAS_RP2040_RTC
#include "hardware/rtc.h"
#elif HAS_POWMAN_TIMER
#include "pico/aon_timer.h"
#endif

#include "mbedtls/platform.h"
#include "timer.h"

#include "mqtt_transport_interface.h"
#include "ssl_transport_interface.h"
#include "timer_interface.h"

#include "mqtt_certificate.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 16)

/* Socket */
#define SOCKET_MQTT 0
#define SOCKET_DHCP 1
#define SOCKET_SNTP 2
// socket number 3 is used in dns interface

/* Port */
#define TARGET_PORT 8883

/* MQTT */
#define MQTT_PUB_PERIOD (1000 * 10) // 10 seconds
#define QOS 1
#define SSL_FLAG 1 // use SSL

/* Timezone */
#define TIMEZONE 40 // Korea

#if HAS_RP2040_RTC
// /* RTC */
// datetime_t rtc_time = {
//             .year  = 2020,
//             .month = 06,
//             .day   = 05,
//             .dotw  = 5, // 0 is Sunday, so 5 is Friday
//             .hour  = 15,
//             .min   = 45,
//             .sec   = 00
//     };
#endif

/* AWS IoT */
#define MQTT_DOMAIN "account-specific-prefix-ats.iot.ap-northeast-2.amazonaws.com"
#define MQTT_PUB_TOPIC "$aws/things/my_rp2040_thing/shadow/update"
#define MQTT_SUB_TOPIC "$aws/things/my_rp2040_thing/shadow/update/accepted"
#define MQTT_USERNAME NULL
#define MQTT_PASSWORD NULL
#define MQTT_CLIENT_ID "my_rp2040_thing"

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_DHCP                         // DHCP
};
static uint8_t g_ethernet_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
}; // common buffer

/* SNTP */
static uint8_t g_sntp_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_sntp_server_ip[4] = {216, 239, 35, 0}; // time.google.com

/* MQTT */
static uint8_t g_mqtt_buf[MQTT_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_mqtt_pub_msg_buf[MQTT_BUF_MAX_SIZE] = {
    0,
};

/* SSL */
tlsContext_t g_mqtt_tls_context;

/* Backoff */
BackoffAlgorithmContext_t backoff_context;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/* DHCP */
static void wizchip_dhcp_init(void);
static void wizchip_dhcp_assign(void);
static void wizchip_dhcp_conflict(void);

/* TIME */
static void sntp_set_time(datetime *sntp_time);
static void set_system_time(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int retval = 0;
    uint32_t tick_start = 0;
    uint32_t tick_end = 0;
    uint32_t pub_cnt = 0;
    uint32_t start_ms = 0;
    datetime sntp_time;

    set_clock_khz();

    stdio_init_all();

    wizchip_delay_ms(1000 * 3); // wait for 3 seconds

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

#if HAS_RP2040_RTC
    rtc_init();
    
#elif HAS_POWMAN_TIMER
    aon_timer_start_with_timeofday();
#endif

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    if (g_net_info.dhcp == NETINFO_DHCP) // DHCP
    {
        wizchip_dhcp_init();

        while (1)
        {
            retval = DHCP_run();

            if (retval == DHCP_IP_LEASED)
            {
                break;
            }

            wizchip_delay_ms(1000);
        }
    }
    else // static
    {
        network_initialize(g_net_info);
        print_network_information(g_net_info);
    }

    SNTP_init(SOCKET_SNTP, g_sntp_server_ip, TIMEZONE, g_sntp_buf);

    start_ms = millis();

    /* Get time */
    do
    {
        retval = SNTP_run(&sntp_time);

        if (retval == 1)
        {
            break;
        }
    } while ((millis() - start_ms) < RECV_TIMEOUT);

    if (retval != 1)
    {
        printf(" SNTP failed : %d\n", retval);

        while (1)
            ;
    }

    sntp_set_time(&sntp_time);
    
    set_system_time();

    /* Setup certificate */
    g_mqtt_tls_context.rootca_option = MBEDTLS_SSL_VERIFY_REQUIRED; // use Root CA verify
    g_mqtt_tls_context.clica_option = 1;                            // use client certificate
    g_mqtt_tls_context.root_ca = mqtt_root_ca;
    g_mqtt_tls_context.client_cert = mqtt_client_cert;
    g_mqtt_tls_context.private_key = mqtt_private_key;

    retval = mqtt_transport_init(true, MQTT_CLIENT_ID, NULL, NULL, MQTT_KEEP_ALIVE);

    if (retval != 0)
    {
        printf(" Failed, mqtt_transport_init returned %d\n", retval);

        while (1)
            ;
    }

    BackoffAlgorithm_InitializeParams(&backoff_context, BACKOFF_BASE, MAX_BACKOFF_DELAY, MAX_RECONNECT_ATTEMPTS);

    retval = mqtt_transport_connect(SOCKET_MQTT, SSL_FLAG, g_mqtt_buf, MQTT_BUF_MAX_SIZE, MQTT_DOMAIN, TARGET_PORT, &g_mqtt_tls_context, &backoff_context);
    if (retval != 0)
    {
        printf(" Failed, mqtt_transport_connect returned %d\n", retval);

        while (1)
            ;
    }
    retval = mqtt_transport_subscribe(QOS, MQTT_SUB_TOPIC);
    if (retval != 0)
    {
        printf(" Failed, mqtt_transport_subscribe returned %d\n", retval);

        while (1)
            ;
    }

    tick_start = millis();

    /* Infinite loop */
    while (1)
    {
        if (g_net_info.dhcp == NETINFO_DHCP)
        {
            DHCP_run();
        }

        retval = mqtt_transport_yield();

        if (retval != 0)
        {
            printf(" Failed, mqtt_transport_yield returned %d\n", retval);

            retval = mqtt_transport_connect(SOCKET_MQTT, SSL_FLAG, g_mqtt_buf, MQTT_BUF_MAX_SIZE, MQTT_DOMAIN, TARGET_PORT, &g_mqtt_tls_context, &backoff_context);

            if (retval != 0)
            {
                printf(" Failed, mqtt_transport_connect returned %d\n", retval);

                while (1)
                    ;
            }
            retval = mqtt_transport_subscribe(QOS, MQTT_SUB_TOPIC);

            if (retval != 0)
            {
                printf(" Failed, mqtt_transport_subscribe returned %d\n", retval);

                while (1)
                    ;
            }
            tick_start = millis();
            continue;
        }

        tick_end = millis();

        if (tick_end > tick_start + MQTT_PUB_PERIOD)
        {
            tick_start = millis();

            sprintf(g_mqtt_pub_msg_buf, "{\"message\":\"Hello, World!\", \"publish count\":\"%d\"}\n", pub_cnt++);
            mqtt_transport_publish(MQTT_PUB_TOPIC, g_mqtt_pub_msg_buf, strlen(g_mqtt_pub_msg_buf), QOS);
        }
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/* DHCP */
static void wizchip_dhcp_init(void)
{
    printf(" DHCP client running\n");

    DHCP_init(SOCKET_DHCP, g_ethernet_buf);

    reg_dhcp_cbfunc(wizchip_dhcp_assign, wizchip_dhcp_assign, wizchip_dhcp_conflict);
}

static void wizchip_dhcp_assign(void)
{
    getIPfromDHCP(g_net_info.ip);
    getGWfromDHCP(g_net_info.gw);
    getSNfromDHCP(g_net_info.sn);
    getDNSfromDHCP(g_net_info.dns);

    g_net_info.dhcp = NETINFO_DHCP;

    /* Network initialize */
    network_initialize(g_net_info); // apply from DHCP

    print_network_information(g_net_info);
    printf(" DHCP leased time : %ld seconds\n", getDHCPLeasetime());
}

static void wizchip_dhcp_conflict(void)
{
    printf(" Conflict IP from DHCP\n");

    // halt or reset or any...
    while (1)
        ; // this example is halt.
}

static void sntp_set_time(datetime *sntp_time)
{
    struct tm timeinfo;

    timeinfo.tm_year = sntp_time->yy - 1900;
    timeinfo.tm_mon = sntp_time->mo - 1;
    timeinfo.tm_mday = sntp_time->dd;
    timeinfo.tm_hour = sntp_time->hh;
    timeinfo.tm_min = sntp_time->mm;
    timeinfo.tm_sec = sntp_time->ss;

#if HAS_RP2040_RTC
    datetime_t rtc_time;

    rtc_time.year = sntp_time->yy;
    rtc_time.month = sntp_time->mo;
    rtc_time.day = sntp_time->dd;
    rtc_time.hour = sntp_time->hh;
    rtc_time.min = sntp_time->mm;
    rtc_time.sec = sntp_time->ss;
    rtc_time.dotw = timeinfo.tm_wday;

    rtc_set_datetime(&rtc_time);

    
#elif HAS_POWMAN_TIMER
    struct timespec ts;
    ts.tv_sec = mktime(&timeinfo);
    ts.tv_nsec = 0;
    aon_timer_set_time(&ts);
#endif
}

mbedtls_time_t sysyem_time_function(mbedtls_time_t *timer)
{
#if HAS_RP2040_RTC
    struct tm timeinfo;
    mbedtls_time_t t;
    datetime_t rtc_time;

    rtc_get_datetime(&rtc_time);

    timeinfo.tm_year = rtc_time.year - 1900;
    timeinfo.tm_mon = rtc_time.month - 1;
    timeinfo.tm_mday = rtc_time.day;
    timeinfo.tm_hour = rtc_time.hour;
    timeinfo.tm_min = rtc_time.min;
    timeinfo.tm_sec = rtc_time.sec;

    t = mktime(&timeinfo);

    if (timer)
    {
        *timer = t;
    }

    return t;
#elif HAS_POWMAN_TIMER

    struct timespec ts;

    aon_timer_get_time(&ts);

    if (timer)
    {
        *timer = ts.tv_sec;
    }

    return ts.tv_sec;

#endif
}
static void set_system_time()
{
    mbedtls_platform_set_time(sysyem_time_function);
}
