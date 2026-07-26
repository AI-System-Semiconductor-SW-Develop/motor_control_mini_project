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

        case SHORT_STOP:  
            Motor_Stop();
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

void Motor_Stop(){
    GPIO_Mode_Stop();
    TIM5_PWM_Disable();
    Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
}

void Motor_Short_Stop(){
    GPIO_Mode_Stop();
    TIM5_PWM_Disable();
    Macro_Write_Block(GPIOA->ODR, 0x3, 0x0, 0);
    TIM4_Timer_Start(300);
}

void Motor_CW(void){
    GPIO_Mode_CW();
    TIM5_PWM_Enable();
    Macro_Set_Bit(GPIOA->ODR, 1);       //PA1 OFF
}

void Motor_CCW(void){
    GPIO_Mode_CCW();
    TIM5_PWM_Enable();
    Macro_Set_Bit(GPIOA->ODR, 0);       //PA0 OFF
}