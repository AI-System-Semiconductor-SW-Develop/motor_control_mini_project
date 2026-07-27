#include "device_driver.h"

void LED_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, GPIOA_EN_BIT); 

	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 10);
	Macro_Clear_Bit(GPIOA->OTYPER, 5);
	Macro_Clear_Bit(GPIOA->ODR, 5); 
}