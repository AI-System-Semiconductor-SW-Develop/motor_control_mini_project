#include "device_driver.h"
#include <stdio.h>

extern volatile MOTOR_STATE next_state;

/* 짧은 누름 처리: STOP → CW, CW/CCW → SHORT_STOP(대기 후 반대 방향) */
void Toggle_Motor_State(MOTOR_STATE *state)
{
	switch (*state) {
	case STOP:			// 정지 -> 정회전
		Motor_CW();
		*state = CW;
		break;

	case CW:			// 정회전 -> (대기) -> 역회전
		next_state = CCW;
		Motor_Short_Stop();
		*state = SHORT_STOP;
		break;

	case CCW:			// 역회전 -> (대기) -> 정회전
		next_state = CW;
		Motor_Short_Stop();
		*state = SHORT_STOP;
		break;

	case SHORT_STOP:	// 방향 전환 대기 중 재입력은 무시
		break;

	default:			// 알 수 없는 상태: 안전하게 정지
		Motor_Stop();
		*state = STOP;
		break;
	}
}

/* TIM4 만료 처리: SHORT_STOP 상태일 때만 예약된 방향으로 재개 */
void Toggle_with_Short_Stop(MOTOR_STATE *state, MOTOR_STATE target)
{
	if (*state != SHORT_STOP)		// 그 사이 정지됐다면 재개하지 않음
		return;

	switch (target) {
	case CW:
		Motor_CW();
		*state = CW;
		break;

	case CCW:
		Motor_CCW();
		*state = CCW;
		break;

	default:
		Motor_Stop();
		*state = STOP;
		break;
	}
}

/* 완전 정지: 출력 차단 + 진행 중인 방향 전환 대기(TIM4)까지 취소 */
void Motor_Stop(void)
{
	TIM4_Stop();
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
	GPIO_Mode_Stop();
}

/* 방향 전환 전 정지: 출력 차단 후 TIM4로 대기 시간 측정 */
void Motor_Short_Stop(void)
{
	Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
	GPIO_Mode_Stop();
	TIM4_Delay_Interrupt_Enable(SHORT_STOP_MS);
}

void Motor_CW(void)
{
	GPIO_Mode_CW();						// PA0 = PWM
	Macro_Set_Bit(GPIOA->ODR, 1);		// PA1 = High (드라이버 진리표 확인 필요)
	printf("Motor CW\n");
}

void Motor_CCW(void)
{
	GPIO_Mode_CCW();					// PA1 = PWM
	Macro_Set_Bit(GPIOA->ODR, 0);		// PA0 = High (드라이버 진리표 확인 필요)
	printf("Motor CCW\n");
}
