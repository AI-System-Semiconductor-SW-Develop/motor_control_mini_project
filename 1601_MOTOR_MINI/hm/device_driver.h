#include "stm32f4xx.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

#define PC13			13U
#define	EXTI15_10		(IRQn_Type)40U

#define TIM2_INT_POS	(IRQn_Type)28U
#define TIM4_INT_POS	(IRQn_Type)30U

#define LONG_PRESS_MS	(3000U)		// 길게 누름 판정 시간
#define SHORT_STOP_MS	(300U)		// 방향 전환 전 정지 시간
#define SPEED_TABLE_SIZE	(9U)

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
    SPD_9 = 90
} MOTOR_SPEED;

extern const MOTOR_SPEED speed_table[SPEED_TABLE_SIZE];

//gpio.c

extern void GPIO_Init(void);
extern void GPIO_Mode_CW(void);
extern void GPIO_Mode_CCW(void);
extern void GPIO_Mode_Stop(void);

// Uart.c

extern void Uart2_Init(int baud);
extern void Uart2_RX_Interrupt_Enable(void);
extern void Uart2_Send_Byte(char data);

extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt, ...);
extern char Uart1_Get_Char(void);
extern char Uart1_Get_Pressed(void);

// SysTick.c

extern void SysTick_Run(unsigned int msec);
extern int SysTick_Check_Timeout(void);
extern unsigned int SysTick_Get_Time(void);
extern unsigned int SysTick_Get_Load_Time(void);
extern void SysTick_Stop(void);

// Led.c

extern void LED_Init(void);
extern void LED_On(void);
extern void LED_Off(void);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Poll_Init(void);
extern int Key_Get_Pressed(void);
extern void Key_Wait_Key_Released(void);
extern void Key_Wait_Key_Pressed(void);
extern void Key_ISR_Enable(int en);
extern void Key_Event_Push(KEY_STATE ev);
extern KEY_STATE Key_Event_Pop(void);

// Timer.c
extern void TIM2_3sec_Timer_Start(void);
extern int TIM2_Check_Timeout(void);
extern void TIM2_Stop(void);

extern void TIM4_Delay(int time);
extern void TIM4_Delay_Interrupt_Enable(unsigned int msec);
extern void TIM4_Stop(void);

extern void TIM5_PWM_Init(int duty);
extern void TIM5_Set_Duty(int duty);
#if 0
extern void TIM2_Stopwatch_Start(void);
extern unsigned int TIM2_Stopwatch_Stop(void);
extern void TIM2_Delay(int time);
extern void TIM4_Repeat(int time);
extern int TIM4_Check_Timeout(void);
extern void TIM4_Stop(void);
extern void TIM4_Change_Value(int time);
#endif

// Motor.c

extern void Toggle_Motor_State(MOTOR_STATE *state);
extern void Toggle_with_Short_Stop(MOTOR_STATE *state, MOTOR_STATE target);
extern void Motor_Stop(void);
extern void Motor_Short_Stop(void);
extern void Motor_CW(void);
extern void Motor_CCW(void);
