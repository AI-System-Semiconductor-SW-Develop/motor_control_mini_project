#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

#define PC13			13U

typedef enum t_MOTOR_STATE
{
	STOP = 0U,
	CW = 1U,
	CCW = 2U,
	SHORT_STOP = 3u
} MOTOR_STATE;

typedef enum t_KEY_STATE
{
	KEY_IDLE = 0U,		// IDLE
	IS_PRESSED = 1U,	// Falling
	IS_RELEASED = 2U	// Rising
} KEY_STATE;

typedef enum t_MOTOR_SPEED{
    SPD_1 = 50,
    SPD_2 = 55,
    SPD_3 = 60,
    SPD_4 = 65,
    SPD_5 = 70,
    SPD_6 = 75,
    SPD_7 = 80,
    SPD_8 = 85,
    SPD_9 = 90,
    SPD_DEFAULT = 100
} MOTOR_SPEED;

extern const MOTOR_SPEED speed_table[10];

// GPIO.c
extern void GPIO_Init(void);
extern void GPIO_Mode_CW(void);
extern void GPIO_Mode_CCW(void);
extern void GPIO_Mode_Stop(void);

// Uart.c

extern void Uart2_Init(int baud);
extern void Uart2_RX_Interrupt_Enable(void);
extern void Uart2_Send_Byte(char data);

// Clock.c
extern void Clock_Init(void);

// Key.c
extern int Key_Get_Pressed(void);
extern void Key_ISR_Enable(void);

// Timer.c
extern void TIM2_Timer_Start(int msec);
extern int TIM2_Check_Timeout(void);
extern void TIM2_Stop(void);

extern void TIM4_Timer_Start(int time);

extern void TIM5_Out_Init(void);
extern void TIM5_PWM_Init(int duty);
extern void TIM5_PWM_Generation(int duty);
extern void TIM5_PWM_Enable(void);
extern void TIM5_PWM_Disable(void);

// Motor.c
extern void Toggle_Motor_State(MOTOR_STATE *state);
extern void Toggle_with_Short_Stop(MOTOR_STATE *state, MOTOR_STATE next_state);
extern void Motor_Stop(void);
extern void Motor_Short_Stop(void);
extern void Motor_CW(void);
extern void Motor_CCW(void);

// Led.c
extern void LED_Init(void);
extern void LED_On(void);
extern void LED_Off(void);

