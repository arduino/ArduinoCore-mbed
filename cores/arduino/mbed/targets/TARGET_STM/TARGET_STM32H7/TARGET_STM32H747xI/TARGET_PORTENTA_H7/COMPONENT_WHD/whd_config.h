#ifndef __WHD_CONFIG__
#define __WHD_CONFIG__

#include "cyhal_gpio.h"
#include "whd_types.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>

#define WHD_COUNTRY     WHD_COUNTRY_AUSTRALIA

/* Internal thread config    */
#define WHD_THREAD_STACK_SIZE   5120
#define WHD_THREAD_PRIORITY     osPriorityHigh

/* please define your configuration , either SDIO or SPI */
#define CY_WHD_CONFIG_USE_SDIO
//#define CY_WHD_CONFIG_USE_SPI

/* Set Timeout for your platform */
#define WLAN_POWER_UP_DELAY_MS  		250
#define SDIO_ENUMERATION_TIMEOUT_MS     500

#define USES_RESOURCE_GENERIC_FILESYSTEM

#define BSP_LED1   	{GPIOK,{.Pin= GPIO_PIN_5 , .Mode = GPIO_MODE_OUTPUT_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
#define BSP_LED2		{GPIOK,{.Pin= GPIO_PIN_6 , .Mode = GPIO_MODE_OUTPUT_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
#define BSP_LED3		{GPIOK,{.Pin= GPIO_PIN_7 , .Mode = GPIO_MODE_OUTPUT_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}

/* power pin */
#define WIFI_WL_REG_ON  	{GPIOJ,{.Pin= GPIO_PIN_1, .Mode = GPIO_MODE_OUTPUT_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW}}
//#define WIFI_32K_CLK    	{GPIOA,{.Pin= GPIO_PIN_8, .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_LOW , .Alternate = GPIO_AF0_MCO}}

#define WIFI_SDIO_CMD		{GPIOD,{.Pin= GPIO_PIN_2 , .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_CLK  		{GPIOC,{.Pin= GPIO_PIN_12, .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_D0		{GPIOC,{.Pin= GPIO_PIN_8 , .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_D1		{GPIOC,{.Pin= GPIO_PIN_9 , .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_D2		{GPIOC,{.Pin= GPIO_PIN_10, .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_D3 		{GPIOC,{.Pin= GPIO_PIN_11, .Mode = GPIO_MODE_AF_PP , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH, .Alternate = GPIO_AF12_SDIO1}}
#define WIFI_SDIO_OOB_IRQ 	{GPIOJ,{.Pin= GPIO_PIN_5,  .Mode = GPIO_MODE_IT_FALLING , .Pull = GPIO_NOPULL , .Speed= GPIO_SPEED_FREQ_VERY_HIGH}}

#ifndef CYHAL_NC_PIN_VALUE
#define CYHAL_NC_PIN_VALUE ( (cyhal_gpio_t)0xFFFFFFFF )
#endif

#endif
