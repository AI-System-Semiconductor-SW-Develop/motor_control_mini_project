#include "device_driver.h"
#include <stdio.h>
#include "gpio.h"

void GPIO_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR, GPIOA_EN_BIT);	// GPIOA En
	Macro_Set_Bit(RCC->AHB1ENR, GPIOC_EN_BIT);	// GPIOC En

    Macro_Write_Block(GPIOA->MODER, FOUR_BIT, 0x5, 0U);	           // PA0, PA1 General output
    Macro_Write_Block(GPIOA->OTYPER, TWO_BIT, PUSH_PULL_MODE, 0U);  // PA0, PA1 Push-pull
    
	Macro_Write_Block(GPIOC->MODER, TWO_BIT, INPUT_MODE, PORTC_BTN_PIN * 2U);	// key mode input
}

void GPIO_Mode_CW(void)
{
    Macro_Write_Block(GPIOA->MODER, TWO_BIT, ALTERNATE_FUNCTION_MODE, 0U);  // PA0 Alternate funtion mode
    Macro_Write_Block(GPIOA->AFR[0], FOUR_BIT, 0x2U, 0U);      // PA0 주변장치 매핑
    Macro_Write_Block(GPIOA->PUPDR, TWO_BIT, 0x1U, 0U);       // PA0 Pull-up

    Macro_Write_Block(GPIOA->MODER, TWO_BIT, GENERAL_OUTPUT_MODE, 2U);    // PA1 General Output mode
    Macro_Clear_Bit(GPIOA->OTYPER, 1U);                  // PA1 Output push-pull
}

void GPIO_Mode_CCW(void)
{
    Macro_Write_Block(GPIOA->MODER, TWO_BIT, GENERAL_OUTPUT_MODE, 0U);  // PA0 General Output mode
    Macro_Clear_Bit(GPIOA->OTYPER, 0);                  // PA0 Output push-pull

    Macro_Write_Block(GPIOA->MODER, TWO_BIT, ALTERNATE_FUNCTION_MODE, 2U);   // PA1 Alternate funtion mode
    Macro_Write_Block(GPIOA->AFR[0], FOUR_BIT, 0x2U, 4U);      // PA1 주변장치 매핑
    Macro_Write_Block(GPIOA->PUPDR, TWO_BIT, 0x1U, 2U);       // PA1 Pull-up
}

void GPIO_Mode_Stop(void)
{
    Macro_Write_Block(GPIOA->MODER, FOUR_BIT, 0x5U, 0U);	// PA0, PA1 General output
    Macro_Write_Block(GPIOA->OTYPER, TWO_BIT, 0x0U, 0U);	// PA0, PA1 Push-pull
}