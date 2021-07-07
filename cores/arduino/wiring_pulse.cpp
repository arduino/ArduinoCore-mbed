#include "Arduino.h"

#if defined(ARDUINO_ARCH_NRF52840) || defined(TARGET_NICLA)

#include "mbed.h"
#include <hal/nrf_timer.h>
#include <hal/nrf_gpiote.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_ppi.h>
#include "nrfx_gpiote.h"
#include "nrfx_ppi.h"

/* Hot encoded peripherals. Could be chosen with a more clever strategy */
#define PULSE_TIMER (NRF_TIMER2)
#define TIMER_FIRST_CHANNEL (NRF_TIMER_CC_CHANNEL1)
#define TIMER_SECOND_CHANNEL (NRF_TIMER_CC_CHANNEL2)
#define TIMER_FIRST_CAPTURE (NRF_TIMER_TASK_CAPTURE1)
#define TIMER_SECOND_CAPTURE (NRF_TIMER_TASK_CAPTURE2)

#define TIMEOUT_US (0)

/* GPIOTE configuration for pin PPI event */
static nrfx_gpiote_in_config_t cfg = 
    {
        .sense = NRF_GPIOTE_POLARITY_TOGGLE,        
        .pull = NRF_GPIO_PIN_NOPULL,                
        .is_watcher = false,                       
        .hi_accuracy = true,                     
        .skip_gpio_setup = true // skip pin setup, the pin is assumed to be already configured 
    };

/* 
 * This function enables the pin edge detection hardware and tries to understand the state of the pin at the time of such activation
 * If the hardware detection event is enabled on an edge of the pin, it's not possible to understand if that edge would be detected or not
 * In such case, TIMEOUT_US constant is returned to indicate this extreme condition 
 * Else, if the function is able to understand the state of the pin at the time of activation, it returns the index of the desired pulse 
 */
static uint8_t measurePulse(PinName pin, PinStatus state, nrf_ppi_channel_group_t firstGroup) 
{
    /* three different reads are needed, because it's not easy to synchronize hardware and software */
    uint32_t firstState, secondState, thirdState; 
    core_util_critical_section_enter();
    firstState = nrf_gpio_pin_read(pin);
    /* Enable the hardware detection of the pin edge */
    nrf_ppi_group_enable(firstGroup);
    secondState = nrf_gpio_pin_read(pin);
    __NOP();
    thirdState = nrf_gpio_pin_read(pin);
    core_util_critical_section_exit();
    uint8_t pulseToTake = 0;
    /* If no changes on the pin were detected, there are no doubts */
    if (firstState == secondState && firstState == thirdState) {
        if (firstState != state) {
            pulseToTake = 1;
        } else {
            pulseToTake = 2;
        }
    } else {
        pulseToTake = TIMEOUT_US;
    }
    return pulseToTake;
}

/*
 * The pulse pin is assumed to be already configured as an input pin and NOT as an interrupt pin
 * Also the serial_api.c from NRF sdk uses ppi channels, but there shouldn't be any issue 
 * The disadvantage of this approach is that some pulses could be missed, and more time could be necessary for retrying the measure. 
 * Using two different events for rising and falling edge would be way better
 */
unsigned long pulseIn(PinName pin, PinStatus state, unsigned long timeout)
{
    /* Configure timer */
    nrf_timer_mode_set(PULSE_TIMER, NRF_TIMER_MODE_TIMER);
    nrf_timer_task_trigger(PULSE_TIMER, NRF_TIMER_TASK_STOP);
    nrf_timer_frequency_set(PULSE_TIMER, NRF_TIMER_FREQ_1MHz); 
    nrf_timer_bit_width_set(PULSE_TIMER, NRF_TIMER_BIT_WIDTH_32);
    nrf_timer_cc_write(PULSE_TIMER, TIMER_FIRST_CHANNEL, 0);
    nrf_timer_cc_write(PULSE_TIMER, TIMER_SECOND_CHANNEL, 0);
    nrf_timer_task_trigger(PULSE_TIMER, NRF_TIMER_TASK_CLEAR);
    /* Configure pin Toggle Event */
    nrfx_gpiote_in_init(pin, &cfg, NULL);
    nrfx_gpiote_in_event_enable(pin, true); 

    /* Allocate PPI channels for starting and stopping the timer */
    nrf_ppi_channel_t firstPPIchannel, firstPPIchannelControl;
    nrf_ppi_channel_t secondPPIchannel, secondPPIchannelControl;
    nrf_ppi_channel_t thirdPPIchannel, thirdPPIchannelControl;
    nrfx_ppi_channel_alloc(&firstPPIchannel);
    nrfx_ppi_channel_alloc(&firstPPIchannelControl);
    nrfx_ppi_channel_alloc(&secondPPIchannel);
    nrfx_ppi_channel_alloc(&secondPPIchannelControl);
    nrfx_ppi_channel_alloc(&thirdPPIchannel);
    nrfx_ppi_channel_alloc(&thirdPPIchannelControl);
    /* Allocate PPI Group channels to allow activation and deactivation of channels as PPI tasks */
    nrf_ppi_channel_group_t firstGroup, secondGroup, thirdGroup;
    nrfx_ppi_group_alloc(&firstGroup);
    nrfx_ppi_group_alloc(&secondGroup);
    nrfx_ppi_group_alloc(&thirdGroup);

    /* Insert channels in corresponding group */
    nrfx_ppi_channel_include_in_group(firstPPIchannel, firstGroup);
    nrfx_ppi_channel_include_in_group(firstPPIchannelControl, firstGroup);
    nrfx_ppi_channel_include_in_group(secondPPIchannel, secondGroup);
    nrfx_ppi_channel_include_in_group(secondPPIchannelControl, secondGroup);
    nrfx_ppi_channel_include_in_group(thirdPPIchannel, thirdGroup);
    nrfx_ppi_channel_include_in_group(thirdPPIchannelControl, thirdGroup);

    /* Configure PPI channels for Start and Stop events */
    /* The first edge on the pin will trigger the timer START task */
    nrf_ppi_channel_endpoint_setup(firstPPIchannel,
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrf_timer_task_address_get(PULSE_TIMER, NRF_TIMER_TASK_START));
    nrf_ppi_channel_and_fork_endpoint_setup(firstPPIchannelControl,
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrfx_ppi_task_addr_group_enable_get(secondGroup),
                                    (uint32_t) nrfx_ppi_task_addr_group_disable_get(firstGroup));
    /* The second edge will result in a capture of the timer counter into a register. In this way the first impulse is captured */
    nrf_ppi_channel_endpoint_setup(secondPPIchannel, 
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrf_timer_task_address_get(PULSE_TIMER, TIMER_FIRST_CAPTURE));
    nrf_ppi_channel_and_fork_endpoint_setup(secondPPIchannelControl,
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrfx_ppi_task_addr_group_enable_get(thirdGroup),
                                    (uint32_t) nrfx_ppi_task_addr_group_disable_get(secondGroup));
    /* The third edge will capture the second impulse. After that, the pulse corresponding to the correct state must be returned */
    nrf_ppi_channel_and_fork_endpoint_setup(thirdPPIchannel,
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrf_timer_task_address_get(PULSE_TIMER, NRF_TIMER_TASK_STOP),
                                    (uint32_t) nrf_timer_task_address_get(PULSE_TIMER, TIMER_SECOND_CAPTURE));
    nrf_ppi_channel_endpoint_setup(thirdPPIchannelControl, 
                                    (uint32_t) nrfx_gpiote_in_event_addr_get(pin),
                                    (uint32_t) nrfx_ppi_task_addr_group_disable_get(thirdGroup));

    uint8_t pulseToTake = TIMEOUT_US;
    auto startMicros = micros();

    pulseToTake = measurePulse(pin, state, firstGroup);
    while (pulseToTake == TIMEOUT_US && (micros() - startMicros < timeout)) {
        /* In case it wasn't possible to detect the initial state, disable the hardware detection control and retry */
        nrf_ppi_group_disable(firstGroup);
        nrf_ppi_group_disable(secondGroup);
        nrf_ppi_group_disable(thirdGroup);
        /* Stop the timer and clear its registers */
        nrf_timer_task_trigger(PULSE_TIMER, NRF_TIMER_TASK_STOP);
        nrf_timer_task_trigger(PULSE_TIMER, NRF_TIMER_TASK_CLEAR);
        nrf_timer_cc_write(PULSE_TIMER, TIMER_FIRST_CHANNEL, 0);
        nrf_timer_cc_write(PULSE_TIMER, TIMER_SECOND_CHANNEL, 0);
        /* Retry to enable the detection hardware figuring out the starting state of the pin */
        pulseToTake = measurePulse(pin, state, firstGroup);
    }

    unsigned long pulseTime = TIMEOUT_US;
    unsigned long pulseFirst = TIMEOUT_US;
    unsigned long pulseSecond = TIMEOUT_US;

    /* Optionally the time reference could be restarted because here the actual wait for the pulse begins */
    //startMicros = micros();

    if (pulseToTake >= 1) {
        while (!pulseFirst && (micros() - startMicros < timeout) ) {
            pulseFirst = nrf_timer_cc_read(PULSE_TIMER, TIMER_FIRST_CHANNEL);
        }
        pulseTime = pulseFirst;
    }

    if (pulseToTake == 2) {
        while (!pulseSecond && (micros() - startMicros < timeout) ) {
            pulseSecond = nrf_timer_cc_read(PULSE_TIMER, TIMER_SECOND_CHANNEL);
        }
        pulseTime = pulseSecond ? pulseSecond - pulseFirst : TIMEOUT_US;
    }
    
    /* Deallocate all the PPI channels, events and groups */    
    nrf_timer_task_trigger(PULSE_TIMER, NRF_TIMER_TASK_SHUTDOWN);
    nrfx_gpiote_in_uninit(pin);
    nrfx_ppi_group_free(firstGroup);
    nrfx_ppi_group_free(secondGroup);
    nrfx_ppi_group_free(thirdGroup);
    nrf_ppi_channel_group_clear(firstGroup);
    nrf_ppi_channel_group_clear(secondGroup);
    nrf_ppi_channel_group_clear(thirdGroup);
    nrfx_ppi_channel_free(firstPPIchannel);
    nrfx_ppi_channel_free(firstPPIchannelControl);
    nrfx_ppi_channel_free(secondPPIchannel);
    nrfx_ppi_channel_free(secondPPIchannelControl);
    nrfx_ppi_channel_free(thirdPPIchannel);
    nrfx_ppi_channel_free(thirdPPIchannelControl);

    /* The timer has a frequency of 1 MHz, so its counting value is already in microseconds */
    return pulseTime; 
}


#elif defined(TARGET_RP2040)

#include "pinDefinitions.h"

unsigned long pulseIn(PinName pin, PinStatus state, unsigned long timeout)
{
    unsigned long startMicros = micros();

    // wait for any previous pulse to end
    while (gpio_get(pin) == state) {
        tight_loop_contents();
        if (micros() - startMicros > timeout)
            return 0;
    }

    // wait for the pulse to start
    while (gpio_get(pin) != state) {
        tight_loop_contents();
        if (micros() - startMicros > timeout)
            return 0;
    }

    unsigned long start = micros();
    // wait for the pulse to stop
    while (gpio_get(pin) == state) {
        tight_loop_contents();
        if (micros() - startMicros > timeout)
            return 0;
    }
    return micros() - start;
}

#elif defined(TARGET_STM32H7)

extern "C" {
    #include "gpio_api.h"
    GPIO_TypeDef *Set_GPIO_Clock(uint32_t port_idx);
}

#include "pinDefinitions.h"

unsigned long pulseIn(PinName pin, PinStatus state, unsigned long timeout)
{

    uint32_t port_index = STM_PORT(pin);
    GPIO_TypeDef *gpio = Set_GPIO_Clock(port_index);

    volatile uint32_t *reg_in = &gpio->IDR;
    uint32_t mask = gpio_set(pin);

    unsigned long startMicros = micros();

    // wait for any previous pulse to end
    while ((*reg_in & mask) == state) {
        if (micros() - startMicros > timeout)
            return 0;
    }

    // wait for the pulse to start
    while ((*reg_in & mask) != state) {
        if (micros() - startMicros > timeout)
            return 0;
    }

    unsigned long start = micros();
    // wait for the pulse to stop
    while ((*reg_in & mask) == state) {
        if (micros() - startMicros > timeout)
            return 0;
    }
    return micros() - start;
}


#endif

// generic, overloaded implementations

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return pulseIn(digitalPinToPinName(pin), (PinStatus)state, timeout);
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return pulseIn(digitalPinToPinName(pin), (PinStatus)state, timeout);
}

unsigned long pulseInLong(PinName pin, PinStatus state, unsigned long timeout)
{
    return pulseIn(pin, state, timeout);
}