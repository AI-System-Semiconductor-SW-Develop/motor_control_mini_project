#define SYSCLK  96000000
#define HCLK	SYSCLK
#define PCLK2	HCLK
#define PCLK1	HCLK/2
#define TIMXCLK ((HCLK == PCLK1)?(PCLK1):(PCLK1*2))

#define RAM_START	0x20000000
#define RAM_END		0x20020000
#define HEAP_BASE	(((unsigned int)&__ZI_LIMIT__ + 0x7) & ~0x7)
#define HEAP_SIZE	(4*1024)
#define HEAP_LIMIT	(HEAP_BASE + HEAP_SIZE)
#define STACK_LIMIT	(HEAP_LIMIT + 8)
#define STACK_BASE	(RAM_END + 1)
#define STACK_SIZE	(STACK_BASE - STACK_LIMIT)

/* Baud rate */
#define BAUD_RATE    (115200U)

/* Timer msec */
#define SHORT_STOP_MSEC   (300U)     
#define LONG_PRESS_MSEC   (3000U) 

/* Bit position */
#define ONE_BIT      (0x1U)
#define TWO_BIT      (0x3U)
#define THREE_BIT    (0x7U)
#define FOUR_BIT     (0xfU)

/* Internal Button */
#define PORTC_BTN_PIN (13U)

/* CLK On-Off */ 
// AHB1ENR: AHB에 속한 주변장치 CLK On-Off
#define GPIOA_EN_BIT (0U)
#define GPIOC_EN_BIT (2U)

// APB1ENR: APB1에 속한 주변장치 CLK On-Off
#define TIM2_EN_BIT  (0U)
#define TIM4_EN_BIT  (2U)
#define TIM5_EN_BIT  (3U)
#define USART_EN_BIT (17U)

// APB2ENR: APB2에 속한 주변장치 CLK On-Off
#define SYSCFG_EN_BIT (14U)

/* Timer */
#define TIM2_TICK	 (20U)	               //usec
#define TIM2_FREQ	 (1000000./TIM2_TICK)  // Hz	
#define TIM2_1ms_Pls (1000./TIM4_TICK)
#define TIM2_MAX	 (0xffffffffU)         // 32bit

#define TIM4_TICK	 (20U) 				   // usec
#define TIM4_FREQ 	 (1000000./TIM4_TICK)  // Hz
#define TIM4_1ms_Pls (1000/TIM4_TICK) 
#define TIM4_MAX	 (0xffffU)             // 16bit

#define TIM5_FREQ			(1000000.)
#define TIM5_TICK			(1000000./TIM5_FREQ)	// 100usec
#define FREQ_TARGET			(10000.)
#define TIM5_1ms_Pls		(1000./TIM5_TICK)		// 10pls = 1msec

// SR: 여러 인터럽트 Pending
#define UIF_BIT      (0U)
   