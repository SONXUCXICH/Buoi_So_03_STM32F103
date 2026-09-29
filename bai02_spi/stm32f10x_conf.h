#ifndef __STM32F10X_CONF_H
#define __STM32F10X_CONF_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"

#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line);

#define assert_param(expr) \
    ((expr) ? (void)0 : \
    assert_failed((uint8_t *)__FILE__, _LINE_))

#else

#define assert_param(expr) ((void)0)

#endif

#endif
