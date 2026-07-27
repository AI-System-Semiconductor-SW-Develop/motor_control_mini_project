#include "device_driver.h"
#include <stdio.h>

#define TIM2_TICK	20	//usec
#define TIM2_FREQ	(1000000./TIM2_TICK)	//Hz로 단위 통일하기 위해 1000000.을 곱해줌. 계산할 때 오버플로우 방지를 위해 괄호 쓰기.
#define TIM2_1ms_Pls	(TIM2_FREQ/1000.)
#define TIM2_MAX		(0xffffffff)

#define TIM4_TICK	  		(20) 				// usec
#define TIM4_FREQ 	  		(1000000/TIM4_TICK) // Hz
#define TIME4_PLS_OF_1ms  	(1000/TIM4_TICK)
#define TIM4_MAX	  		(0xffffu)


/* TIMER 2: 3초 타이머용 */

void TIM2_3sec_Timer_Start(void){

	Macro_Set_Bit(RCC->APB1ENR, 0);		//TMR2 en set

	//TIMx->CR1: URS(Bit2) 설정, over/underflow발생시에만 UIF set
	Macro_Write_Block(TIM2->CR1, 0x7, 0x7, 2);		//down count, one pulse, URS

	TIM2->PSC = (int)(TIMXCLK/TIM2_FREQ +0.5) - 1;	//분주비 1920, 50kHz
	TIM2->ARR = 150000;

	Macro_Set_Bit(TIM2->EGR, 0);	//Update event Generation

	Macro_Clear_Bit(TIM2->SR, 0);			// TIM4 Pending Clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);		// NVIC Pending Clear

	Macro_Set_Bit(TIM2->DIER, 0);		// TIM4 Interrupt Enable
	NVIC_EnableIRQ(TIM2_IRQn);		// NVIC Interrupt Enable

	Macro_Set_Bit(TIM2->CR1, 0);	//TIM2 Start
}


int TIM2_Check_Timeout(void){

	if(Macro_Check_Bit_Set(TIM2->SR, 0)){
		//printf("Timeout !!!\n");
		Macro_Clear_Bit(TIM2->SR, 0);	//flag clear
		return 1;
	}
	else           
		return 0;
}

void TIM2_Stop(void)
{
	//printf("timer stop\n");
	Macro_Clear_Bit(TIM2->CR1, 0);

	Macro_Clear_Bit(TIM2->DIER, 0);  // 인터럽트 비활성화
    NVIC_DisableIRQ(TIM2_IRQn);             // NVIC 비활성화
    
    Macro_Clear_Bit(TIM2->SR, 0);    // UIF 펜딩 클리어
    NVIC_ClearPendingIRQ(TIM2_IRQn);        // NVIC 펜딩 클리어
}

/* TIMER 4: 딜레이용 */

void TIM4_Delay_Interrupt_Enable(int time)
{
	// TIM4 Clock On
	Macro_Set_Bit(RCC->APB1ENR, 2);		//TMR4 en set
	
	Macro_Clear_Bit(TIM4->CR1, 0);		//TIM4 stop
	Macro_Write_Block(TIM4->CR1, 0x7, 0x7, 2);		//down count, one pulse, URS

	TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
	TIM4->ARR = TIME4_PLS_OF_1ms * time;

	Macro_Set_Bit(TIM4->EGR,0);			//Update event

	Macro_Clear_Bit(TIM4->SR, 0);		// TIM4 Pending Clear
	NVIC_ClearPendingIRQ(TIM4_IRQn);	// NVIC Pending Clear

	Macro_Set_Bit(TIM4->DIER, 0);		// TIM4 Interrupt Enable
	NVIC_EnableIRQ(TIM4_IRQn);			// NVIC Interrupt Enable

	Macro_Set_Bit(TIM4->CR1, 0);		// TIM4 Start
}

/* TIMER 5: 모터 속도 제어용 PWM*/

#define TIM5_FREQ			(1000000)
#define FREQ_TARGET			(10000)
#define TIM5_TICK			(1000000/TIM5_FREQ)		//100usec
#define TIM5_PLS_OF_1ms		(1000/TIM5_TICK)		//10pls = 1msec

void TIM5_Out_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);
	Macro_Set_Bit(RCC->APB1ENR, 3);
	
	Macro_Write_Block(TIM5->CCMR1,0xffff, 0x7878, 0);	//PWM 2 mode로 설정 CNT>CCR(duty)일 때, LOW
	TIM5->CCER = (0<<5)|(1<<4)|(0<<1)|(1<<0);	//TIM5 CH1, CH2 Capture&Compare Enable
}

void TIM5_PWM_Generation(int duty){
	
	Macro_Write_Block(TIM5->CR1, 0x3, 0x2, 3);
	TIM5->PSC = TIMXCLK / TIM5_FREQ - 1;		//TIM5_FREQ를 만들 수 있게 분주
	TIM5->ARR = (int)((double)TIM5_FREQ / FREQ_TARGET);		//목표주파수 10kHz

	TIM5->CCR1 = TIM5->ARR * duty / 100;		//PA0 duty
	TIM5->CCR2 = TIM5->ARR * duty / 100;		//PA1 duty

	Macro_Set_Bit(TIM5->EGR, 0);	//Manual Update

	Macro_Set_Bit(TIM5->CR1, 0);	//Timer Start
}
