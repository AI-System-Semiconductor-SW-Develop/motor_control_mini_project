/*
* REV1.2 수정사항
* Magic number 대신 매크로 사용
* 인터럽트 레지스터 매핑 값을 CMSIS 헤더(stm32f411xe.h)에 정의된 이름으로 변경
*/

/*
모터 방향 버튼 제어
버튼 상태에 관계 없이 3초 이상 누르면 정지
모터 속도 PWM, UART 제어
이벤트 처리에 인터럽트 사용
	- 루프에 갇히지 않도록 설계하여 실시간성 확보
*/


#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	LED_Init();
	setvbuf(stdout, NULL, _IONBF, 0);
}

/* Global Variable Definition */
volatile KEY_STATE key_state = KEY_IDLE;		//Key Press/Release INT flag

volatile int TIM2_Expired = 0U;			//TIM2 Expired INT flag
volatile int TIM4_Expired = 0U;			//TIM4 Expired INT flag

volatile int Uart_Data_In = 0U;			//UART INT flag
volatile MOTOR_SPEED curr_motor_speed = SPD_DEFAULT;

volatile MOTOR_STATE next_state = 0;


void Main(void)
{
	/* 초기화 */
	Sys_Init(115200);
	GPIO_Init();
	TIM5_Out_Init();					//PWM Enable
	printf("\nMotor_Test_REV1.2\n");
			
	/* Variable Definition */
	MOTOR_STATE motor_state = STOP;		// 모터 상태 정지로 초기화
	int is_timer_started = 0; 			// 타이머가 현재 동작 중인지, 1: 동작중(타임아웃 이전)

	/* Interrupt Enable */
	Key_ISR_Enable();					//Key Rising/Fallind Edge INT
	Uart2_RX_Interrupt_Enable();		//Uart RX INT Enable
	//TIM2, TIM4는 타이머 실행 함수에 INT를 함께 허용해놓아서 다로 Enable함수를 호춣하지 않음.

	TIM5_PWM_Generation(curr_motor_speed);	//PWM 기본 duty 설정: 70
	
	for (;;) {

		//Falling Edge INT
		if (key_state == IS_PRESSED) {
			key_state = KEY_IDLE;
			printf("button pressed\n");
			
			TIM2_3sec_Timer_Start();		// 3초 타이머 시작
			is_timer_started = 1;
		}
		//Rising Edge INT
		else if (key_state == IS_RELEASED) {
			key_state = KEY_IDLE;
			printf("button released\n");
			
			if (is_timer_started){			//타임아웃 X, 타이머 동작중
				TIM2_Stop();
				is_timer_started = 0;

				Toggle_Motor_State(&motor_state);
			}
		}

		//Timer Expired
		if (TIM2_Expired) {
			printf("is timeout\n");

			TIM2_Expired = 0;
			is_timer_started = 0;

			Motor_Stop();
			motor_state = STOP;
		}

		if (TIM4_Expired){
			TIM4_Expired = 0;

			Toggle_with_Short_Stop(&motor_state, next_state);
		}
		
		//UART2 RX Interrupt
		if (Uart_Data_In){
			Uart_Data_In = 0;

			TIM5_PWM_Generation(curr_motor_speed);
		}
	}
}