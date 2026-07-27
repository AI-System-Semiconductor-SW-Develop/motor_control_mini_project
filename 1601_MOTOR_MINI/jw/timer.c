#include "device_driver.h"
#include <stdio.h>
#include "timer.h"

/* ========== TIMER 2: 3초 타이머용 ========== */
void TIM2_Timer_Start(int msec)
{
	Macro_Set_Bit(RCC->APB1ENR, TIM2_EN_BIT);  // TIM2 CLK Enable

	Macro_Clear_Bit(TIM2->CR1, CEN_BIT);                    // Timer2 Stop
	Macro_Write_Block(TIM2->CR1, THREE_BIT, 0x7U, URS_BIT);	// down count, one pulse, URS

	TIM2->PSC = (int)(TIMXCLK/(double)TIM2_FREQ + 0.5) - 1;	// 분주비 1920, 50kHz
	TIM2->ARR = TIM2_1ms_Pls * msec;

	Macro_Set_Bit(TIM2->EGR, UG_BIT);	 // Update event Generation

	Macro_Clear_Bit(TIM2->SR, UIF_BIT);	 // TIM4 Pending Clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);     // NVIC Pending Clear

	Macro_Set_Bit(TIM2->DIER, UIE_BIT);  // TIM4 Interrupt Enable
	NVIC_EnableIRQ(TIM2_IRQn);		     // NVIC Interrupt Enable

	Macro_Set_Bit(TIM2->CR1, CEN_BIT);	 // TIM2 Start
}

// 타이머 종료
void TIM2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, CEN_BIT);

	Macro_Clear_Bit(TIM2->DIER, UIE_BIT);  // 인터럽트 비활성화
    NVIC_DisableIRQ(TIM2_IRQn);            // NVIC 비활성화
    
    Macro_Clear_Bit(TIM2->SR, UIF_BIT);    // UIF 펜딩 클리어
    NVIC_ClearPendingIRQ(TIM2_IRQn);       // NVIC 펜딩 클리어
}

/* ========== TIMER 4: 딜레이용 ========== */
void TIM4_Timer_Start(int msec)
{
	Macro_Set_Bit(RCC->APB1ENR, TIM4_EN_BIT);  // TIM4 CLK Enable
	
	Macro_Clear_Bit(TIM4->CR1, CEN_BIT);		             // TIM4 stop
	Macro_Write_Block(TIM4->CR1, THREE_BIT, 0x7U, URS_BIT);   // down count, one pulse, URS

	TIM4->PSC = (int)(TIMXCLK/(double)TIM4_FREQ + 0.5) - 1;  // 분주비 1920, 50kHz
	TIM4->ARR = TIM4_1ms_Pls * msec;

	Macro_Set_Bit(TIM4->EGR, UG_BIT);	 // Update event

	Macro_Clear_Bit(TIM4->SR, UIF_BIT);  // TIM4 Pending Clear
	NVIC_ClearPendingIRQ(TIM4_IRQn);	 // NVIC Pending Clear

	Macro_Set_Bit(TIM4->DIER, UIE_BIT);	 // TIM4 Interrupt Enable
	NVIC_EnableIRQ(TIM4_IRQn);			 // NVIC Interrupt Enable

	Macro_Set_Bit(TIM4->CR1, CEN_BIT);	 // TIM4 Start
}

/* ========== TIMER 5: 모터 속도 제어용 PWM ========== */
void TIM5_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, GPIOA_EN_BIT);
	Macro_Set_Bit(RCC->APB1ENR, TIM5_EN_BIT);
	
	Macro_Write_Block(TIM5->CCMR1, 0xffffU, 0x7878U, 0U);	// PWM 2 mode로 설정 CNT>CCR(duty)일 때, LOW
	TIM5->CCER = (0<<5)|(1<<4)|(0<<1)|(1<<0);	        // TIM5 CH1, CH2 Capture&Compare Enable
}

void TIM5_PWM_Init(int duty)
{
	Macro_Write_Block(TIM5->CR1, TWO_BIT, 0x2U, 3U);
	
	TIM5->PSC = (int)(TIMXCLK/(double)TIM5_FREQ + 0.5) - 1;  // TIM5_FREQ를 만들 수 있게 분주
	TIM5->ARR = (int)((double)TIM5_FREQ / FREQ_TARGET);		 // 목표주파수 10kHz
	
	TIM5->CCR1 = TIM5->ARR * duty / 100;		// PA0 duty
	TIM5->CCR2 = TIM5->ARR * duty / 100;		// PA1 duty
	
	Macro_Set_Bit(TIM5->EGR, UG_BIT);	       // Manual Update
}

void TIM5_PWM_Generation(int duty)
{
	TIM5->CCR1 = TIM5->ARR * duty / 100;	// PA0 duty
	TIM5->CCR2 = TIM5->ARR * duty / 100;	// PA1 duty

	Macro_Set_Bit(TIM5->EGR, UG_BIT);
}

void TIM5_PWM_Enable(void)
{
	Macro_Set_Bit(TIM5->CR1, CEN_BIT);
}

void TIM5_PWM_Disable(void)
{
	Macro_Clear_Bit(TIM5->CR1, CEN_BIT);
}

