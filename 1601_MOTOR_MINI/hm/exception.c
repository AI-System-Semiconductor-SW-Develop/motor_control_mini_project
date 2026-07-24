#include "device_driver.h"
#include <stdio.h>


void _Invalid_ISR(void)
{
	unsigned int r = Macro_Extract_Area(SCB->ICSR, 0x1ff, 0);
	printf("\nInvalid_Exception: %d!\n", r);
	printf("Invalid_ISR: %d!\n", r - 16);
	for(;;);
}

extern volatile KEY_STATE key_state;

void EXTI15_10_IRQHandler(void)     //Key Falling/Rising EXTI
{
    printf("KEY INTERRUPT !!!\n");
    if(EXTI->PR && (0x1 <<PC13)){      //PC13 pending 여부 확인
        if(Key_Get_Pressed()){
            key_state = IS_PRESSED;    //falling
        }
        else key_state = IS_RELEASED;   //rising
        
        EXTI->PR = 0x1 << PC13;             //PC13 Pending Clear
        NVIC_ClearPendingIRQ(EXTI15_10);   //NVIC Pending Clear        
    }
}

extern volatile int TIM2_Expired;

void TIM2_IRQHandler(void)      //TIM2 Expired INT
{
    printf("TIMER2 Expired !!!\n");
    TIM2_Expired = 1;

    Macro_Clear_Bit(TIM2->SR, 0);           // TIM2 Interrupt Pending Clear
	NVIC_ClearPendingIRQ(TIM2_INT_POS);     // NVIC Pending Clear
}

extern volatile int TIM4_Expired;

void TIM4_IRQHandler(void)
{
    printf("TIMER4 Expired !!!\n");
    TIM4_Expired = 1;

    Macro_Clear_Bit(TIM4->SR, 0);           // TIM4 Interrupt Pending Clear
	NVIC_ClearPendingIRQ(TIM4_INT_POS);     // NVIC Pending Clear
}

extern volatile int Uart_Data_In;
extern volatile MOTOR_SPEED curr_motor_speed;
const MOTOR_SPEED speed_table[10] = {
	SPD_1, SPD_2, SPD_3, SPD_4, 
    SPD_5, SPD_6, SPD_7, SPD_8, SPD_9
};


void USART2_IRQHandler(void)
{
    Uart_Data_In = 1;

    char rx_table_num = USART2->DR;

    if (rx_table_num > '0' && rx_table_num <= '9') 
    {
        int index = rx_table_num - '0' - 1;
        curr_motor_speed = speed_table[index];
    }
    else{
        printf("SPEED SET WRONG !!!\n");
    }

    printf("motor speed: %d\n", curr_motor_speed);
    NVIC_ClearPendingIRQ((IRQn_Type)38);    // NVIC Pending Clear

}