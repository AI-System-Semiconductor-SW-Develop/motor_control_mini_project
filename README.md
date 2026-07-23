# Motor_Contorl_Mini_Project

버튼 입력 시간을 판단하여 모터의 **시계 방향 회전**, **반시계 방향 회전**, **정지** 상태를 제어하는 프로젝트입니다.

버튼을 짧게 누르면 모터의 동작 또는 회전 방향이 변경되고, 버튼을 3초 이상 누르면 모터가 정지합니다.

---

## 목차

1. [프로젝트 개요](#overview)
2. [Revision History](#revision)
3. [자료형 정의](#datatype)
4. [변수 정의](#variable)
5. [함수 정의](#function)
6. [FSM](#FSM)
7. [event&action](#event&action)
8. [Flowchart](#Flowchart)

---
<a name="overview"></a>

## 1. 프로젝트 개요

### 주요 기능

| 기능 | 설명 |
|---|---|
| 버튼 입력 감지 | 버튼의 Falling Edge와 Rising Edge를 이용하여 입력 상태 확인 |
| 짧게 누르기 | 모터를 회전시키거나 현재 회전 방향 변경 |
| 길게 누르기 | 버튼을 3초 이상 누르면 모터 정지 |
| 방향 전환 보호 | 방향 전환 전 모터를 짧게 정지하여 부하 감소 |
| Timer2 사용 | 버튼을 누르고 있는 시간을 측정 |
| FSM 제어 | 모터 상태에 따라 시계 방향, 반시계 방향, 정지 동작 수행 |

### 참여 인원

**곽혜민**, **도재우**,**박서희**

---
<a name="revision"></a>

## 2. Revision History

| NO. | DATE | Revision Record | REV. |
|---:|:---:|---|:---:|
| 1 | 2026.07.22 | Top Level Designs<br>모터 버튼 제어 | REV. 1.0 |

---
<a name="datatype"></a>

## 3. Type Definition

### 3.1 `MOTOR_STATE`

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

### 3.2 `BTN_STATE`

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

## 4. Flag Variable

| 변수명 | 자료형 | 설명 |
|---|---|---|
| `prev_btn_state` | `BTN_STATE` | 이전 버튼 상태 저장 |
| `curr_btn_state` | `BTN_STATE` | 현재 버튼 상태 저장 |
| `is_timer_started` | `uint8_t` | 타이머의 동작 여부 표시 |
| `is_timer_timeout` | `uint8_t` | 타이머 Timeout 발생 여부 표시 |

### Button State Value

| 버튼 상태 | 값 |
|---|---:|
| Pressed | 1 |
| Released | 0 |

---

<a name="function"></a>

## 5. Function Definition

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

## 6. Motor Control FSM
<img width="4096" height="2880" alt="image" src="https://github.com/user-attachments/assets/7ce9a23e-e5f9-4fc5-bce0-a3351324f343" />

### 6.1 Motor Control FSM State Definition

| State | 설명 |
|---|---|
| `STOP` | 모터 대기 및 정지 상태 |
| `TURN_CLOCKWISE` | 모터 시계 방향 회전 |
| `TURN_COUNTER_CLOCKWISE` | 모터 반시계 방향 회전 |
| `SHORT_STOP` | 방향 전환 시 모터 부하를 줄이기 위한 짧은 정지 |

### 6.2 Motor Control FSM State Transition

| State Transition | 설명 |
|---|---|
| Button Press | 버튼 입력에 따라 모터 동작 상태 변경 |
| Timer2 Timeout | Timer2 Timeout 발생 시 모터 정지 |

### 6.3 State Transition 

| 현재 상태 | 이벤트 | 중간 동작 | 다음 상태 |
|---|---|---|---|
| `STOP` | 버튼 짧게 누름 | 없음 | `TURN_CLOCKWISE` |
| `TURN_CLOCKWISE` | 버튼 짧게 누름 | `SHORT_STOP` | `TURN_COUNTER_CLOCKWISE` |
| `TURN_COUNTER_CLOCKWISE` | 버튼 짧게 누름 | `SHORT_STOP` | `TURN_CLOCKWISE` |
| `TURN_CLOCKWISE` | Timer2 Timeout | 모터 정지 | `STOP` |
| `TURN_COUNTER_CLOCKWISE` | Timer2 Timeout | 모터 정지 | `STOP` |
| `STOP` | Timer2 Timeout | 정지 유지 | `STOP` |

---

<a name="event&action"></a>

## 7. Event & Action

| Event | Action Requirement | Action |
|---|---|---|
| Key Pressed | 버튼이 처음 눌렸을 때 | 3초 Timer2 시작 |
| Key Released | Timeout 발생 전에 버튼을 뗐을 때 | Timer2 정지 및 현재 모터 상태에 따라 모터 제어 |
| Key Pressed | 버튼을 눌렀다가 떼었고 모터가 회전 중인 경우 | 모터 회전 방향 전환 |
| TIM2 Timeout | 버튼을 누르고 있는 동안 Timeout 발생 | UIF 확인 후 모터 정지 |

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

## 최종 동작 정리

```text
1. 전원이 공급되면 시스템을 초기화하고 모터를 정지한다.

2. 버튼을 처음 누르면 Falling Edge를 감지한다.

3. Falling Edge가 감지되면 3초 Timer2를 시작한다.

4. 3초 전에 버튼을 떼면 Rising Edge를 감지한다.

5. 현재 모터가 정지 상태라면 시계 방향으로 회전한다.

6. 현재 모터가 회전 중이라면 잠시 정지한 후 반대 방향으로 회전한다.

7. 버튼을 누른 상태로 3초가 지나면 Timer2 Timeout이 발생한다.

8. Timeout이 발생하면 모터를 정지하고 관련 플래그를 초기화한다.

9. 현재 state 값에 따라 모터 제어 함수를 실행한다.

10. 위 과정을 무한 반복한다.
```

