#include "Timer.h"

static volatile uint32_t msTicks = 0;    // Milliseconds
static uint32_t start_time;

void TIMER_Init(void) {

	SysTick_Config(SystemCoreClock / 1000);    // 1ms interrupt
}

// SYSTICK interrupt handler
void SysTick_Handler(void) {
	msTicks++;
	HAL_IncTick();
}

void TIMER_Start(void) {
	start_time = msTicks;
}

uint32_t TIMER_GetTime(void) {
	return msTicks - start_time;
}

TIM_HandleTypeDef htim2; // handle for Timer 2

void TIMER2_Init(void) {
	__HAL_RCC_TIM2_CLK_ENABLE(); // enable the clock for Timer 2

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 62500 - 1;      // prescaler (16 MHz / 16000 = 1 kHz)
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP; // count up mode
	htim2.Init.Period = 256 - 1;   // period for 1 second (1 kHz / 1000 = 1 Hz)
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	HAL_TIM_Base_Init(&htim2);

	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0); // set priority for Timer 2 interrupt
	HAL_NVIC_EnableIRQ(TIM2_IRQn);         // enable Timer 2 interrupt
}

void TIM2_IRQHandler(void) {
	HAL_TIM_IRQHandler(&htim2);
}

void TIMER2_Start(void) {
	HAL_TIM_Base_Start_IT(&htim2);			// start timer 2
}

