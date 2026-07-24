#include "device_driver.h"
#include <stdio.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);

	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

/* Global Variable Definition */
volatile KEY_STATE key_state = KEY_IDLE;		//Key Press/Release INT flag

volatile int TIM2_Expired = 0U;			//TIM2 Expired INT flag
volatile int TIM4_Expired = 0U;			//TIM4 Expired INT flag

volatile int Uart_Data_In = 0U;			//UART INT flag

volatile MOTOR_SPEED curr_motor_speed = SPD_DEFAULT;
volatile MOTOR_STATE next_state       = STOP;

void Main(void)
{
	/* 초기화 */
	Sys_Init(115200);
	GPIO_Init();
	TIM5_Out_Init();					//PWM Enable
	printf("\nMotor_test_rev1.1\n");
			
	/* Variable Definition */
	MOTOR_STATE motor_state = STOP;		// 모터 상태 정지로 초기화
	int is_timer2_running = 0; 			// 타이머가 현재 동작 중인지, 1: 동작중(타임아웃 이전)

	/* Interrupt Enable */
	Key_ISR_Enable(1);					//Key Rising/Fallind Edge INT
	Uart2_RX_Interrupt_Enable();		//Uart RX INT Enable
	//TIM2, TIM4는 타이머 실행 함수에 INT를 함께 허용해놓아서 다로 Enable함수를 호춣하지 않음.

	TIM5_PWM_Generation(curr_motor_speed);	//PWM 기본 duty 설정: 70
	
	for (;;) {

		//Falling Edge INT
		if (key_state == IS_PRESSED) {
			TIM2_3sec_Timer_Start();		// 3초 타이머 시작
			printf("button pressed\n");
			
			is_timer2_running = 1;
			key_state = KEY_IDLE;
		}
		//Rising Edge INT
		else if (key_state == IS_RELEASED) {
			if (is_timer2_running){			//타임아웃 X, 타이머 동작중
				TIM2_Stop();
				Toggle_Motor_State(&motor_state);

				is_timer2_running = 0;
			}

			key_state = KEY_IDLE;
			printf("button released\n");
		}

		//Timer Expired
		if (TIM2_Expired) {
			Motor_Stop();
			TIM2_Stop();
			printf("is timeout\n");
			
			is_timer2_running = 0;
			motor_state = STOP;

			TIM2_Expired = 0;
		}

		if (TIM4_Expired){
			Toggle_with_Short_Stop(&motor_state, next_state);

			TIM4_Expired = 0;
		}
		
		//UART2 RX Interrupt
		if (Uart_Data_In){
			TIM5_PWM_Generation(curr_motor_speed);
			
			Uart_Data_In = 0;
		}
	}
}
