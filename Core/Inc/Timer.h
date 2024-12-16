#ifndef __TIMER
#define __TIMER

#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include <stdint.h>

void TIMER_Init(void);
void SysTick_Handler(void);
void TIMER_Start(void);
uint32_t TIMER_GetTime(void);

void TIMER2_Init(void);
void TIM2_IRQHandler(void);

void TIMER2_Start(void);


#endif
