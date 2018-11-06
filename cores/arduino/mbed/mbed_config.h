/*
 * mbed SDK
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Automatically generated configuration file.
// DO NOT EDIT, content will be overwritten.

#ifndef __MBED_CONFIG_DATA__
#define __MBED_CONFIG_DATA__

// Configuration parameters
#define CLOCK_SOURCE                                                          USE_PLL_HSE_XTAL|USE_PLL_HSI                                                                     // set by target:DISCO_F429ZI
#define CLOCK_SOURCE_USB                                                      1                                                                                                // set by target:DISCO_F429ZI
#define LPTICKER_DELAY_TICKS                                                  1                                                                                                // set by target:FAMILY_STM32
#define MBED_CONF_ATMEL_RF_ASSUME_SPACED_SPI                                  1                                                                                                // set by library:atmel-rf[STM]
#define MBED_CONF_ATMEL_RF_FULL_SPI_SPEED                                     7500000                                                                                          // set by library:atmel-rf
#define MBED_CONF_ATMEL_RF_FULL_SPI_SPEED_BYTE_SPACING                        250                                                                                              // set by library:atmel-rf
#define MBED_CONF_ATMEL_RF_LOW_SPI_SPEED                                      3750000                                                                                          // set by library:atmel-rf
#define MBED_CONF_ATMEL_RF_PROVIDE_DEFAULT                                    0                                                                                                // set by library:atmel-rf
#define MBED_CONF_ATMEL_RF_USE_SPI_SPACING_API                                0                                                                                                // set by library:atmel-rf
#define MBED_CONF_CELLULAR_DEBUG_AT                                           0                                                                                                // set by library:cellular
#define MBED_CONF_CELLULAR_RANDOM_MAX_START_DELAY                             0                                                                                                // set by library:cellular
#define MBED_CONF_CELLULAR_USE_APN_LOOKUP                                     1                                                                                                // set by library:cellular
#define MBED_CONF_DRIVERS_UART_SERIAL_RXBUF_SIZE                              256                                                                                              // set by library:drivers
#define MBED_CONF_DRIVERS_UART_SERIAL_TXBUF_SIZE                              256                                                                                              // set by library:drivers
#define MBED_CONF_ESP8266_DEBUG                                               0                                                                                                // set by library:esp8266
#define MBED_CONF_ESP8266_PROVIDE_DEFAULT                                     0                                                                                                // set by library:esp8266
#define MBED_CONF_EVENTS_PRESENT                                              1                                                                                                // set by library:events
#define MBED_CONF_EVENTS_SHARED_DISPATCH_FROM_APPLICATION                     0                                                                                                // set by library:events
#define MBED_CONF_EVENTS_SHARED_EVENTSIZE                                     256                                                                                              // set by library:events
#define MBED_CONF_EVENTS_SHARED_HIGHPRIO_EVENTSIZE                            256                                                                                              // set by library:events
#define MBED_CONF_EVENTS_SHARED_HIGHPRIO_STACKSIZE                            1024                                                                                             // set by library:events
#define MBED_CONF_EVENTS_SHARED_STACKSIZE                                     1024                                                                                             // set by library:events
#define MBED_CONF_EVENTS_USE_LOWPOWER_TIMER_TICKER                            0                                                                                                // set by library:events
#define MBED_CONF_FILESYSTEM_PRESENT                                          1                                                                                                // set by library:filesystem
#define MBED_CONF_LORA_ADR_ON                                                 1                                                                                                // set by library:lora
#define MBED_CONF_LORA_APPLICATION_EUI                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                                                 // set by library:lora
#define MBED_CONF_LORA_APPLICATION_KEY                                        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // set by library:lora
#define MBED_CONF_LORA_APPSKEY                                                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // set by library:lora
#define MBED_CONF_LORA_APP_PORT                                               15                                                                                               // set by library:lora
#define MBED_CONF_LORA_AUTOMATIC_UPLINK_MESSAGE                               1                                                                                                // set by library:lora
#define MBED_CONF_LORA_DEVICE_ADDRESS                                         0x00000000                                                                                       // set by library:lora
#define MBED_CONF_LORA_DEVICE_EUI                                             {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                                                 // set by library:lora
#define MBED_CONF_LORA_DOWNLINK_PREAMBLE_LENGTH                               5                                                                                                // set by library:lora
#define MBED_CONF_LORA_DUTY_CYCLE_ON                                          1                                                                                                // set by library:lora
#define MBED_CONF_LORA_DUTY_CYCLE_ON_JOIN                                     1                                                                                                // set by library:lora
#define MBED_CONF_LORA_FSB_MASK                                               {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00FF}                                                         // set by library:lora
#define MBED_CONF_LORA_FSB_MASK_CHINA                                         {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF}                                                 // set by library:lora
#define MBED_CONF_LORA_LBT_ON                                                 0                                                                                                // set by library:lora
#define MBED_CONF_LORA_MAX_SYS_RX_ERROR                                       10                                                                                               // set by library:lora
#define MBED_CONF_LORA_NB_TRIALS                                              12                                                                                               // set by library:lora
#define MBED_CONF_LORA_NWKSKEY                                                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // set by library:lora
#define MBED_CONF_LORA_OVER_THE_AIR_ACTIVATION                                1                                                                                                // set by library:lora
#define MBED_CONF_LORA_PHY                                                    EU868                                                                                            // set by library:lora
#define MBED_CONF_LORA_PUBLIC_NETWORK                                         1                                                                                                // set by library:lora
#define MBED_CONF_LORA_TX_MAX_SIZE                                            64                                                                                               // set by library:lora
#define MBED_CONF_LORA_UPLINK_PREAMBLE_LENGTH                                 8                                                                                                // set by library:lora
#define MBED_CONF_LWIP_ADDR_TIMEOUT                                           5                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_ADDR_TIMEOUT_MODE                                      1                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_DEBUG_ENABLED                                          0                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_DEFAULT_THREAD_STACKSIZE                               512                                                                                              // set by library:lwip
#define MBED_CONF_LWIP_ENABLE_PPP_TRACE                                       0                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_ETHERNET_ENABLED                                       1                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_IPV4_ENABLED                                           1                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_IPV6_ENABLED                                           0                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_IP_VER_PREF                                            4                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_MEM_SIZE                                               25600                                                                                            // set by library:lwip[STM]
#define MBED_CONF_LWIP_PPP_THREAD_STACKSIZE                                   768                                                                                              // set by library:lwip
#define MBED_CONF_LWIP_SOCKET_MAX                                             4                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_TCPIP_THREAD_STACKSIZE                                 1200                                                                                             // set by library:lwip
#define MBED_CONF_LWIP_TCP_ENABLED                                            1                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_TCP_SERVER_MAX                                         4                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_TCP_SOCKET_MAX                                         4                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_UDP_SOCKET_MAX                                         4                                                                                                // set by library:lwip
#define MBED_CONF_LWIP_USE_MBED_TRACE                                         0                                                                                                // set by library:lwip
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_CHANNEL                            0                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_CHANNEL_MASK                       0x7fff800                                                                                        // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_CHANNEL_PAGE                       0                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_DEVICE_TYPE                        NET_6LOWPAN_ROUTER                                                                               // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_PANID_FILTER                       0xffff                                                                                           // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_PSK_KEY                            {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf} // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_PSK_KEY_ID                         1                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_SECURITY_MODE                      NONE                                                                                             // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_6LOWPAN_ND_SEC_LEVEL                          5                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_HEAP_SIZE                                     32500                                                                                            // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_HEAP_STAT_INFO                                NULL                                                                                             // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_CHANNEL                         22                                                                                               // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_CHANNEL_MASK                    0x7fff800                                                                                        // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_CHANNEL_PAGE                    0                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_COMMISSIONING_DATASET_TIMESTAMP 0x10000                                                                                          // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_EXTENDED_PANID                  {0xf1, 0xb5, 0xa1, 0xb2,0xc4, 0xd5, 0xa1, 0xbd }                                                 // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_ML_PREFIX                       {0xfd, 0x0, 0x0d, 0xb8, 0x0, 0x0, 0x0, 0x0}                                                      // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_NETWORK_NAME                    "Thread Network"                                                                                 // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PANID                           0x0700                                                                                           // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_CONFIG_PSKC                            {0xc8, 0xa6, 0x2e, 0xae, 0xf3, 0x68, 0xf3, 0x46, 0xa9, 0x9e, 0x57, 0x85, 0x98, 0x9d, 0x1c, 0xd0} // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_DEVICE_TYPE                            MESH_DEVICE_TYPE_THREAD_ROUTER                                                                   // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_MASTER_KEY                             {0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff} // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_PSKD                                   "ABCDEFGH"                                                                                       // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_SECURITY_POLICY                        255                                                                                              // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_THREAD_USE_STATIC_LINK_CONFIG                 1                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MBED_MESH_API_USE_MALLOC_FOR_HEAP                           0                                                                                                // set by library:mbed-mesh-api
#define MBED_CONF_MCR20A_PROVIDE_DEFAULT                                      0                                                                                                // set by library:mcr20a
#define MBED_CONF_NANOSTACK_CONFIGURATION                                     nanostack_full                                                                                   // set by library:nanostack
#define MBED_CONF_NANOSTACK_HAL_CRITICAL_SECTION_USABLE_FROM_INTERRUPT        0                                                                                                // set by library:nanostack-hal
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_DISPATCH_FROM_APPLICATION          0                                                                                                // set by library:nanostack-hal
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_THREAD_STACK_SIZE                  6144                                                                                             // set by library:nanostack-hal
#define MBED_CONF_NANOSTACK_HAL_EVENT_LOOP_USE_MBED_EVENTS                    0                                                                                                // set by library:nanostack-hal
#define MBED_CONF_NANOSTACK_HAL_NVM_CFSTORE                                   0                                                                                                // set by library:nanostack-hal
#define MBED_CONF_NSAPI_DEFAULT_MESH_TYPE                                     THREAD                                                                                           // set by library:nsapi
#define MBED_CONF_NSAPI_DEFAULT_STACK                                         LWIP                                                                                             // set by library:nsapi
#define MBED_CONF_NSAPI_DEFAULT_WIFI_SECURITY                                 NONE                                                                                             // set by library:nsapi
#define MBED_CONF_NSAPI_DNS_CACHE_SIZE                                        3                                                                                                // set by library:nsapi
#define MBED_CONF_NSAPI_DNS_RESPONSE_WAIT_TIME                                5000                                                                                             // set by library:nsapi
#define MBED_CONF_NSAPI_DNS_RETRIES                                           0                                                                                                // set by library:nsapi
#define MBED_CONF_NSAPI_DNS_TOTAL_ATTEMPTS                                    3                                                                                                // set by library:nsapi
#define MBED_CONF_NSAPI_PRESENT                                               1                                                                                                // set by library:nsapi
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE                           9600                                                                                             // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_ALL_THREADS_INFO                             0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_DECODE_HTTP_URL_STR                          "\nFor more info, visit: https://armmbed.github.io/mbedos-error/?error=0x%08X"                   // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_FILENAME_CAPTURE_ENABLED                     0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_HIST_ENABLED                                 0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_ERROR_HIST_SIZE                                    4                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_FORCE_NON_COPYABLE_ERROR                           0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_MAX_ERROR_FILENAME_LEN                             16                                                                                               // set by library:platform
#define MBED_CONF_PLATFORM_POLL_USE_LOWPOWER_TIMER                            0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE                                    9600                                                                                             // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_BUFFERED_SERIAL                              0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES                             0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_TTY_NEWLINES                         0                                                                                                // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT                                1                                                                                                // set by library:platform
#define MBED_CONF_PPP_CELL_IFACE_APN_LOOKUP                                   1                                                                                                // set by library:ppp-cell-iface
#define MBED_CONF_PPP_CELL_IFACE_AT_PARSER_BUFFER_SIZE                        256                                                                                              // set by library:ppp-cell-iface
#define MBED_CONF_PPP_CELL_IFACE_AT_PARSER_TIMEOUT                            8000                                                                                             // set by library:ppp-cell-iface
#define MBED_CONF_PPP_CELL_IFACE_BAUD_RATE                                    115200                                                                                           // set by library:ppp-cell-iface
#define MBED_CONF_RTOS_IDLE_THREAD_STACK_SIZE                                 512                                                                                              // set by library:rtos
#define MBED_CONF_RTOS_MAIN_THREAD_STACK_SIZE                                 4096                                                                                             // set by library:rtos
#define MBED_CONF_RTOS_PRESENT                                                1                                                                                                // set by library:rtos
#define MBED_CONF_RTOS_THREAD_STACK_SIZE                                      4096                                                                                             // set by library:rtos
#define MBED_CONF_RTOS_TIMER_THREAD_STACK_SIZE                                768                                                                                              // set by library:rtos
#define MBED_CONF_TARGET_LPUART_CLOCK_SOURCE                                  USE_LPUART_CLK_LSE|USE_LPUART_CLK_PCLK1                                                          // set by target:FAMILY_STM32
#define MBED_CONF_TARGET_LSE_AVAILABLE                                        0                                                                                                // set by target:DISCO_F429ZI
#define MBED_LFS_BLOCK_SIZE                                                   512                                                                                              // set by library:littlefs
#define MBED_LFS_ENABLE_INFO                                                  0                                                                                                // set by library:littlefs
#define MBED_LFS_INTRINSICS                                                   1                                                                                                // set by library:littlefs
#define MBED_LFS_LOOKAHEAD                                                    512                                                                                              // set by library:littlefs
#define MBED_LFS_PROG_SIZE                                                    64                                                                                               // set by library:littlefs
#define MBED_LFS_READ_SIZE                                                    64                                                                                               // set by library:littlefs
#define MBED_MEM_TRACING_ENABLED                                              0                                                                                                // set by library:platform
#define MEM_ALLOC                                                             malloc                                                                                           // set by library:mbed-trace
#define MEM_FREE                                                              free                                                                                             // set by library:mbed-trace
#define NSAPI_PPP_AVAILABLE                                                   0                                                                                                // set by library:lwip
#define NSAPI_PPP_IPV4_AVAILABLE                                              1                                                                                                // set by library:lwip
#define NSAPI_PPP_IPV6_AVAILABLE                                              0                                                                                                // set by library:lwip
#define NVSTORE_ENABLED                                                       1                                                                                                // set by library:nvstore
#define NVSTORE_MAX_KEYS                                                      16                                                                                               // set by library:nvstore
// Macros
#define NS_USE_EXTERNAL_MBED_TLS                                                                                                                                               // defined by library:nanostack
#define UNITY_INCLUDE_CONFIG_H                                                                                                                                                 // defined by library:utest
#define _RTE_                                                                                                                                                                  // defined by library:rtos

#endif
