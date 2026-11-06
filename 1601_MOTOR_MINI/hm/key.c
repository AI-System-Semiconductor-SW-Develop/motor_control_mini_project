#include "device_driver.h"

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_Get_Pressed(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, 13);	
}

void Key_Wait_Key_Pressed(void)
{
	while(!Macro_Check_Bit_Clear(GPIOC->IDR, 13));
}

void Key_Wait_Key_Released(void)
{
	while(!Macro_Check_Bit_Set(GPIOC->IDR, 13));
}

void Key_ISR_Enable(int en)
{
	if(en)
	{
		Macro_Set_Bit(RCC->AHB1ENR, 2); 				//GPIOC CLK ON
		Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);	//Input Mode

		Macro_Set_Bit(RCC->APB2ENR, 14); 				//SYSCFG CLK ON: EXTI사용을 위해 SET 필수
		Macro_Write_Block(SYSCFG->EXTICR[3], 0xf, 0x2, 4);	//PC13을 EXTI 13의 SCR로 설정

		Macro_Set_Bit(EXTI->FTSR, PC13);		//Falling Edge 트리거 활성화
		Macro_Set_Bit(EXTI->RTSR, PC13);		//Rising Edge 트리거 활성화
		EXTI->PR = 0x1 << PC13;				//Pending Clear(쓰레기값 방지 초기화)
		
		NVIC_ClearPendingIRQ(EXTI15_10);	//내부 INT Pending Clear
		Macro_Set_Bit(EXTI->IMR, PC13);			//EXTI 활성화
		NVIC_EnableIRQ(EXTI15_10);			//내부 INT 활성화
	}

	else
	{
		NVIC_DisableIRQ(EXTI15_10);
	}
}

/*
 * 버튼 이벤트 큐 (EXTI ISR → 메인 루프, 생산자 1·소비자 1)
 *  - 변수 하나(key_state)만 쓰면 메인이 처리하기 전에 누름이 뗌으로 덮여 짧은 누름을 놓친다.
 *  - head는 ISR만, tail은 메인만 쓰므로 인터럽트를 막지 않아도 안전하다.
 */
#define KEY_Q_SIZE	8U					// 2의 거듭제곱
#define KEY_Q_MASK	(KEY_Q_SIZE - 1U)

static volatile KEY_STATE key_q[KEY_Q_SIZE];
static volatile unsigned int key_q_head = 0;	// ISR이 씀
static volatile unsigned int key_q_tail = 0;	// 메인이 씀
volatile int Key_Q_Overflow = 0;

void Key_Event_Push(KEY_STATE ev)		// ISR 전용
{
	unsigned int next = (key_q_head + 1U) & KEY_Q_MASK;

	if (next == key_q_tail) {			// 가득 참: 새 이벤트 버림, 사실은 기록
		Key_Q_Overflow = 1;
		return;
	}
	key_q[key_q_head] = ev;
	key_q_head = next;
}

KEY_STATE Key_Event_Pop(void)			// 메인 루프 전용
{
	KEY_STATE ev;

	if (key_q_tail == key_q_head)
		return KEY_IDLE;

	ev = key_q[key_q_tail];
	key_q_tail = (key_q_tail + 1U) & KEY_Q_MASK;
	return ev;
}
