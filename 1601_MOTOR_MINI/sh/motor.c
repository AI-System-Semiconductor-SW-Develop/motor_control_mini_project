#include "device_driver.h"
#include <stdio.h>

extern volatile MOTOR_STATE next_state;

void Toggle_Motor_State(MOTOR_STATE *state) {
    switch (*state) {
        case STOP:  // 정지 -> 정회전
            Motor_CW();
            *state = CW;
            break;

        case CW:    // 정회전 -> 역회전
            Motor_Short_Stop();
            *state = SHORT_STOP;
            next_state = CCW;
            break;

        case CCW:   // 역회전 -> 정회전
            Motor_Short_Stop();
            *state = SHORT_STOP;
            next_state = CW;
            break;

        default:    // 예외 처리
            Motor_Stop();
            *state = STOP;
            break;
    }
}

void Toggle_with_Short_Stop(MOTOR_STATE *state, MOTOR_STATE next_state){
    switch(next_state){
        case CW:    // 정회전 -> 역회전
            Motor_CW();
            *state = CW;
            break;

        case CCW:   // 역회전 -> 정회전
            Motor_CCW();
            *state = CCW;
            break;
        
        default:
            break;
    }
}

void Motor_Stop(void){
    GIPO_Mode_Stop();
    Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
}

void Motor_Short_Stop(void){
    GIPO_Mode_Stop();
    Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
    TIM4_Delay_Interrupt_Enable(200);
}

void Motor_CW(void){
    GPIO_Mode_CW();
    Macro_Set_Bit(GPIOA->ODR, 1);       //PA1 OFF
    printf("!!Motor CW!!\n\n");
}

void Motor_CCW(void){
    GPIO_Mode_CCW();
    Macro_Set_Bit(GPIOA->ODR, 0);       //PA0 OFF
    printf("!!CCW Motor CCW!!\n\n");
}