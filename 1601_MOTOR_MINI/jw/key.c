#include "device_driver.h"

int Key_Get_Pressed(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, PORTC_BTN_PIN);	
}

void Key_ISR_Enable()
{
	Macro_Set_Bit(RCC->AHB1ENR, GPIOC_EN_BIT); 				             // GPIOC CLK ON
	Macro_Write_Block(GPIOC->MODER, TWO_BIT, 0x0, PORTC_BTN_PIN * 2U);	 // Input Mode

	Macro_Set_Bit(RCC->APB2ENR, SYSCFG_EN_BIT); 	         // SYSCFG CLK ON: EXTI사용을 위해 SET 필수
	Macro_Write_Block(SYSCFG->EXTICR[3], FOUR_BIT, 0x2, 4);	 // PC13을 EXTI 13의 SCR로 설정

	Macro_Set_Bit(EXTI->FTSR, PORTC_BTN_PIN);		// Falling Edge 트리거 활성화
	Macro_Set_Bit(EXTI->RTSR, PORTC_BTN_PIN);		// Rising Edge 트리거 활성화
	EXTI->PR = (0x1 << PORTC_BTN_PIN);				// Pending Clear(쓰레기값 방지 초기화)
	
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);	   // 내부 INT Pending Clear
	Macro_Set_Bit(EXTI->IMR, PORTC_BTN_PIN);   // EXTI 활성화
	NVIC_EnableIRQ(EXTI15_10_IRQn);			   // 내부 INT 활성화

}