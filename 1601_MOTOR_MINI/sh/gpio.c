#include "device_driver.h"
#include <stdio.h>

void GPIO_Init(void){
    Macro_Set_Bit(RCC->AHB1ENR, 0);	//GPIOA En
	Macro_Set_Bit(RCC->AHB1ENR, 3);	//GPIOC En

    Macro_Write_Block(GPIOA->MODER,0xf, 0x5, 0);	//PA0, PA1 General output
    Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, 0);	//PA0, PA1 Push-pull
    
	Macro_Write_Block(GPIOC->MODER,0x3, 0x0, 26);	//key mode input
}

void GPIO_Mode_CW(void){
    Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 0);       // PA0 Alternate funtion mode
    Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 0);      // PA0 주변장치 매핑
    Macro_Write_Block(GPIOA->PUPDR, 0x3, 0x1, 0);       // PA0 Pull-up

    Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 2);       // PA1 General Output mode
    Macro_Clear_Bit(GPIOA->OTYPER, 1);                  // PA1 Output push-pull
}

void GPIO_Mode_CCW(void){
    Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 0);       // PA0 General Output mode
    Macro_Clear_Bit(GPIOA->OTYPER, 0);                  // PA0 Output push-pull

    Macro_Write_Block(GPIOA->MODER, 0x3, 0x2, 2);       // PA1 Alternate funtion mode
    Macro_Write_Block(GPIOA->AFR[0], 0xf, 0x2, 4);      // PA1 주변장치 매핑
    Macro_Write_Block(GPIOA->PUPDR, 0x3, 0x1, 2);       // PA1 Pull-up
}

void GIPO_Mode_Stop(void){
    Macro_Write_Block(GPIOA->MODER,0xf, 0x5, 0);	//PA0, PA1 General output
    Macro_Write_Block(GPIOA->OTYPER, 0x3, 0x0, 0);	//PA0, PA1 Push-pull
}