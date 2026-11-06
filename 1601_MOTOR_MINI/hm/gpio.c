#include "device_driver.h"
#include <stdio.h>

void GPIO_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);						// GPIOA clock enable
	Macro_Set_Bit(RCC->AHB1ENR, 2);						// GPIOC clock enable (bit2 = GPIOC, bit3은 GPIOD)

	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);			// 출력 전환 전 Low로 준비
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);		// PA0, PA1 General output
	Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, 0);		// PA0, PA1 Push-pull
	Macro_Write_Block(GPIOA->PUPDR, 0xf, 0x0, 0);		// PA0, PA1 No pull
	Macro_Write_Block(GPIOA->AFR[0], 0xff, 0x22, 0);	// PA0/PA1 = AF2(TIM5_CH1/CH2), AF 모드일 때만 사용

	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);		// PC13 key input
}

/* 정회전: PA0 = TIM5_CH1 PWM, PA1 = GPIO High
 * ※ PA1 High가 드라이버에서 어떤 의미인지(구동/브레이크)는 드라이버 진리표로 확인할 것 */
void GPIO_Mode_CW(void)
{
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 0);		// PA0 Alternate function (PWM)
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 2);		// PA1 General output
}

/* 역회전: PA0 = GPIO High, PA1 = TIM5_CH2 PWM */
void GPIO_Mode_CCW(void)
{
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 0);		// PA0 General output
	Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 2);		// PA1 Alternate function (PWM)
}

/* 정지: PA0, PA1 모두 GPIO 출력 (ODR 값은 호출 측에서 Low로 설정) */
void GPIO_Mode_Stop(void)
{
	Macro_Write_Block(GPIOA->MODER, 0xf, 0x5, 0);		// PA0, PA1 General output
}
