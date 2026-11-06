#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud)
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2);
	Clock_Init();
	Uart2_Init(baud);
	Uart1_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

/*
 * REV 1.1-fix
 *  - 버튼 짧게 누름: STOP → CW, CW/CCW → 300ms 정지 후 반대 방향
 *  - 버튼 3초 길게 누름: 떼지 않아도 어느 상태에서든 STOP (방향 전환 대기도 취소)
 *  - UART '1'~'9': PWM 듀티 50~90% (5% 단위)
 *  - ISR은 이벤트/값만 저장, 상태 전환과 로그 출력은 메인 루프에서 처리
 *  (이전 리비전 코드는 git 기록 참고)
 */

/* ISR ↔ 메인 공유 변수 */
volatile int TIM2_Expired = 0;			// 3초 길게 누름 만료
volatile int TIM4_Expired = 0;			// 방향 전환 대기 만료
volatile int Uart_Data_In = 0;			// 유효한 속도 명령 수신
volatile int Uart_Bad_Input = 0;		// 잘못된 문자 수신
volatile int Uart_Overrun = 0;			// UART 오버런 발생
volatile MOTOR_SPEED curr_motor_speed = SPD_5;
volatile MOTOR_STATE next_state = STOP;	// SHORT_STOP 이후 진행할 방향

extern volatile int Key_Q_Overflow;

/* 플래그를 읽고 지우는 사이에 ISR이 끼어들어 이벤트를 덮지 않도록 원자적으로 처리 */
static int take_flag(volatile int *flag)
{
	int v;
	__disable_irq();
	v = *flag;
	*flag = 0;
	__enable_irq();
	return v;
}

void Main(void)
{
	/* 초기화 */
	Sys_Init(115200);
	GPIO_Init();
	TIM5_PWM_Init(curr_motor_speed);	// PWM은 계속 동작, 핀 연결은 GPIO 모드로 결정
	printf("\nMotor_test_rev1.1-fix\n");

	MOTOR_STATE motor_state = STOP;
	int is_timer_started = 0;			// 1: 3초 타이머 동작 중(아직 만료 전)

	/* Interrupt Enable */
	Key_ISR_Enable(1);					// PC13 Falling/Rising EXTI
	Uart2_RX_Interrupt_Enable();		// USART2 RX interrupt
	// TIM2, TIM4 인터럽트는 각 타이머 시작 함수에서 허용

	for (;;) {

		/* ① 3초 만료를 버튼 뗌보다 먼저 처리
		 *    → 정확히 3초 무렵에 뗐을 때 방향 전환과 정지가 함께 실행되지 않음 */
		if (take_flag(&TIM2_Expired)) {
			is_timer_started = 0;
			Motor_Stop();				// TIM4(방향 전환 대기)도 함께 취소
			motor_state = STOP;
			printf("long press: STOP\n");
		}

		/* ② 버튼 이벤트는 큐에서 순서대로 꺼냄 (짧은 누름 유실 방지) */
		KEY_STATE key;
		while ((key = Key_Event_Pop()) != KEY_IDLE) {
			if (key == IS_PRESSED) {
				TIM2_3sec_Timer_Start();
				is_timer_started = 1;
			}
			else if (key == IS_RELEASED && is_timer_started) {
				TIM2_Stop();			// 정지 직전 세워진 만료 플래그까지 제거
				is_timer_started = 0;
				Toggle_Motor_State(&motor_state);	// SHORT_STOP 중이면 무시
			}
		}

		/* ③ 방향 전환 대기 만료: SHORT_STOP 상태일 때만 재개 */
		if (take_flag(&TIM4_Expired))
			Toggle_with_Short_Stop(&motor_state, next_state);

		/* ④ 속도 변경: CCR만 갱신 */
		if (take_flag(&Uart_Data_In)) {
			TIM5_Set_Duty(curr_motor_speed);
			printf("motor speed: %d%%\n", (int)curr_motor_speed);
		}

		/* ⑤ 경고·오류 로그 (ISR 밖에서 출력) */
		if (take_flag(&Uart_Bad_Input))	printf("SPEED SET WRONG: use 1~9\n");
		if (take_flag(&Uart_Overrun))	printf("UART overrun\n");
		if (take_flag(&Key_Q_Overflow))	printf("key event queue overflow\n");
	}
}
