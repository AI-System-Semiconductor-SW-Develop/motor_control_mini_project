#include "device_driver.h"
#include <stdio.h>

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

extern volatile int Key_Pressed;
extern volatile int Key_Released;
extern volatile int Key_State;

void EXTI9_5_IRQHandler(void)
{

	if (Key_Get_Pressed())
	{
		if(Key_State==0)
		{
			Key_State = 1;
			Key_Pressed = 1;
		}
	
	}
	else
	{
		if (Key_State == 1)
		{
			Key_Released = 1;
			Key_State = 0;
		}
	}
	

	EXTI->PR = 0x1 << 7;
	NVIC_ClearPendingIRQ(23);
}

extern volatile int Uart_Data_In;
extern volatile unsigned char Uart_Data;

void USART2_IRQHandler(void)
{
	Uart_Data = (unsigned char)USART2->DR;
	Uart_Data_In = 1;
	NVIC_ClearPendingIRQ(38);
}

extern volatile int TIM4_Expired;

void TIM4_IRQHandler(void)
{
	// TIM4 Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR,0);
	// NVIC Pending Clear
	NVIC_ClearPendingIRQ(30);
	
	TIM4_Expired = 1;
}