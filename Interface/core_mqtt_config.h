/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _CORE_MQTT_CONFIG_H_
#define _CORE_MQTT_CONFIG_H_

/*
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */

/*
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Buffer */
#define MQTT_BUF_MAX_SIZE (1024 * 2)
#define MQTT_DOMAIN_MAX_SIZE 128

/* Timeout */
#define MQTT_TIMEOUT (1000 * 10)         // 10 seconds
#define MQTT_DEFAULT_YIELD_TIMEOUT 10    // 10 milliseconds
#define MQTT_DEFAULT_KEEP_ALIVE (60 * 3) // unit : second

/* Subscription number */
#define MQTT_SUBSCRIPTION_MAX_NUM 3

#if 0
#define LogError(message) printf message
#define LogWarn(message) printf message
#define LogInfo(message) printf message
#define LogDebug(message) printf message
#endif

/*
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/*
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

#endif /* _CORE_MQTT_CONFIG_H_ */
