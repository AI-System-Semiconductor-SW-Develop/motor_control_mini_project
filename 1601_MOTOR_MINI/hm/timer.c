#include "device_driver.h"
#include <stdio.h>

#define TIM2_TICK		20U								// usec
#define TIM2_FREQ		(1000000U / TIM2_TICK)			// 50kHz
#define TIM2_1ms_Pls	(1000U / TIM2_TICK)				// 50 counts = 1ms
#define TIM2_MAX		(0xffffffffU)

#define TIM4_TICK		20U								// usec
#define TIM4_FREQ		(1000000U / TIM4_TICK)			// 50kHz
#define TIME4_PLS_OF_1ms (1000U / TIM4_TICK)			// 50 counts = 1ms
#define TIM4_MAX		(0xffffU)

extern volatile int TIM2_Expired;
extern volatile int TIM4_Expired;

/* ========== TIMER 2: 3초 길게 누름 판정 (one-pulse, down count) ========== */

void TIM2_3sec_Timer_Start(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);					// TIM2 clock enable

	Macro_Clear_Bit(TIM2->CR1, 0);					// 재시작 전 정지
	// URS(bit2)=1: over/underflow 때만 UIF set, OPM(bit3)=1, DIR(bit4)=1(down)
	Macro_Write_Block(TIM2->CR1, 0x7, 0x7, 2);
	TIM2->PSC = (unsigned int)(TIMXCLK / (double)TIM2_FREQ + 0.5) - 1;	// 50kHz
	// down count는 ARR → 0까지 (ARR+1)카운트이므로 -1
	TIM2->ARR = LONG_PRESS_MS * TIM2_1ms_Pls - 1;

	Macro_Set_Bit(TIM2->EGR, 0);					// PSC/ARR 반영 (URS=1이라 UIF 안 생김)

	Macro_Clear_Bit(TIM2->SR, 0);					// TIM2 UIF clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);
	TIM2_Expired = 0;								// 이전 만료 플래그 제거

	Macro_Set_Bit(TIM2->DIER, 0);					// TIM2 Update interrupt enable
	NVIC_EnableIRQ(TIM2_IRQn);

	Macro_Set_Bit(TIM2->CR1, 0);					// TIM2 start
}

int TIM2_Check_Timeout(void)
{
	if (Macro_Check_Bit_Set(TIM2->SR, 0)) {
		Macro_Clear_Bit(TIM2->SR, 0);
		return 1;
	}
	return 0;
}

/*
 * 타이머를 멈추고, 멈추기 직전에 ISR이 세운 만료 플래그까지 지운다.
 * 플래그를 마지막에 지워야 "정지 직전 만료" 경쟁에서도 플래그가 남지 않는다.
 */
void TIM2_Stop(void)
{
	Macro_Clear_Bit(TIM2->CR1, 0);					// TIM2 stop
	Macro_Clear_Bit(TIM2->DIER, 0);					// interrupt disable
	Macro_Clear_Bit(TIM2->SR, 0);					// UIF clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);
	TIM2_Expired = 0;
}

/* ========== TIMER 4: 방향 전환 전 정지 대기 (one-pulse, down count) ========== */

void TIM4_Delay_Interrupt_Enable(unsigned int msec)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);					// TIM4 clock enable

	Macro_Clear_Bit(TIM4->CR1, 0);					// TIM4 stop
	Macro_Write_Block(TIM4->CR1, 0x7, 0x7, 2);		// URS, one pulse, down count
	TIM4->PSC = (unsigned int)(TIMXCLK / (double)TIM4_FREQ + 0.5) - 1;	// 50kHz
	TIM4->ARR = TIME4_PLS_OF_1ms * msec - 1;		// 16비트: 최대 약 1310ms

	Macro_Set_Bit(TIM4->EGR, 0);

	Macro_Clear_Bit(TIM4->SR, 0);
	NVIC_ClearPendingIRQ(TIM4_IRQn);
	TIM4_Expired = 0;

	Macro_Set_Bit(TIM4->DIER, 0);
	NVIC_EnableIRQ(TIM4_IRQn);

	Macro_Set_Bit(TIM4->CR1, 0);					// TIM4 start
}

/* 정지 명령이 들어오면 진행 중인 방향 전환 대기를 취소한다. */
void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
	Macro_Clear_Bit(TIM4->DIER, 0);
	Macro_Clear_Bit(TIM4->SR, 0);
	NVIC_ClearPendingIRQ(TIM4_IRQn);
	TIM4_Expired = 0;
}

/* ========== TIMER 5: 모터 속도 PWM (10kHz) ========== */

#define TIM5_FREQ		(1000000U)						// 카운터 1MHz (1카운트 = 1usec)
#define FREQ_TARGET		(10000U)						// PWM 10kHz
#define TIM5_PERIOD		(TIM5_FREQ / FREQ_TARGET)		// 100카운트 = 100usec

/*
 * 상향 카운트 + PWM mode 1, CCxP=0(Active High)
 *   CNT < CCR 동안 High  →  핀 High 비율 = CCR / (ARR+1) = duty / 100
 * OCxPE(프리로드)를 켜서 CCR 변경은 다음 주기 시작(update)부터 반영된다.
 */
void TIM5_PWM_Init(int duty)
{
	Macro_Set_Bit(RCC->AHB1ENR, 0);					// GPIOA clock enable
	Macro_Set_Bit(RCC->APB1ENR, 3);					// TIM5 clock enable

	Macro_Clear_Bit(TIM5->CR1, 0);					// 설정 중 정지
	Macro_Write_Block(TIM5->CR1, 0x3, 0x0, 3);		// OPM=0, DIR=0(up)
	// CH1/CH2: OCxM=110(PWM mode 1), OCxPE=1  → 0x68씩
	Macro_Write_Block(TIM5->CCMR1, 0xffff, 0x6868, 0);
	TIM5->CCER = (0 << 5) | (1 << 4) | (0 << 1) | (1 << 0);	// CH1·CH2 출력, Active High

	TIM5->PSC = (unsigned int)(TIMXCLK / (double)TIM5_FREQ + 0.5) - 1;	// 96MHz → 1MHz
	TIM5->ARR = TIM5_PERIOD - 1;					// 0~99, 정확히 10kHz

	TIM5->CCR1 = (unsigned int)duty;				// PA0
	TIM5->CCR2 = (unsigned int)duty;				// PA1

	Macro_Set_Bit(TIM5->EGR, 0);					// 초기값 반영 (최초 1회만)
	Macro_Set_Bit(TIM5->CR1, 0);					// TIM5 start
}

/* 동작 중 듀티 변경: CCR만 갱신 (카운터 리셋 없음 → 파형 끊김 없음) */
void TIM5_Set_Duty(int duty)
{
	if (duty < 0)   duty = 0;
	if (duty > 100) duty = 100;
	TIM5->CCR1 = (unsigned int)duty;
	TIM5->CCR2 = (unsigned int)duty;
}

#if 0
/* TIMER 2 */

void TIM2_Stopwatch_Start(void)
{
	Macro_Set_Bit(RCC->APB1ENR, 0);		//tim2 en set.

	// TIM2 CR1 설정: down count, one pulse
	Macro_Write_Block(TIM2->CR1, 0x3, 0x3, 3);
	//TIM2->CR1 = (0x1 << 4) | (0x1 << 3);	매크로만 쓰지 말고 비트로 직접 하는 것도 연습해야겟어...
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = TIMXCLK * TIM2_TICK/1000000 - 1;
	// TIM2->PSC = (int)(TIMXCLK/TIM2_FREQ +0.5) - 1;		//TIMXCLK/TIM2_FREQ하면 실수니까 +0.5하고 int형으로 보정.
	// TIM2->PSC = TIMXCLK * (TIM2_TICK/1000000.) - 1;		//오버플로우 방지하려면 실수형으로(. 추가) 만들어서 괄호 사용.
	// TIM2->PSC = 1920-1;
	//  ARR 초기값 설정 => 최대값 0xFFFF 설정
	TIM2->ARR = 0xffffffff;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, 0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1, 0);
}

unsigned int TIM2_Stopwatch_Stop(void)
{
	unsigned int time;

	// TIM2 stop
	Macro_Clear_Bit(TIM2->CR1, 0);
	// CNT 초기 설정값 (0xffff)와 현재 CNT의 펄스수 차이를 구하고
	// 그 펄스수 하나가 20usec이므로 20을 곱한값을 time에 저장
	time = (TIM2->ARR - TIM2->CNT) * TIM2_TICK;
	// 계산된 time 값을 리턴(단위는 usec)
	return time;
}

void TIM2_Delay(int time)		//시간 제약이 거의 없는 delay함수
{
	Macro_Set_Bit(RCC->APB1ENR, 0);

	// TIM2 CR1 설정: down count, one pulse
	TIM2->CR1 = (0x1 << 4) | (0x1 << 3);
	// PSC 초기값 설정 => 20usec tick이 되도록 설계 (50KHz)
	TIM2->PSC = TIMXCLK * (TIM2_TICK/1000000.) - 1;
	// ARR 초기값 설정 => 요청한 time msec에 해당하는 초기값 설정
	unsigned int pls = TIM2_1ms_Pls * time;		//pls의 최댓값이 셀 수 있는 최댓값. 42억/50-> 1억msec 정도.
	int n = pls / TIM2_MAX;
	int m = pls % TIM2_MAX;
	int i;

	for (i = 0; i < n; i++){
		TIM2->ARR = TIM2_MAX;
		Macro_Set_Bit(TIM2->EGR, 0);
		Macro_Clear_Bit(TIM2->SR, 0);
		Macro_Set_Bit(TIM2->CR1, 0);
		while(!Macro_Check_Bit_Set(TIM2->SR, 0));
	}
	TIM2->ARR = m;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM2->EGR, 0);
	// UIF(Update Interrupt Pending) Clear
	Macro_Clear_Bit(TIM2->SR, 0);
	// TIM2 start
	Macro_Set_Bit(TIM2->CR1, 0);
	// Wait timeout
	while(!Macro_Check_Bit_Set(TIM2->SR, 0));


	// TIM2 Stop
	Macro_Clear_Bit(TIM2->CR1, 0);
}


/* TIMER 4 */
void TIM4_Repeat_Interrupt_Enable(int en, int time)
{
	if(en)
	{
		// TIM4 Clock On
		Macro_Set_Bit(RCC->APB1ENR, 2);

		TIM4->CR1 = (1<<4)|(0<<3);
		TIM4->PSC = (unsigned int)(TIMXCLK/(double)TIM4_FREQ + 0.5)-1;
		TIM4->ARR = TIME4_PLS_OF_1ms * time;
		Macro_Set_Bit(TIM4->EGR,0);

		// TIM4 Pending Clear
		Macro_Clear_Bit(TIM4->SR, 0);
		// NVIC Pending Clear
		NVIC_ClearPendingIRQ(30);

		// TIM4 Interrupt Enable
		Macro_Set_Bit(TIM4->DIER, 0);
		// NVIC Interrupt Enable
		NVIC_EnableIRQ(30);

		// TIM4 Start
		Macro_Set_Bit(TIM4->CR1, 0);
	}

	else
	{
		NVIC_DisableIRQ(30);
		Macro_Clear_Bit(TIM4->CR1, 0);
		Macro_Clear_Bit(TIM4->DIER, 0);
	}
}

void TIM4_Repeat(int time)
{
	Macro_Set_Bit(RCC->APB1ENR, 2);
	
	// TIM4 CR1: ARPE=0, down counter, repeat mode
	TIM4->CR1 = (0x0 << 7) | (0x1 << 4) | (0x0 << 3);
	// PSC(50KHz),  ARR(reload시 값) 설정
	TIM4->PSC = TIMXCLK * (TIM2_TICK/1000000.) - 1;
	TIM4->ARR = time * TIM2_1ms_Pls;
	// UG 이벤트 발생
	Macro_Set_Bit(TIM4->EGR, 0);
	// Update Interrupt Pending Clear
	Macro_Clear_Bit(TIM4->SR, 0);
	// TIM4 start
	Macro_Set_Bit(TIM4->CR1, 0);
}

int TIM4_Check_Timeout(void)
{
	// 타이머가 timeout 이면 1 리턴, 아니면 0 리턴
	if(Macro_Check_Bit_Set(TIM4->SR, 0)){
		Macro_Clear_Bit(TIM4->SR, 0);
		return 1;
	}
	return 0;
}

void TIM4_Stop(void)
{
	Macro_Clear_Bit(TIM4->CR1, 0);
}

void TIM4_Change_Value(int time)
{
	TIM4->ARR = 50 * time;
}

#endif