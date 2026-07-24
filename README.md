# Motor_Contorl_Mini_Project

외부 버튼 인터럽트, USART 수신 인터럽트, TIMER 인터럽트를 이용하여 모터의 회전 방향과 속도 정지 상태를 제어하는 프로젝트 입니다.

각 인터럽트 서비스 루틴에서 입력을 확이하고 이벤트 플래그만 설정하며, 실제 모터 상태 변경과 제어는 Main Loop에서 실행합니다.

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

버튼, 타이머, UART 인터럽트를 이용하여 DC 모터의 회전 방향과 속도를 제어한다.

버튼 입력은 Short Press와 Long Press로 구분하며, 방향 전환 시에는 모터를 잠시 정지한 후 반대 방향으로 회전한다.

UART2로 속도 명령을 수신하여 TIM5 PWM Duty를 변경한다.

### 주요 기능

| 기능 | 설명 |
|---|---|
| Key Interrupt | 버튼의 누름과 버튼 해제 이벤트 감지 |
| UART2 RX Inerrupt | UART로 PWM Duty 명령 수신 |
| Timer Interrupt | 버튼을 3초 이상 누르는 long press 판단 |
| TIM5 PWM| 10KHz PWM을 이용한 모터 속도 제어|
| Motor FSM | 정지, 시계 방향, 반시계 방향 상태 관리 |
| 방향 전환 보호 | 방향 전환 전 모터를 짧게 정지하여 부하 감소 |

### 참여 인원

**곽혜민**, **도재우**,**박서희**

---
<a name="revision"></a>

## 2. Revision History

| NO. | DATE | Revision Record | REV. |
|---:|:---:|---|:---:|
| 1 | 2026.07.22 | Top Level Designs<br>모터 버튼 제어 | REV. 1.0 |
| 2 | 2026.07.24 | Top Level Designs<br>모터 버튼 제어 | REV. 2.0 |

---

<a name="event&action"></a>

## 3. Event & Action

### 3.1 이벤트 처리표

| Event | 발생 조건 | Interrupt Action | Main Action |
|---|---|---|---|
| `KEY_PRESSED` | 버튼을 누름 | `key_state`를 `IS_PRESSED`로 설정 | TIM2 3초 타이머 시작 |
| `KEY_RELEASED` | 버튼을 뗌 | `key_state`를 `IS_RELEASED`로 설정 | TIM2 정지 후 Short Press 처리 |
| `TIM2_TIMEOUT` | 버튼을 누른 상태로 3초 경과 | `TIM2_Expired` 설정 | 모터 정지 |
| `TIM4_TIMEOUT` | 방향 전환을 위한 Short Stop 시간 경과 | `TIM4_Expired` 설정 | `next_state` 방향으로 모터 전환 |
| `UART_RX` | UART2 데이터 수신 | 수신 문자에 따라 `curr_motor_speed` 저장 | TIM5 PWM Duty 갱신 |

---

### 3.2 Short Press 동작

버튼을 누른 후 TIM2 Timeout이 발생하기 전에 버튼을 떼면 Short Press로 처리한다.

| 현재 상태 | Action | 다음 상태 |
|---|---|---|
| `STOP` | 모터 회전 시작 | 첫 번째 회전 방향 |
| `CW` | Short Stop 후 반시계 방향 회전 | `CCW` |
| `CCW` | Short Stop 후 시계 방향 회전 | `CW` |

방향을 전환할 때 즉시 반대 방향으로 회전하지 않고 TIM4를 이용한 Short Stop을 수행한다.

---

### 3.3 Long Press 동작

버튼을 누르면 TIM2 3초 타이머를 시작한다.

버튼을 누른 상태로 3초가 지나면 TIM2 인터럽트가 발생하고 현재 모터 상태와 관계없이 모터를 정지한다.

```text
버튼 누름
    ↓
TIM2 3초 타이머 시작
    ↓
3초 동안 버튼 유지
    ↓
TIM2 Timeout
    ↓
Motor Stop
```

TIM2 Timeout 이후 버튼을 떼더라도 Short Press 동작은 수행하지 않는다.

---

### 3.4 UART 속도 제어

UART2로 문자 `'1'`부터 `'9'`까지 수신하여 모터 속도를 변경한다.

| UART 입력 | 설정 속도 |
|:---:|:---:|
| `'1'` | `SPD_1` |
| `'2'` | `SPD_2` |
| `'3'` | `SPD_3` |
| `'4'` | `SPD_4` |
| `'5'` | `SPD_5` |
| `'6'` | `SPD_6` |
| `'7'` | `SPD_7` |
| `'8'` | `SPD_8` |
| `'9'` | `SPD_9` |

수신 문자를 이용하여 `speed_table`에서 모터 속도 값을 선택한다.

```text
Speed Index = 수신 문자 - '0' - 1
```

유효하지 않은 문자를 수신하면 현재 속도를 유지한다.

<a name="datatype"></a>

## 4. Type Definition

### 4.1 `MOTOR_STATE`

모터의 현재 동작 상태를 나타내는 열거형입니다.

```c
typedef enum
{
    TURN_CLOCKWISE = 0,
    TURN_COUNTER_CLOCKWISE = 1,
    STOP = 2
} MOTOR_STATE;
```

| 상태 | 값 | 설명 |
|---|---:|---|
| `TURN_CLOCKWISE` | 0 | 모터 시계 방향 회전 |
| `TURN_COUNTER_CLOCKWISE` | 1 | 모터 반시계 방향 회전 |
| `STOP` | 2 | 모터 정지 |

### 4.2 `BTN_STATE`

버튼이 눌려 있는지를 나타내는 열거형입니다.

```c
typedef enum
{
    IS_RELEASED = 0,
    IS_PRESSED = 1
} BTN_STATE;
```

| 상태 | 값 | 설명 |
|---|---:|---|
| `IS_RELEASED` | 0 | 버튼을 뗀 상태 |
| `IS_PRESSED` | 1 | 버튼을 누른 상태 |

---
<a name="variable"></a>

## 5. Flag Variable

| 변수명 | 자료형 | 설명 |
|---|---|---|
| `prev_btn_state` | `BTN_STATE` | 이전 버튼 상태 저장 |
| `curr_btn_state` | `BTN_STATE` | 현재 버튼 상태 저장 |
| `is_timer_started` | `uint8_t` | 타이머의 동작 여부 표시 |
| `is_timer_timeout` | `uint8_t` | 타이머 Timeout 발생 여부 표시 |

---
<a name="function"></a>

## 6. Function Definition

| NO. | 함수명 | 반환형 | 매개변수 | 기능 |
|---:|---|---|---|---|
| 1 | `Sys_Init` | `void` | `void` | Clock 초기화 |
| 2 | `GPIO_Init` | `void` | `void` | 모터 제어용 PA0, PA1 핀 초기화 |
| 3 | `Key_Poll_Init` | `void` | `void` | 보드 내부 스위치 초기화 |
| 4 | `Turn_Clockwise` | `void` | `void` | 모터 시계 방향 회전 |
| 5 | `Turn_Counter_Clockwise` | `void` | `void` | 모터 반시계 방향 회전 |
| 6 | `Stop` | `void` | `void` | 모터 정지 |
| 7 | `Short_Stop` | `void` | `int msec` | 방향 전환 전 모터를 짧게 정지 |
| 8 | `TIM2_One_Shot` | `void` | `int msec` | 지정된 시간만큼 Timer2 시작 |
| 9 | `TIM2_Check_Timeout` | `uint8_t` | `void` | Timer2 Timeout 발생 여부 검사 |

---

<a name="FSM"></a>

## 7. Motor Control FSM
<img width="4096" height="2880" alt="image" src="https://github.com/user-attachments/assets/7ce9a23e-e5f9-4fc5-bce0-a3351324f343" />

### 7.1 Motor Control FSM State Definition

| State | 설명 |
|---|---|
| `STOP` | 모터 대기 및 정지 상태 |
| `TURN_CLOCKWISE` | 모터 시계 방향 회전 |
| `TURN_COUNTER_CLOCKWISE` | 모터 반시계 방향 회전 |
| `SHORT_STOP` | 방향 전환 시 모터 부하를 줄이기 위한 짧은 정지 |

### 7.2 Motor Control FSM State Transition

| State Transition | 설명 |
|---|---|
| Button Press | 버튼 입력에 따라 모터 동작 상태 변경 |
| Timer2 Timeout | Timer2 Timeout 발생 시 모터 정지 |

### 7.3 State Transition 

| 현재 상태 | 이벤트 | 중간 동작 | 다음 상태 |
|---|---|---|---|
| `STOP` | 버튼 짧게 누름 | 없음 | `TURN_CLOCKWISE` |
| `TURN_CLOCKWISE` | 버튼 짧게 누름 | `SHORT_STOP` | `TURN_COUNTER_CLOCKWISE` |
| `TURN_COUNTER_CLOCKWISE` | 버튼 짧게 누름 | `SHORT_STOP` | `TURN_CLOCKWISE` |
| `TURN_CLOCKWISE` | Timer2 Timeout | 모터 정지 | `STOP` |
| `TURN_COUNTER_CLOCKWISE` | Timer2 Timeout | 모터 정지 | `STOP` |
| `STOP` | Timer2 Timeout | 정지 유지 | `STOP` |

---

<a name="Flowchart"></a>

## 8. Flow Chart
<img width="5762" height="7512" alt="image" src="https://github.com/user-attachments/assets/b7d08b30-ee07-48ec-af41-3c682b47999c" />

### 8.1 Flow Chart Detail

<details>
<summary><strong>전체 Flowchart 단계 보기</strong></summary>

| 노드 구분 | 단계명 | 상세 설명 | 연관 함수 및 변수 |
|---|---|---|---|
| Start | Power On | 시스템에 전원 공급 | - |
| Process | 초기화 | Clock, 내부 스위치, 모터 제어 핀 초기화 및 모터 정지 | `Sys_Init()`<br>`Key_Poll_Init()`<br>`GPIO_Init()`<br>`Stop()` |
| Decision | Button Falling Edge Detected | 버튼이 처음 눌렸으면 True | `curr_btn_state`<br>`prev_btn_state` |
| Process | `TIM2_One_Shot(3000)` | 3초 Timer2 시작 | `TIM2_One_Shot()` |
| Process | 타이머 플래그 설정 | Timeout 플래그를 0으로 설정하고 타이머 동작 플래그 Set | `is_timer_timeout`<br>`is_timer_started` |
| Decision | Button Rising Edge Detected | 버튼이 떼어졌으면 True | `curr_btn_state`<br>`prev_btn_state` |
| Process | Timer Stop | 버튼을 떼면 타이머 동작 플래그 Clear | `is_timer_started` |
| Decision | `is_timer_timeout` | Timeout 플래그가 1이면 True | `is_timer_timeout` |
| Decision | `state == STOP` | 현재 모터 상태가 정지이면 True | `state`<br>`STOP` |
| Process | `state = 1 - state` | 회전 중이면 상태값을 토글하여 방향 변경 | `state` |
| Process | `state = TURN_CLOCKWISE` | 정지 상태에서 버튼을 짧게 누르면 시계 방향 회전 | `state`<br>`TURN_CLOCKWISE` |
| Process | `state = STOP` | 버튼을 떼었을 때 이미 Timeout 상태라면 모터 정지 | `state` |
| Decision | Timer2 Timeout | Timeout이 발생했고 타이머가 동작 중이면 True | `TIM2_Check_Timeout()`<br>`is_timer_started` |
| Process | 상태 및 플래그 초기화 | 모터 정지 후 Timeout과 타이머 동작 플래그 Clear | `state`<br>`is_timer_timeout`<br>`is_timer_started` |
| Decision | State | 현재 `state` 값에 따라 모터 동작 결정 | `state` |
| Process | Clockwise | 모터 시계 방향 회전 | `Turn_Clockwise()` |
| Process | Counter Clockwise | 모터 반시계 방향 회전 | `Turn_Counter_Clockwise()` |
| Process | Stop | 모터 정지 | `Stop()` |
| Process | Default | 정의되지 않은 상태일 경우 모터 정지 | `Stop()` |

</details>

---


9. 현재 state 값에 따라 모터 제어 함수를 실행한다.

10. 위 과정을 무한 반복한다.
```


