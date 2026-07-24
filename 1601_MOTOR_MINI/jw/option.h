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

/* ================ Bit position ================ */
#define FOUR_BIT (0xFU)
#define TWO_BIT  (0x3U)
#define ONE_BIT  (0x1U)

/* ================ Internal Button PIN ================ */
#define PORTC_BTN_PIN (13U)

/* ================ RCC->APB1FNR ================ */
/* APB1 버스 주변장치 CLK 설정 레지스터 */
#define TIM2_CLK_EN_BIT (0U)
#define TIM4_CLK_EN_BIT (2U)

/* Timer */
/* Timer 2 1pulse 주기, 주파수 */
// #define TIM2_TICK (20U)
// #define TIM2_FREQ (1000000. / TIM2_TICK)
// #define TIM2_1ms_Pls (TIM2_FREQ/1000.)
// #define TIM2_MAX    (0xFFFFU)

// /* Timer 2 1pulse 주기, 주파수 */
// #define TIM3_TICK					(1000000/TIM3_FREQ)	// usec
// #define TIM3_FREQ					(8000000)			// Hz
// #define TIME3_PLS_OF_1ms			(1000/TIM3_TICK)
// #define TIM3_PWM_ARR                (10000U - 1U)        // 10k

// /* Timer 4 1pulse 주기, 주파수 */
// #define TIM4_TICK (20U)
// #define TIM4_FREQ (1000000. / TIM4_TICK)
// #define TIM4_1ms_Pls (TIM4_FREQ/1000.)
// #define TIM4_MAX    (0xFFFFU)

// #define TIM5_TICK (20U)
// #define TIM5_FREQ (1000000. / TIM5_TICK)

/* TIMx->CR1: Timer 기본 설정 레지스터 */
// bit position
#define ARPE_BIT     (7U)
#define DIR_BIT      (4U)
#define OPM_BIT      (3U)
#define CEN_BIT      (0U)
#define UIE_BIT      (0U)
#define UIF_BIT      (0U)

// bit value
#define ARPE_DOWN    (0U)
#define DIR_DOWN     (1U)
#define OPM_ONE_SHOT (1U)
#define OPM_REPEAT   (0U)
#define CEN_ENABLE   (1U)
#define CEN_DISABLE  (0U)

/* TIMx->EGR: 업데이트 발생 레지스터 */
#define UG_BIT (0U)

/* TIMx->SR: Timer 상태 레지스터 */
#define UIF_BIT (0U)

/* 16 bit clock prescaler: (value + 1)분주 */
#define DEFAULT_PSC (TIMXCLK / TIM2_FREQ)

/* 16 bit auto-reload register: reload시 적재될 초기값 설정 */
#define DEFAULT_ARR (0xFFFFU)

/* IQR 관련 비트 설정 */
#define SYSCFG_CLK_ENABLE_BIT  (14U)
#define EXTICR_PORT_C_BIT     (0x2U)

#define FALLING_EDGE_SELECT   (0x1U)
#define RISING_EDGE_SELECT    (0x1U)

#define PENDING_CLEAR_BIT     (0x1U)

/* UART2 */
#define RXNEIE_BIT (5U)

