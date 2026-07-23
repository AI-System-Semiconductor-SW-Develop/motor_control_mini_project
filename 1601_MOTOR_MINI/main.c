#include "device_driver.h"
#include <stdio.h>

void button_init (void);
extern void TIM2_3s_Wait(void);
int TIM2_Check_Timeout(void);
void Motor_Control(int state);

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

enum {Motor_STOP, Motor_CW, Motor_CCW};

int Main(void)
{
    int is_timer_started = 0;
    int end_flag = 0;
    int state = Motor_STOP;
    
    // 버튼의 이전 상태를 저장할 변수 (초기값 1: 떼어진 상태 가정)
    int prev_button_state = 1; 
    int current_button_state = 1;

    Sys_Init(115200);
    printf("\nMotor Test\n");

    Uart1_Init(115200);
    button_init();
	TIM5_Out_Init();
    
    state = Motor_STOP;
    Motor_Control(state);
    
    for(;;)
    {   
        // 1. 현재 버튼 상태 읽기 (버튼 안 눌림=1(Set), 눌림=0(Clear) 가정)
        if (Macro_Check_Bit_Set(GPIOC->IDR, 7))
		{
            current_button_state = 1;
        } 
		else 
		{
            current_button_state = 0;
        }

        // 2. 버튼이 막 '눌려진 순간' 감지 (Falling Edge)
        if (prev_button_state == 1 && current_button_state == 0)
        {
            if (is_timer_started == 0) 
			{
                TIM2_3s_Wait();
                is_timer_started = 1;
                end_flag = 0;
            }
        }
        
        // 3. 버튼이 막 '떼어진 순간' 감지 (Rising Edge)
        if (prev_button_state == 0 && current_button_state == 1)
        {
            // 3초가 지나기 전에 버튼을 뗐다면 모터 방향 전환 진행
            if (is_timer_started == 1) 
			{ 
                is_timer_started = 0;
                end_flag = 1;
            }
        }

        // 4. 이전 버튼 상태 업데이트
        prev_button_state = current_button_state;

        // 5. 3초 타이머 타임아웃 처리
        if(TIM2_Check_Timeout() && is_timer_started == 1)
        {
            is_timer_started = 0;
            state = Motor_STOP;
            Motor_Control(state);
        }

        // 6. 모터 상태 전환 로직
        if (end_flag == 1)
        {
            if(state == Motor_STOP)
            {
                state = Motor_CW;
                Motor_Control(state);
            }
            else if(state == Motor_CCW)
            {
                state = Motor_STOP;
                Motor_Control(state);
                TIM2_Delay(20);
                state = Motor_CW;
                Motor_Control(state);
            }
            else if(state == Motor_CW)
            {
                state = Motor_STOP;
                Motor_Control(state);
                TIM2_Delay(20);
                state = Motor_CCW;
                Motor_Control(state);
            }
            end_flag = 0; // 처리가 끝났으므로 플래그 초기화
        }
    }
}


void Motor_Control(int state)
{
    char key;
    key = Uart2_Get_Pressed();
    Uart2_Send_Byte(key);
    
	switch(state)
	{
		case Motor_CW:
			TIM5->CCR1 = TIM5->ARR*(key -'0')/10;
			PORTA_Mode(1);
			break;
		case Motor_CCW:
			PORTA_Mode(0);
			TIM5->CCR2 = TIM5->ARR*(key -'0')/10;
			break;
		case Motor_STOP:
			PORTA_Mode(0);
            PORTA_Mode(1);
			break;
	}
}
void button_init (void)
{
	// C Port Clock on
	Macro_Set_Bit(RCC->AHB1ENR , 2);
	
	// PC7 input mode 설정
	Macro_Write_Block(GPIOC->MODER,0x03,0x00,14);

	// 내부 pull up on
	Macro_Write_Block(GPIOC->PUPDR,0x03,0x01,14);
}

void PORTA_Mode(int a)
{
    if (a == 0)
    {
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,0);   
        Macro_Clear_Bit(GPIOA->ODR,0);
    }
    if (a == 1)
    {
        Macro_Write_Block(GPIOA->MODER,0x03,0x01,2);   
        Macro_Clear_Bit(GPIOA->ODR,1);
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