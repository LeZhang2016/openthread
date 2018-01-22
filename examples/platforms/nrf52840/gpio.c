/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for UART communication.
 *
 */
#include <openthread/config.h>
// NRF tools use #define PACKAGE - for other purposes
// ie: the physical package the chip comes in
// This conflicts with the GNU Autoconf "PACAKGE" define
#undef PACKAGE

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#include <common/logging.hpp>
#include <openthread/types.h>
#include <openthread/platform/toolchain.h>
#include <openthread/platform/gpio.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/logging.h>
#include <utils/code_utils.h>

#include <drivers/clock/nrf_drv_clock.h>
#include <hal/nrf_uart.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_gpiote.h>
#include "platform-nrf5.h"

#define IN_PIN 3

#define TE_IDX_TO_EVENT_ADDR(idx)    (nrf_gpiote_events_t)((uint32_t)NRF_GPIOTE_EVENTS_IN_0 + \
                                     (sizeof(uint32_t) * (idx)))

// void nrf_drv_gpiote_evt_handler_t(uint32_t pin, nrf_gpiote_polarity_t action);

static uint8_t sPinNumbers[30];
otInstance *sInstance;
// static uint32_t receiverTime = 0, senderTime = 0;
static int state = 0;

static uint32_t sPin = 3;

void nrf5GpioInit(void)
{
    for (uint32_t i = 0; i < 30; i++)
    {
        nrf_gpio_pin_clear(i);
    }
}

void otGpioInit(void)
{
    // for (uint32_t i = 0; i < sizeof(sPinNumbers)/sPinNumbers[0]; ++i)
    // {
    //     sPinNumbers[i] = 0;
    // }
    for (uint32_t i = 0; i < 30; i++)
    {
        nrf_gpio_pin_clear(i);
    }
}

void nrf5GpioProcess(otInstance *aInstance)
{
    (void) aInstance;
    // sInstance = aInstance;
    // for (uint32_t i = 0; i < 30; i++)
    // {
    if (nrf_gpio_pin_read(3) > 0)
    {
        if (state != 1)
        {
            otLogCritPlat(sInstance, " %d", state);
            state = 1;
            otPlatGpioSignalEvent(3);
        }
    }
    else
    {
        state = 0;
    }
    //     switch (sPinNumbers[i])
    //     {
    //     case 1:
    //         if (nrf_gpio_pin_read(i) > 0)
    //         {
    //             uint32_t receiverTimestamp = otPlatAlarmMicroGetNow();
    //             sPinNumbers[i] = 2;
    //             if (i == 3)
    //             {
    //                 receiverTime = receiverTimestamp;
    //                 otLogCritPlat(sInstance, " %d , %u", i, receiverTime);
    //             }
    //             else
    //             {
    //                 senderTime = receiverTimestamp;
    //                 otLogCritPlat(sInstance, " %d , %u", i, senderTime);
    //             }
    //         }
    //     case 2:
    //         if (nrf_gpio_pin_read(i) <= 0)
    //         {
    //             sPinNumbers[i] = 1;
    //         }
    //     }
    // }

}

void nrf5GpioDeinit(void)
{
    for (uint32_t i = 0; i < 30; i++)
    {
        nrf_gpio_pin_clear(i);
    }
}

void otPlatGpioCfgOutput(uint32_t aPin)
{
    nrf_gpio_cfg_output(aPin);
}

void otPlatGpioCfgInput(uint32_t aPin)
{
    // sPinNumbers[aPin] = 1;
    // otLogCritPlat(sInstance, "pin %d", aPin);
    nrf_gpio_cfg_input(aPin, NRF_GPIO_PIN_NOPULL);
}

void otPlatGpioWrite(uint32_t aPin, uint32_t aValue)
{
    nrf_gpio_pin_write(aPin, aValue);
}

void otPlatGpioClear(uint32_t aPin)
{
    // sPinNumbers[aPin] = 0;
    nrf_gpio_pin_clear(sPinNumbers[aPin]);
}

uint32_t otPlatGpioRead(uint32_t aPin)
{
    return nrf_gpio_pin_read(aPin);
}

void otPlatGpioSet(uint32_t aPin)
{
    nrf_gpio_pin_set(aPin);
}


void otPlatGpioToggle(uint32_t aPin)
{
    nrf_gpio_pin_toggle(aPin);
}


void otPlatGpioEnableInterrupt(uint32_t aPin)
{
    // init gpiote for event/interrupt
    nrf_drv_common_irq_enable(GPIOTE_IRQn, 7);//GPIOTE_CONFIG_IRQ_PRIORITY);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_PORT);
    nrf_gpiote_int_enable(GPIOTE_INTENSET_PORT_Msk);

    nrf_gpio_cfg_input(aPin, NRF_GPIO_PIN_PULLDOWN);
    nrf_gpiote_event_configure(GPIOTE_CHANNEL, aPin, GPIOTE_CONFIG_POLARITY_LoToHi);

    //nrf_drv_gpiote_in_event_enable(IN_PIN, true);
    nrf_gpiote_events_t event   = TE_IDX_TO_EVENT_ADDR(GPIOTE_CHANNEL);

    nrf_gpiote_event_enable(GPIOTE_CHANNEL);
    nrf_gpiote_event_clear(event);
    nrf_gpiote_int_enable(1 << GPIOTE_CHANNEL);
    sPin = aPin;
}

void otPlatGpioDisableInterrupt(uint32_t aPin)
{
    nrf_drv_common_irq_disable(GPIOTE_IRQn);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_PORT);
    nrf_gpiote_int_disable(GPIOTE_INTENSET_PORT_Msk);

    nrf_gpio_pin_clear(aPin);
    nrf_gpiote_event_disable(GPIOTE_CHANNEL);
    nrf_gpiote_int_disable(1 << GPIOTE_CHANNEL);
}

void nrf_drv_gpiote_evt_handler_t(uint32_t pin, nrf_gpiote_polarity_t action)
{
    (void)pin;
    (void)action;
    int i;
    nrf_gpiote_events_t event = NRF_GPIOTE_EVENTS_IN_0;
    uint32_t            mask  = (uint32_t)NRF_GPIOTE_INT_IN0_MASK;

    //clear the event
    for (i = 0; i < GPIOTE_CH_NUM; i++)
    {
        if (nrf_gpiote_event_is_set(event) && nrf_gpiote_int_is_enabled(mask))
        {
            nrf_gpiote_event_clear(event);
            //status |= mask;
        }

        mask <<= 1;
        /* Incrementing to next event, utilizing the fact that events are grouped together
         * in ascending order. */
        event = (nrf_gpiote_events_t)((uint32_t)event + sizeof(uint32_t));
    }

    //toggle LED
    // nrf_gpio_pin_toggle(LED_PIN);
    otPlatGpioSignalEvent(sPin);
}

void GPIOTE_IRQHandler(void)
{
    nrf_drv_gpiote_evt_handler_t(sPin, GPIOTE_CONFIG_POLARITY_LoToHi);
}

/**
 * The gpio driver weak functions definition.
 *
 */
OT_TOOL_WEAK void otPlatGpioSignalEvent(uint32_t aPin)
{
    // otLogCritPlat(sInstance, "Hello world! %d", aPin);
}
