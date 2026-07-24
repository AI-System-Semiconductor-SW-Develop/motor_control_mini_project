#include "device_driver.h"
#include <stdio.h>


volatile int Key_Pressed = 0;
volatile int TIM4_Expired = 0;
volatile int Key_Released = 0;
volatile int Key_State = 0;

volatile unsigned char Uart_Data = 0;
volatile int Uart_Data_In = 0;
volatile int Duty = 90;

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
    TIM5_Out_Init();
}

enum {Motor_STOP, Motor_CW, Motor_CCW};

int Main(void)
{
    int state = Motor_STOP;
    int long_press_done = 0;

    Sys_Init(115200);

    printf("\nMotor Test\n");

    Uart1_Init(115200);
    Key_ISR_Enable(1);
    Uart2_RX_Interrupt_Enable(1);

    Motor_Control(Motor_STOP);

    for (;;)
    {   
        if (Uart_Data_In)
        {
            Uart_Data_In = 0;

            if('0'<=Uart_Data && Uart_Data <= '9')
            {
                Duty = (Uart_Data - '0')*10;  
                Motor_Control(state);    
            }
        }
        
        if (Key_Pressed)
        {
            Key_Pressed =0;
            TIM4_Expired = 0;
            long_press_done = 0;
            
            TIM4_Repeat_Interrupt_Enable(0,0);
            TIM4_Repeat_Interrupt_Enable(1, 3);

            printf("Key pressed\n");
        }

        if (TIM4_Expired)
        {
            TIM4_Expired = 0;

            /* 아직 버튼을 누르고 있는지 확인 */
            if (Key_State == 1 && long_press_done == 0 )
            {   
                TIM4_Repeat_Interrupt_Enable(0,0);
                state = Motor_STOP;
                Motor_Control(state);

                long_press_done = 1;

                printf("Long press: Motor STOP\n");
            }
        }
        
        if(Key_Released)
         {
            Key_Released = 0;
            TIM4_Expired = 0;
            TIM4_Repeat_Interrupt_Enable(0,0);

            if (long_press_done == 0)
            {
                if (state == Motor_STOP)
                {
                    state = Motor_CW;
                    Motor_Control(state);

                    printf("Motor CW\n");
                }
                else if (state == Motor_CW)
                {
                    state = Motor_STOP;
                    Motor_Control(state);

                    TIM2_Delay(20);

                    state = Motor_CCW;
                    Motor_Control(state);

                    printf("Motor CCW\n");
                }
                else
                {
                    state = Motor_STOP;
                    Motor_Control(state);

                    TIM2_Delay(20);

                    state = Motor_CW;
                    Motor_Control(state);

                    printf("Motor CW\n");
                }
            }

            long_press_done = 0;
        }
    }
}

void Motor_Control(int state)
{  
	switch(state)
	{
		case Motor_CW:
            PORTA_Mode(0);
            TIM5->CCR1 = (TIM5->ARR+1)*Duty/100;
			break;
		case Motor_CCW:
            PORTA_Mode(1);
            TIM5->CCR2 = (TIM5->ARR+1)*Duty/100;
			break;
		case Motor_STOP:
            PORTA_Mode(2);
			break;
	}
}

void PORTA_Mode (int port)
{
    if (port == 0)
    {
        TIM5->CCR2 = 0;
        Macro_Write_Block(GPIOA->MODER, 0x03,0x02,0); // altimate function mode
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,2);
        Macro_Clear_Bit(GPIOA->ODR,1);
    }
    else if (port == 1)
    {
        TIM5->CCR1 = 0;
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,0);
        Macro_Clear_Bit(GPIOA->ODR,0);
        Macro_Write_Block(GPIOA->MODER, 0x03,0x02,2); // altimate function mode
    }
    else if (port == 2)
    {
        TIM5->CCR1 = 0;
        TIM5->CCR2 = 0;
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,0);
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,2);
    }
}
void TIM5_Out_Init(void)
{
    Macro_Set_Bit(RCC->AHB1ENR,0); // porta clock on
	Macro_Set_Bit(RCC->APB1ENR, 3); // timer 5 on
	Macro_Write_Block(TIM5->CCMR1,0xff, 0x60, 0); // 타이머 5 채널 1 cc 설정
	Macro_Write_Block(TIM5->CCMR1,0xff, 0x60, 8); // 타이머 5 채널 2 cc 설정
	Macro_Write_Block(GPIOA->MODER, 0x03,0x02,0); // altimate function mode
	Macro_Write_Block(GPIOA->AFR[0],0xf,0x02,0); // af[02]번과 A0번 매핑 
	Macro_Write_Block(GPIOA->MODER, 0x03,0x02,2); // altimate function mode
	Macro_Write_Block(GPIOA->AFR[0],0xf,0x02,4); // af[02]번과 A1번포트 매핑
	TIM5->CCER = (0<<5)|(1<<4)|(0<<1)|(1<<0); //CC 허용 여부 설정
	TIM5->CR1 = (0<<7)|(0<<4)|(0<<3); // up count, repeat mode
	// Timer 주파수가 TIM3_FREQ가 되도록 PSC 설정
	TIM5->PSC = (TIMXCLK  / 1000000) - 1; 
	// 요청한 주파수가 되도록 ARR 설정
	TIM5->ARR = 1000000/10000 -1;
	// Duty Rate 설정
	TIM5->CCR1 = 0;
	TIM5->CCR2 = 0;
	// Manual Update(UG 발생)
	Macro_Set_Bit(TIM5->EGR,0);
	// timer on
	Macro_Set_Bit(TIM5->CR1,0);
}