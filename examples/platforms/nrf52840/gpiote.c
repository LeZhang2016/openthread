#include <common/logging.hpp>
#include <openthread/platform/gpiote.h>

#include <hal/nrf_gpiote.h>
#include <hal/nrf_gpio.h>
#include <drivers/common/nrf_drv_common.h>
#include "platform-nrf5.h"

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
    // otLogCritPlat(sInstance, "there is a signal");
}

void nrf5GpioteInit(void)
{
    // init LED pin
    nrf_gpio_cfg_output(LED_PIN);
    nrf_gpio_pin_write(LED_PIN, 0);

    // init gpiote for event/interrupt
    nrf_drv_common_irq_enable(GPIOTE_IRQn, 7);//GPIOTE_CONFIG_IRQ_PRIORITY);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_PORT);
    nrf_gpiote_int_enable(GPIOTE_INTENSET_PORT_Msk);

    nrf_gpio_cfg_input(IN_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpiote_event_configure(GPIOTE_CHANNEL, IN_PIN, GPIOTE_CONFIG_POLARITY_HiToLo);

    //nrf_drv_gpiote_in_event_enable(IN_PIN, true);
    nrf_gpiote_events_t event   = TE_IDX_TO_EVENT_ADDR(GPIOTE_CHANNEL);

    nrf_gpiote_event_enable(GPIOTE_CHANNEL);
    nrf_gpiote_event_clear(event);
    nrf_gpiote_int_enable(1 << GPIOTE_CHANNEL);
}


void nrf5GpioteDeinit(void)
{
    nrf_gpio_pin_clear(LED_PIN);
    nrf_drv_common_irq_disable(GPIOTE_IRQn);
    nrf_gpiote_event_clear(NRF_GPIOTE_EVENTS_PORT);
    nrf_gpiote_int_disable(GPIOTE_INTENSET_PORT_Msk);

    nrf_gpio_pin_clear(IN_PIN);
    nrf_gpiote_event_disable(GPIOTE_CHANNEL);
    nrf_gpiote_int_disable(1 << GPIOTE_CHANNEL);
}

void GPIOTE_IRQHandler(void)
{
    nrf_drv_gpiote_evt_handler_t(IN_PIN, GPIOTE_CONFIG_POLARITY_HiToLo);
}