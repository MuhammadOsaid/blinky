
/*
•LD1 PWR: Red LED indicates that the board is powered.
•LD2 COM: LD2 default status is red. LD2 turns to green to indicate that
communications are in progress between the PC and the ST-LINK/V2.
•User LD3: Red LED is a user LED connected to the I/O PE9 of the STM32F303VCT6.
•User LD4: Blue LED is a user LED connected to the I/O PE8 of the STM32F303VCT6.
•User LD5: Orange LED is a user LED connected to the I/O PE10 of the
STM32F303VCT6.
•User LD6: Green LED is a user LED connected to the I/O PE15 of the
STM32F303VCT6.
•User LD7: Green LED is a user LED connected to the I/O PE11 of the
STM32F303VCT6.
•User LD8: Orange LED is a user LED connected to the I/O PE14 of the
STM32F303VCT6.
•User LD9: Blue LED is a user LED connected to the I/O PE12 of the STM32F303VCT6.
•User LD10: Red LED is a user LED connected to the I/O PE13 of the
STM32F303VCT6.
*/


#include <stdint.h>
#include <stdbool.h>

#include <stm32f30x.h>

extern void delay_us(uint32_t us);

#define BLINK_VERY_FAST 25000
#define BLINK_FAST 250000
#define BLINK_SLOW 500000

#define DELAY BLINK_FAST 


typedef enum ERROR_E
{
	E_NOT_OK,
	E_OK = 0

} ERROR_T;


typedef enum LED_STATUS_E
{
	LED_OFF= 0,
	LED_ON = 1

} LED_STATUS_T;

/**
 * Stub required by newlibc.
 * 
 * E.g. for malloc()
 */
void
_sbrk(void)
{
}

/**
 * Stub required by newlibc.
 *
 * Used for static constructors in C++
 */
void
_init(void)
{
}

/**
 * Initialize the system clock.
 * 
 * Uses the external 8 MHz clock from the ST-Link to generate the
 * 72 MHz system clock.
 */
ERROR_T init_clock()
{
	// use external 8MHz clock from ST-LINK
	RCC->CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
	
	// wait until the external clock is stable
	uint32_t t = 1500;
	while (!(RCC->CR & RCC_CR_HSERDY) && --t) {
	}
	
	if (!(RCC->CR & RCC_CR_HSERDY)) {
		// no external clock found => abort
		return E_NOT_OK;
	}
	
	uint32_t tmp = 0;
	// Read reserved and read-only values and clear all other values
	tmp |= RCC->CFGR & ((1 << 31) | (1 << 30) | (1 << 29) | (1 << 28) | (1 << 27) |
	                    (1 << 15) | (1 << 14) |
	                    (1 << 3) | (1 << 2));

	// PLLSRC source for PLL
	tmp |= RCC_CFGR_PLLSRC_PREDIV1;
	
	// PLLMUL = factor is user defined
	tmp |= RCC_CFGR_PLLMULL9;
	
	RCC->CFGR = tmp;
	
	// enable PLL
	RCC->CR |= RCC_CR_PLLON;	

	// Wait until the PLL is stable
	while (!(RCC->CR & RCC_CR_PLLRDY)) 
	{
		if (!(--t)) {
			return E_NOT_OK;
		}
	}
	
	// Switch to the PLL
	// AHB = 72 / 1 = 72 MHz
	RCC->CFGR = ( RCC->CFGR & 0xffff0000) |
	            ((RCC_CFGR_PPRE1_DIV2 | // APB1 = 72 / 2 = 36 MHz
	              RCC_CFGR_PPRE2_DIV1 | // APB2 = 72 / 1 = 72 MHz
	              RCC_CFGR_HPRE_DIV1 |  // SYSCLK = 72 / 1 = 72 MHz
	              RCC_CFGR_SW_PLL) &    // select PLL as source
	              0x0000ffff);

	// Wait till the main PLL is used as system clock source
	while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
	{
		/* Do nothing */
	}

	return E_OK;
}

int
main (void)
{
	init_clock();
	
	// Struct of PIO Port E
	GPIO_TypeDef *PE = GPIOE;

	// Red LED is PE9
	// set mode to 01 -> output
	PE->MODER = (1 << (9*2));

	// Orange LED is PE10
	// set mode to 01 -> output
	PE->MODER |= (1 << (10*2));


	// switch LED on (is connected between IO and GND)
	PE->ODR = (1 << 9);
	PE->ODR |= (1 << 10);
	
	while(1) {
		PE->ODR = (LED_ON << 9);
		PE->ODR |= (LED_OFF << 10);
		delay_us(DELAY);
		PE->ODR = (LED_OFF << 9);
		PE->ODR |= (LED_ON << 10);
		delay_us(DELAY);
	}
}



