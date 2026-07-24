# Motor_Control_Mini_Project

외부 버튼 인터럽트, USART2 수신 인터럽트, TIMER 인터럽트를 이용하여 모터의 회전 방향과 속도, 정지 상태를 제어하는 프로젝트입니다.

각 인터럽트 서비스 루틴(ISR)에서는 입력을 확인하고 이벤트 플래그만 설정하며, 실제 모터 상태 변경과 제어는 Main Loop에서 실행합니다.

---

## 목차

1. [프로젝트 개요](#overview)
2. [Revision History](#revision)
3. [Event & Action](#event&action)
4. [Type 정의](#datatype)
5. [변수 정의](#variable)
6. [함수 정의](#function)
7. [FSM](#FSM)
8. [Flowchart](#Flowchart)

---
<a name="overview"></a>

## 1. 프로젝트 개요

### 주요 기능

| 기능 | 설명 |
|---|---|
| EXTI Key Interrupt | 버튼의 눌림(Falling)과 뗌(Rising) 이벤트를 인터럽트로 감지 |
| USART2 RX Interrupt | UART로 숫자(`1`~`9`)를 수신하여 PWM Duty(모터 속도) 명령 수신 |
| TIM2 Interrupt | 버튼을 3초 이상 누르는 Long Press(정지) 판단 |
| TIM4 Interrupt | 방향 전환 시 Short Stop(짧은 정지) 후 반대 방향 회전 재개 |
| TIM5 PWM | 10KHz PWM으로 모터 속도(Duty) 제어 |
| Motor FSM | 정지(STOP), 정회전(CW), 역회전(CCW), 짧은 정지(SHORT_STOP) 상태 관리 |

### 참여 인원

**혜민** **서희** **재우**

---
<a name="revision"></a>

## 2. Revision History

| NO. | DATE | Revision Record | REV. |
|---:|:---:|---|:---:|
| 1 | 2026.07.22 | 폴링 방식 버튼 제어, 3초 정지 로직 구현 | REV. 1.0 |
| 2 | 2026.07.24 | EXTI/TIM2/TIM4/USART2 인터럽트 기반으로 전면 개편<br>UART 속도 제어(PWM Duty) | REV. 1.1 |

---

<a name="event&action"></a>

## 3. Event & Action

인터럽트에서는 입력 상태와 이벤트를 전역 플래그(Flag)에 저장하고,
Main Loop에서 해당 플래그를 확인하여 모터 상태와 PWM Duty를 변경한다.

### 3.1 이벤트 처리표

| Event | 발생 조건 | Action |
|---|---|---|
| `key_state == IS_PRESSED` | EXTI13 Falling Edge (버튼 누름) | `TIM2_3sec_Timer_Start()` 호출, 3초 타이머 시작 |
| `key_state == IS_RELEASED` | EXTI13 Rising Edge (버튼 뗌) | 타이머 동작 중이면 `TIM2_Stop()` 후 `Toggle_Motor_State()` 호출 |
| `TIM2_Expired` | 버튼을 3초 이상 누른 상태로 TIM2 Timeout 발생 | `Motor_Stop()` 호출, `motor_state = STOP` |
| `TIM4_Expired` | 방향 전환 시 Short Stop 후 TIM4 Timeout 발생 | `Toggle_with_Short_Stop()` 호출, 예약된 다음 방향으로 회전 |
| `Uart_Data_In` | USART2로 숫자 데이터 수신 | `TIM5_PWM_Generation(curr_motor_speed)` 호출, PWM Duty 갱신 |

### 3.2 버튼 Short Press 동작 (`Toggle_Motor_State`)

| 현재 상태 | 동작 | 다음 상태 |
|---|---|---|
| `STOP` | 정회전 시작 (`Motor_CW`) | `CW` |
| `CW` | 짧은 정지 후 역회전 예약 (`Motor_Short_Stop`, `next_state = CCW`) | `SHORT_STOP` |
| `CCW` | 짧은 정지 후 정회전 예약 (`Motor_Short_Stop`, `next_state = CW`) | `SHORT_STOP` |
| 그 외 | 모터 정지 (`Motor_Stop`) | `STOP` |

### 3.3 Long Press 동작

버튼을 3초 이상 누르면 현재 모터 상태와 관계없이 모터를 정지한다.

```text
버튼 누름 (key_state = IS_PRESSED)
    ↓
TIM2_3sec_Timer_Start()
    ↓
TIM2 Timeout → TIM2_Expired = 1
    ↓
Motor_Stop(), motor_state = STOP
```

### 3.4 방향 전환 시 Short Stop 동작

버튼을 짧게 눌렀다 떼면 즉시 반대 방향으로 회전하지 않고, 모터 부하 감소를 위해 짧게 정지한 뒤 방향을 전환한다.

```text
버튼 짧게 누름/뗌 (Short Press)
    ↓
Motor_Short_Stop(), next_state 예약
    ↓
TIM4 Timeout(200ms) → TIM4_Expired = 1
    ↓
Toggle_with_Short_Stop() → 예약된 방향으로 회전
```

### 3.5 UART 입력 동작

USART2로 숫자 문자 `'1'`부터 `'9'`까지 수신하면, `speed_table`에서 해당 인덱스의 속도를 읽어 `curr_motor_speed`를 갱신하고 PWM Duty를 재생성한다.

### 3.6 이벤트 처리 순서

Main Loop에서는 다음 순서로 이벤트를 처리한다.

```text
KEY_PRESSED → KEY_RELEASED → TIM2_Expired → TIM4_Expired → Uart_Data_In
```

TIM2 Timeout(Long Press 정지)을 버튼 해제 판단과 별도로 처리하여
Long Press와 Short Press 로직이 중복 실행되지 않도록 한다.

---

<a name="datatype"></a>

## 4. Type Definition

### 4.1 `MOTOR_STATE`

모터의 현재 동작 상태를 나타내는 열거형입니다.

```c
typedef enum t_MOTOR_STATE
{
    STOP = 0U,
    CW = 1U,
    CCW = 2U,
    SHORT_STOP = 3U
} MOTOR_STATE;
```

| 상태 | 값 | 설명 |
|---|---:|---|
| `STOP` | 0 | 모터 정지 |
| `CW` | 1 | 모터 정회전(시계 방향) |
| `CCW` | 2 | 모터 역회전(반시계 방향) |
| `SHORT_STOP` | 3 | 방향 전환 전 짧은 정지 상태 |

### 4.2 `KEY_STATE`

버튼 인터럽트 이벤트를 나타내는 열거형입니다.

```c
typedef enum t_KEY_STATE
{
    KEY_IDLE = 0U,      // IDLE
    IS_PRESSED = 1U,    // Falling
    IS_RELEASED = 2U    // Rising
} KEY_STATE;
```

| 상태 | 값 | 설명 |
|---|---:|---|
| `KEY_IDLE` | 0 | 처리할 버튼 이벤트 없음 |
| `IS_PRESSED` | 1 | 버튼이 눌림(Falling Edge) |
| `IS_RELEASED` | 2 | 버튼이 떼어짐(Rising Edge) |

### 4.3 `MOTOR_SPEED`

UART로 설정 가능한 모터 속도(PWM Duty %)를 나타내는 열거형입니다.

```c
typedef enum t_MOTOR_SPEED
{
    SPD_1 = 50, SPD_2 = 55, SPD_3 = 60, SPD_4 = 65, SPD_5 = 70,
    SPD_6 = 75, SPD_7 = 80, SPD_8 = 85, SPD_9 = 90
} MOTOR_SPEED;
```

| 단계 | Duty(%) | 단계 | Duty(%) |
|---|---:|---|---:|
| `SPD_1` | 50 | `SPD_6` | 75 |
| `SPD_2` | 55 | `SPD_7` | 80 |
| `SPD_3` | 60 | `SPD_8` | 85 |
| `SPD_4` | 65 | `SPD_9` | 90 |
| `SPD_5` | 70 | `SPD_DEFAULT` | 100 (기본값) |

---
<a name="variable"></a>

## 5. Variables

| 변수명 | 자료형 | 설명 |
|---|---|---|
| `key_state` | `volatile KEY_STATE` | EXTI ISR에서 설정하는 버튼 Press/Release 이벤트 플래그 |
| `TIM2_Expired` | `volatile int` | TIM2(3초 Long Press) Timeout 발생 여부 |
| `TIM4_Expired` | `volatile int` | TIM4(Short Stop) Timeout 발생 여부 |
| `Uart_Data_In` | `volatile int` | USART2 데이터 수신 여부 |
| `curr_motor_speed` | `volatile MOTOR_SPEED` | 현재 설정된 모터 속도(PWM Duty), 기본값 `SPD_DEFAULT` |
| `next_state` | `volatile MOTOR_STATE` | Short Stop 이후 전환할 다음 회전 방향 |
| `motor_state` | `MOTOR_STATE` | 현재 모터 상태(Main 지역 변수), `STOP`으로 초기화 |
| `is_timer_started` | `int` | TIM2(3초 타이머) 동작 여부(Main 지역 변수) |

---
<a name="function"></a>

## 6. Function Definition

| NO. | 함수명 | 반환형 | 매개변수 | 기능 |
|---:|---|---|---|---|
| 1 | `Sys_Init` | `void` | `int baud` | Clock, UART1/2, LED 초기화 |
| 2 | `GPIO_Init` | `void` | `void` | 모터 제어용 PA0, PA1 및 버튼용 PC13 초기화 |
| 3 | `Key_ISR_Enable` | `void` | `int en` | EXTI13(PC13) 버튼 인터럽트 활성화/비활성화 |
| 4 | `Uart2_RX_Interrupt_Enable` | `void` | `void` | USART2 수신 인터럽트 활성화 |
| 5 | `TIM2_3sec_Timer_Start` | `void` | `void` | 3초 Long Press 판단용 TIM2 시작 |
| 6 | `TIM2_Stop` | `void` | `void` | TIM2 정지 및 인터럽트 비활성화 |
| 7 | `TIM4_Delay_Interrupt_Enable` | `void` | `int time` | 지정 시간(ms) 후 TIM4 인터럽트 발생, Short Stop용 |
| 8 | `TIM5_Out_Init` | `void` | `void` | TIM5 PWM 출력(CH1, CH2) 초기화 |
| 9 | `TIM5_PWM_Generation` | `void` | `int duty` | 10kHz PWM Duty(%) 설정, 모터 속도 반영 |
| 10 | `Toggle_Motor_State` | `void` | `MOTOR_STATE *state` | 버튼 Short Press 시 모터 상태 토글(STOP→CW→SHORT_STOP↔SHORT_STOP) |
| 11 | `Toggle_with_Short_Stop` | `void` | `MOTOR_STATE *state, MOTOR_STATE next_state` | Short Stop 이후 예약된 방향으로 회전 재개 |
| 12 | `Motor_Stop` | `void` | `void` | 모터 정지(GPIO Stop 모드 전환) |
| 13 | `Motor_Short_Stop` | `void` | `void` | 모터 정지 후 200ms TIM4 인터럽트 예약 |
| 14 | `Motor_CW` | `void` | `void` | 모터 정회전(GPIO Alternate/Output 모드 전환) |
| 15 | `Motor_CCW` | `void` | `void` | 모터 역회전(GPIO Alternate/Output 모드 전환) |
| 16 | `GPIO_Mode_CW` / `GPIO_Mode_CCW` / `GIPO_Mode_Stop` | `void` | `void` | 회전 방향에 따른 PA0/PA1 핀 모드(PWM/Output) 전환 |

---

<a name="FSM"></a>

## 7. Motor Control FSM

### 7.1 Motor Control FSM State Definition

| State | 설명 |
|---|---|
| `STOP` | 모터 대기 및 정지 상태 |
| `CW` | 모터 정회전(시계 방향) |
| `CCW` | 모터 역회전(반시계 방향) |
| `SHORT_STOP` | 방향 전환 시 모터 부하를 줄이기 위한 짧은 정지(200ms) 상태 |

### 7.2 Motor Control FSM State Transition

| State Transition | 설명 |
|---|---|
| Button Short Press | 버튼을 짧게 누르고 뗐을 때 모터 상태를 전환 |
| Button Long Press (3sec) | 버튼을 3초 이상 눌렀을 때 TIM2 Timeout으로 모터 정지 |
| TIM4 Timeout | `SHORT_STOP` 이후 200ms 경과 시 예약된 방향으로 회전 재개 |

### 7.3 State Transition

| 현재 상태 | 이벤트 | 중간 동작 | 다음 상태 |
|---|---|---|---|
| `STOP` | 버튼 짧게 누름/뗌 | 없음, `Motor_CW()` 즉시 실행 | `CW` |
| `CW` | 버튼 짧게 누름/뗌 | `Motor_Short_Stop()`, `next_state = CCW` | `SHORT_STOP` |
| `CCW` | 버튼 짧게 누름/뗌 | `Motor_Short_Stop()`, `next_state = CW` | `SHORT_STOP` |
| `SHORT_STOP` | TIM4 Timeout | `next_state`에 따라 `Motor_CW()` 또는 `Motor_CCW()` | `CW` 또는 `CCW` |
| `CW` / `CCW` | TIM2 Timeout(3초 이상 누름) | `Motor_Stop()` | `STOP` |
| `STOP` | TIM2 Timeout | 정지 유지 | `STOP` |

---

<a name="Flowchart"></a>

## 8. Flow Chart

### 8.1 Flow Chart Detail

<details>
<summary><strong>전체 Flowchart 단계 보기</strong></summary>

| 노드 구분 | 단계명 | 상세 설명 | 연관 함수 및 변수 |
|---|---|---|---|
| Start | Power On | 시스템에 전원 공급 | - |
| Process | 초기화 | Clock, UART1/2, LED, GPIO, TIM5 PWM 초기화 및 인터럽트 Enable | `Sys_Init()`<br>`GPIO_Init()`<br>`TIM5_Out_Init()`<br>`Key_ISR_Enable()`<br>`Uart2_RX_Interrupt_Enable()`<br>`TIM5_PWM_Generation()` |
| Decision | `key_state == IS_PRESSED` | 버튼이 눌렸는지 확인 | `key_state` |
| Process | 3초 타이머 시작 | Long Press 판단용 TIM2 시작 및 동작 플래그 Set | `TIM2_3sec_Timer_Start()`<br>`is_timer_started` |
| Decision | `key_state == IS_RELEASED` | 버튼이 떼어졌는지 확인 | `key_state` |
| Process | 타이머 정지 및 상태 토글 | 타이머가 동작 중이면 TIM2 정지 후 모터 상태 토글 | `TIM2_Stop()`<br>`Toggle_Motor_State()` |
| Decision | `TIM2_Expired` | 3초 Timeout 발생 여부 확인 | `TIM2_Expired` |
| Process | 모터 정지 | Timeout 시 모터 정지 및 상태 초기화 | `Motor_Stop()`<br>`motor_state = STOP` |
| Decision | `TIM4_Expired` | Short Stop Timeout 발생 여부 확인 | `TIM4_Expired` |
| Process | 방향 전환 재개 | 예약된 `next_state` 방향으로 회전 재개 | `Toggle_with_Short_Stop()` |
| Decision | `Uart_Data_In` | UART 데이터 수신 여부 확인 | `Uart_Data_In` |
| Process | PWM Duty 갱신 | 수신된 속도값으로 PWM 재생성 | `TIM5_PWM_Generation()`<br>`curr_motor_speed` |
| Loop | Main Loop 반복 | 위 이벤트들을 무한 반복하며 확인 | `for(;;)` |

</details>
