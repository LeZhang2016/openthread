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
#include "platform-nrf5.h"


static uint8_t sPinNumbers[30];
otInstance *sInstance;
// static uint32_t receiverTime = 0, senderTime = 0;
static int state = 0;

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
            nrf_gpio_pin_toggle(13);
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
    // for (uint32_t i = 0; i < sizeof(sPinNumbers)/sPinNumbers[0]; i++)
    // {
    //     if (sPinNumbers[i] == 1)
    //     {
    //         nrf_gpio_pin_clear(i);
    //         sPinNumbers[i] = 0;
    //     }
    // }
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
/**
 * The gpio driver weak functions definition.
 *
 */
OT_TOOL_WEAK void otPlatGpioSignalEvent(uint32_t aPin)
{
    // otLogCritPlat(sInstance, "Hello world! %d", aPin);
}
