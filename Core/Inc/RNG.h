/*
 * RNG.h
 *
 *  Created on: Dec 1, 2024
 *      Author: Will Fraser
 */

#ifndef __RNG_H
#define __RNG_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_rng.h"

#define NB_OF_GENERATED_RANDOM_NUMBERS 3

#define BUTTON_PORT GPIOA
#define BUTTON_PIN  GPIO_PIN_0
#define EXTI0_BUTTON_IRQ_NUMBER EXTI0_IRQn

#define PRESSED   GPIO_PIN_RESET
#define RELEASED  GPIO_PIN_SET

void RNG_Init(void);
uint32_t RandomNumbersGeneration(void);
uint32_t SetSystemToHSI(void);
void BUTTON_Init(void);

#endif
