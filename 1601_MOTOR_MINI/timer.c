#include "device_driver.h"
#define TIM2_TICK 20 //usec
#define TIM4_Hz 50000.
#define TIM2_FREQ ( 1000000. / TIM2_TICK)
#define TIM2_1ms_Pls (TIM2_FREQ/1000.)
#define TIM2_MAX (0xFFFF)
#define TIM4_FREQ (HCLK/TIM4_Hz) 
#define TIM4_1ms_Pls (TIM4_Hz/1000.)
#define TIM5_Hz 10000000

void TIM5_Out_PWM_Generation(int duty,int a,int b)
{	
	TIM5->CR1 = (0<<7)|(1<<4)|(0<<3);
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	TIM5->PSC = (TIMXCLK  / 1000000) - 1;
	// 요청한 주파수가 되도록 ARR 설정
	TIM5->ARR = (TIM5_Hz/10000) -1;
	// Duty Rate 50%가 되도록 CCR3 설정
	TIM5->CCR1 = (int)(TIM5->ARR * a * ((double)duty/100.) +0.5);
	TIM5->CCR2 = (int)(TIM5->ARR * b * ((double)duty/100.) +0.5);
	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM5->EGR,0);
	// Down Counter, Repeat Mode, Timer Start
	Macro_Set_Bit(TIM5->CR1,0);
}

void TIM2_Stopwatch_Start(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (0<<0)|(1<<3)|(1<<4);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2-> PSC = TIMXCLK*TIM2_TICK/1000000 -1;
	// (int)(TIMXCLK/TIM2_FREQ +0.5) -1;
	// ARR 초기값 설정 => 최대값 0xFFFF 설정
	TIM2->ARR = TIM2_MAX;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR,0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);
}


unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	// TIM2 stop
	Macro_Clear_Bit(TIM2->CR1,0);
	// CNT 초기 설정값 (0xffff)와 현재 CNT의 펄스수 차이를 구하고
	time = TIM2_TICK*(TIM2->ARR - TIM2->CNT);
	// 그 펄스수 하나가 20usec이므로 20을 곱한값을 time에 저장
	// 계산된 time 값을 리턴(단위는 usec)
	return time;
}

void TIM2_3s_Wait(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (0<<0)|(1<<3)|(1<<4);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = (int)(TIMXCLK/TIM2_FREQ +0.5) -1;
	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	TIM2->ARR = 3000*TIM2_1ms_Pls;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, 0);
	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR,0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);
}

int TIM2_Check_Timeout(void)
{
	// 타이머가 timeout 이면 1 리턴, 아니면 0 리턴
	if (Macro_Check_Bit_Set(TIM2->SR,0))
	{
		Macro_Clear_Bit(TIM2->SR,0);
		return 1;
	}
	else return 0;
}

void TIM2_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (0<<0)|(1<<3)|(1<<4);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = TIMXCLK*TIM2_TICK/1000000 -1;
	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	TIM2->ARR = TIM2_TICK*50*time;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, 0);
	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR,0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);
	// Wait timeout
	while(!Macro_Check_Bit_Set(TIM2->SR,0));
	// TIM2 Stop
	Macro_Clear_Bit(TIM2->CR1, 0);
}


void TIM2_Long_Delay(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (0<<0)|(1<<3)|(1<<4);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = TIMXCLK*TIM2_TICK/1000000 -1;
	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	unsigned int pls = TIM2_1ms_Pls *time;
	int n = pls / TIM2_MAX;
	int m = pls % TIM2_MAX;
	int i;

	for ( i = 0; i < n; i++)
	{
		TIM2->ARR = TIM2_MAX;
		Macro_Set_Bit(TIM2->EGR, 0);
		// UIF(Update Interrupt Pending) Clear
		Macro_Clear_Bit(TIM2->SR,0);
		// TIM2 start
		Macro_Set_Bit(TIM2->CR1,0);
		// Wait timeout
		while(!Macro_Check_Bit_Set(TIM2->SR,0));	
	}
	
	TIM2->ARR = m;
	Macro_Set_Bit(TIM2->EGR, 0);
	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR,0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1,0);
	// Wait timeout
	while(!Macro_Check_Bit_Set(TIM2->SR,0));
	// TIM2 Stop
	Macro_Clear_Bit(TIM2->CR1, 0);
}

void TIM4_Repeat(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);
	// TIM4 CR1: ARPE=0, down counter, repeat mode
	TIM4->CR1 = (0<<7)|(1<<4)|(0<<3)|(0<<0);
	// PSC(50KHz),  ARR(reload시 값) 설정
	TIM4->PSC = (int)(TIM4_FREQ+0.5)-1;
	TIM4->ARR = time*TIM4_1ms_Pls;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM4->EGR,0);
	// Update Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR,0);
	// TIM4 start
	Macro_Set_Bit(TIM4->CR1,0);
}

int TIM4_Check_Timeout(void)
{
	// 타이머가 timeout 이면 1 리턴, 아니면 0 리턴
	if (Macro_Check_Bit_Set(TIM4->SR,0))
	{
		Macro_Clear_Bit(TIM4->SR,0);
		return 1;
	}
	else return 0;
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = 50 * time;
}
