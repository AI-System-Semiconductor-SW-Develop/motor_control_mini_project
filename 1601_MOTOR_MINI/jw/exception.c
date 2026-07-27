#include "device_driver.h"
#include <stdio.h>

#define SPEED_TABLE_SIZE (sizeof(speed_table)/sizeof(speed_table[0]))

void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

/* Key Falling/Rising ISR */
extern volatile KEY_STATE key_state;

void EXTI15_10_IRQHandler(void)     // Key Falling/Rising EXTI
{
    //printf("KEY INTERRUPT !!!\n");
    if((EXTI->PR & (0x1 << PORTC_BTN_PIN)) != 0)
    {      //PC13 pending 여부 확인
        if(Key_Get_Pressed()){
            key_state = IS_PRESSED;    //falling
        }
        else key_state = IS_RELEASED;   //rising
        
        EXTI->PR = (0x1 << PORTC_BTN_PIN);             //PC13 Pending Clear
        NVIC_ClearPendingIRQ(EXTI15_10_IRQn);   //NVIC Pending Clear        
    }
}

/* Timer2 timeout ISR */
extern volatile int is_timer2_timeout;

void TIM2_IRQHandler(void) 
{
    is_timer2_timeout = 1U;

    Macro_Clear_Bit(TIM2->SR, UIF_BIT);  // TIM2 Interrupt Pending Clear
	NVIC_ClearPendingIRQ(TIM2_IRQn);     // NVIC Pending Clear
}

/* Timer4 timeout ISR */
extern volatile int is_timer4_timeout;

void TIM4_IRQHandler(void)
{
    is_timer4_timeout = 1U;

    Macro_Clear_Bit(TIM4->SR, UIF_BIT);  // TIM4 Interrupt Pending Clear
	NVIC_ClearPendingIRQ(TIM4_IRQn);     // NVIC Pending Clear
}

/* Uart Rx ISR */
extern volatile int is_uart_data_in;
extern volatile MOTOR_SPEED curr_motor_speed;
const MOTOR_SPEED speed_table[SPEED_TABLE_SIZE] = 
{
	SPD_1, SPD_2, SPD_3, SPD_4, SPD_5,
     SPD_6, SPD_7, SPD_8, SPD_9, SPD_DEFAULT
};

void USART2_IRQHandler(void)
{
    char rx_table_num = USART2->DR;

    if (rx_table_num >= '1' && rx_table_num <= '9') 
    {
        uint8_t index = (uint8_t)rx_table_num - '1';

        if (index < SPEED_TABLE_SIZE) 
        {
            curr_motor_speed = speed_table[index];
            is_uart_data_in = 1U;
        }
    }

    NVIC_ClearPendingIRQ(USART2_IRQn);    // NVIC Pending Clear
}