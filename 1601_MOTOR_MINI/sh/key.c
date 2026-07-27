#include "device_driver.h"

int Key_Get_Pressed(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, 13);	
}

void Key_ISR_Enable()
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 				//GPIOC CLK ON
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);	//Input Mode

	Macro_Set_Bit(RCC->APB2ENR, 14); 				//SYSCFG CLK ON: EXTI사용을 위해 SET 필수
	Macro_Write_Block(SYSCFG->EXTICR[3], 0xf, 0x2, 4);	//PC13을 EXTI 13의 SCR로 설정

	Macro_Set_Bit(EXTI->FTSR, PC13);		//Falling Edge 트리거 활성화
	Macro_Set_Bit(EXTI->RTSR, PC13);		//Rising Edge 트리거 활성화
	EXTI->PR = 0x1 << PC13;				//Pending Clear(쓰레기값 방지 초기화)
	
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);	//내부 INT Pending Clear
	Macro_Set_Bit(EXTI->IMR, PC13);			//EXTI 활성화
	NVIC_EnableIRQ(EXTI15_10_IRQn);			//내부 INT 활성화

}