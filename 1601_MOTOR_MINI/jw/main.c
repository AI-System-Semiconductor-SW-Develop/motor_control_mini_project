#include "device_driver.h"
#include <stdio.h>

/* Function Declaration */ 
static void Sys_Init(int baud) ;

/* Global Variable Definition */
volatile KEY_STATE key_state          = KEY_IDLE;		//Key Press/Release INT flag
volatile MOTOR_SPEED curr_motor_speed = SPD_DEFAULT;
volatile MOTOR_STATE next_state       = 0;

volatile int is_timer2_timeout = 0U;			//TIM2 Expired INT flag
volatile int is_timer4_timeout = 0U;			//TIM4 Expired INT flag
volatile int is_uart_data_in = 0U;			//UART INT flag

void Main(void)
{
	/* 초기화 */
	Sys_Init(115200);
	GPIO_Init();
	TIM5_Out_Init();					//PWM Enable
	TIM5_PWM_Init(curr_motor_speed);

	printf("\nMotor_Test_REV1.2\n");
	
	/* Variable Definition */
	MOTOR_STATE motor_state = STOP;		// 모터 상태 정지로 초기화
	uint8_t is_timer2_started = 0; 			// 타이머가 현재 동작 중인지, 1: 동작중(타임아웃 이전)
	
	/* Interrupt Enable */
	Key_ISR_Enable();					//Key Rising/Fallind Edge INT
	Uart2_RX_Interrupt_Enable();		//Uart RX INT Enable

	for (;;) {
		//Falling Edge INT
		if (key_state == IS_PRESSED) {
			key_state = KEY_IDLE;
			printf("button pressed\n");
			
			TIM2_Timer_Start(3000);		// 3초 타이머 시작
			is_timer2_started = 1;
		}
		//Rising Edge INT
		else if (key_state == IS_RELEASED) {
			key_state = KEY_IDLE;
			printf("button released\n");
			
			if (is_timer2_started){			//타임아웃 X, 타이머 동작중
				TIM2_Stop();
				is_timer2_started = 0;
				
				Toggle_Motor_State(&motor_state);
			}
		}
		
		//Timer Expired
		if (is_timer2_timeout) {
			is_timer2_timeout = 0;
			is_timer2_started = 0;
			
			Motor_Stop();
			motor_state = STOP;
		}
		
		if (is_timer4_timeout){
			is_timer4_timeout = 0;
			
			Toggle_with_Short_Stop(&motor_state, next_state);
		}
		
		//UART2 RX Interrupt
		if (is_uart_data_in){
			is_uart_data_in = 0;
			
			TIM5_PWM_Generation(curr_motor_speed);
		}
	}
}

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	LED_Init();
	setvbuf(stdout, NULL, _IONBF, 0);
}