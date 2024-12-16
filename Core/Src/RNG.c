#include "RNG.h"

__IO uint32_t aRandom32bit[NB_OF_GENERATED_RANDOM_NUMBERS];

void RNG_Init(void) {
	LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_RNG); // RNG clock

	// Set HSI and ignore PLL
	while (SetSystemToHSI() != 0) {
	};

	// PLLSAI
	LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336,
			LL_RCC_PLLQ_DIV_7);

	// Latency
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);

	// PLL enable
	LL_RCC_PLL_Enable();

	while (LL_RCC_PLL_IsReady() != 1) {
	};

	// Turn on Sysclk
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
	};

	// Set systick as 1ms
	SysTick_Config(168000000 / 1000);

	SystemCoreClock = 168000000;
}

uint32_t RandomNumbersGeneration(void) {
	register uint8_t index = 0;

	// init RNG perhiperal
	LL_RNG_Enable(RNG);

	for (index = 0; index < NB_OF_GENERATED_RANDOM_NUMBERS; index++) {

		// wait for DRDY flag
		while (!LL_RNG_IsActiveFlag_DRDY(RNG)) {
		}

		// read and store 32 bit random num
		aRandom32bit[index] = LL_RNG_ReadRandData32(RNG);
		return aRandom32bit[index];
	}

	// stop RNG
	LL_RNG_Disable(RNG);

	return 0;
}

uint32_t SetSystemToHSI(void) {
	uint32_t timeout = 0;
	// Enable HSI
	if (LL_RCC_HSI_IsReady() != 1) {
		LL_RCC_HSI_Enable();
		timeout = 1000;
		while ((LL_RCC_HSI_IsReady() != 1) && (timeout != 0)) {
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
				timeout--;
			}
			if (timeout == 0) {
				return 1;
			}
		}
	}

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

	SystemCoreClock = HSI_VALUE;

	// Disable PLL
	if (LL_RCC_PLL_IsReady() != 0) {
		LL_RCC_PLL_Disable();
		timeout = 1000;
		while ((LL_RCC_PLL_IsReady() != 0) && (timeout != 0)) {
			if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
				timeout--;
			}
			if (timeout == 0) {
				return 1;
			}
		}
	}
	return 0;
}

void BUTTON_Init(void) {

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef buttonConfig;

	// Setup button GPIO params
	buttonConfig = (GPIO_InitTypeDef ) {0};

	buttonConfig.Pin = GPIO_PIN_0;
	buttonConfig.Mode = GPIO_MODE_IT_RISING_FALLING;
	buttonConfig.Pull = GPIO_NOPULL;
	buttonConfig.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(BUTTON_PORT, &buttonConfig);

	// Set button IRQ
	HAL_NVIC_SetPriority((IRQn_Type) (EXTI0_IRQn), 0x0F, 0x00);
	HAL_NVIC_EnableIRQ((IRQn_Type) (EXTI0_IRQn));
}
