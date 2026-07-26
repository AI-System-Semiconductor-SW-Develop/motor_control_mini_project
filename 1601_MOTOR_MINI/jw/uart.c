#include "device_driver.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "uart.h"

void Uart2_Init(int baud)
{
  double div;
  unsigned int mant;
  unsigned int frac;

  Macro_Set_Bit(RCC->AHB1ENR, GPIOA_EN_BIT);        // PA2,3
  Macro_Set_Bit(RCC->APB1ENR, USART_EN_BIT);        // USART2 ON
  Macro_Write_Block(GPIOA->MODER, 0xf, 0xa, 4);     // PA2,3 => ALT
  Macro_Write_Block(GPIOA->AFR[0], 0xff, 0x77, 8);  // PA2,3 => AF07
  Macro_Write_Block(GPIOA->PUPDR, 0xf, 0x5, 4);     // PA2,3 => Pull-Up  

  volatile unsigned int t = GPIOA->LCKR & 0x7FFF;
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<2);                // Lock PA2, 3 Configuration
  GPIOA->LCKR = (0x0<<16)|t|(0x3<<2);
  GPIOA->LCKR = (0x1<<16)|t|(0x3<<2);
  t = GPIOA->LCKR;

  div = PCLK1/(16. * baud);
  mant = (int)div;
  frac = (int)((div - mant) * 16. + 0.5);
  mant += frac >> 4;
  frac &= 0xf;

  USART2->BRR = (mant<<4)|(frac<<0);
  USART2->CR1 = (1<<UE_BIT)|(0<<M_BIT)|(0<<PCE_BIT)|(1<<TE_BIT)|(1<<RE_BIT);
  USART2->CR2 = 0<<12;
  USART2->CR3 = 0;
}

void Uart2_RX_Interrupt_Enable(void)
{
    (void)USART2->DR;                   // (선택) 수신 버퍼를 읽으면 자동 clear

    Macro_Set_Bit(USART2->CR1, RXNEIE_BIT);    // USART2 RX Interrupt Enable
    NVIC_ClearPendingIRQ(USART2_IRQn);         // NIVC Pending Clear
    NVIC_EnableIRQ(USART2_IRQn);               // NVIC Interrupt Enable
}

void Uart2_Send_Byte(char data)
{
  if(data == '\n')
  {
    while(!Macro_Check_Bit_Set(USART2->SR, TXE_BIT));
    USART2->DR = 0x0d;
  }

  while(!Macro_Check_Bit_Set(USART2->SR, TXE_BIT));
  USART2->DR = data;
}