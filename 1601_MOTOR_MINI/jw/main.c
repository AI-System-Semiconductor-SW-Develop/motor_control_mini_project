#include "device_driver.h"
#include <stdio.h>

/* Function Declaration */ 
static void Sys_Init(int baud) ;

/* Global Variable Definition */
volatile KEY_STATE key_state          = KEY_IDLE;		//Key Press/Release INT flag
volatile MOTOR_SPEED curr_motor_speed = SPD_DEFAULT;
volatile MOTOR_STATE next_state       = 0U;

volatile int is_timer2_timeout = 0U;			//TIM2 Expired INT flag
volatile int is_timer4_timeout = 0U;			//TIM4 Expired INT flag
volatile int is_uart_data_in   = 0U;			//UART INT flag

void Main(void)
{
	/* 초기화 */
	Sys_Init(BAUD_RATE);
	GPIO_Init();
	TIM5_Out_Init();					//PWM Enable
	TIM5_PWM_Init(curr_motor_speed);

	printf("\nMotor_Test_REV1.2\n");
	
	/* Variable Definition */
	MOTOR_STATE motor_state   = STOP;   // 모터 상태 정지로 초기화
	uint8_t is_timer2_started = 0U; 	// 타이머가 현재 동작 중인지, 1: 동작 중(타임아웃 이전)
	
	/* Interrupt Enable */
	Key_ISR_Enable();					// Key Rising/Falling Edge INT
	Uart2_RX_Interrupt_Enable();		// Uart RX INT Enable

	for (;;) 
	{
		// Falling Edge Detected
		if (key_state == IS_PRESSED) 
		{
			key_state = KEY_IDLE;                  

			TIM2_Timer_Start(LONG_PRESS_MSEC);
			is_timer2_started = 1U;
		}

		// Rising Edge Detected
		else if (key_state == IS_RELEASED) 
		{
			key_state = KEY_IDLE;                  

			if (is_timer2_started)
			{			
				TIM2_Stop();
				Toggle_Motor_State(&motor_state);
				is_timer2_started = 0U;
			}
		}

		// Timer2 timeout - 모터 초기화
		if (is_timer2_timeout) 
		{
			is_timer2_timeout = 0U;               
			is_timer2_started = 0U;

			Motor_Stop();
			motor_state = STOP;
		}

		// Timer4 timeout - short stop 종료
		if (is_timer4_timeout)
		{
			is_timer4_timeout = 0U;              

			Toggle_with_Short_Stop(&motor_state, next_state);
		}

		//UART2 RX Interrupt
		if (is_uart_data_in)
		{
			is_uart_data_in = 0U;                 

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