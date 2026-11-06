#include "device_driver.h"
#include <stdio.h>

/*
 * ISR 원칙: 입력을 읽고, 이벤트/값을 저장하고, Pending을 지운다.
 *           printf(UART 폴링 송신)나 상태 전환은 메인 루프에서 수행한다.
 */

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for (;;);		// 복구 불가 예외: 여기서는 출력 후 정지
}

/* ---------- EXTI13 (PC13 버튼, 누름·뗌 양쪽 엣지) ---------- */

void EXTI15_10_IRQHandler(void)
{
	if (EXTI->PR & (1U << PC13)) {			// && 가 아닌 & (비트 검사)
		EXTI->PR = 1U << PC13;				// 먼저 Pending clear → 처리 중 들어온 엣지는 다시 잡힘
		Key_Event_Push(Key_Get_Pressed() ? IS_PRESSED : IS_RELEASED);
	}
}

/* ---------- TIM2: 3초 길게 누름 만료 ---------- */

extern volatile int TIM2_Expired;

void TIM2_IRQHandler(void)
{
	Macro_Clear_Bit(TIM2->SR, 0);			// UIF clear
	TIM2_Expired = 1;
}

/* ---------- TIM4: 방향 전환 대기 만료 ---------- */

extern volatile int TIM4_Expired;

void TIM4_IRQHandler(void)
{
	Macro_Clear_Bit(TIM4->SR, 0);			// UIF clear
	TIM4_Expired = 1;
}

/* ---------- USART2 RX: 속도 명령 '1'~'9' ---------- */

extern volatile int Uart_Data_In;
extern volatile int Uart_Bad_Input;
extern volatile int Uart_Overrun;
extern volatile MOTOR_SPEED curr_motor_speed;

const MOTOR_SPEED speed_table[SPEED_TABLE_SIZE] = {
	SPD_1, SPD_2, SPD_3, SPD_4,
	SPD_5, SPD_6, SPD_7, SPD_8, SPD_9
};

void USART2_IRQHandler(void)
{
	unsigned int sr = USART2->SR;			// SR → DR 순서로 읽어야 RXNE와 ORE가 함께 해제됨
	char rx = (char)USART2->DR;

	if (sr & USART_SR_ORE)
		Uart_Overrun = 1;					// 처리 지연으로 앞 바이트를 놓쳤음을 기록

	if (rx >= '1' && rx <= '9') {
		curr_motor_speed = speed_table[rx - '1'];
		Uart_Data_In = 1;					// 유효한 명령일 때만 메인에 통지
	}
	else if (rx != '\r' && rx != '\n') {
		Uart_Bad_Input = 1;					// 경고 출력은 메인 루프에서
	}
}
