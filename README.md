# STM32 Bare-Metal SysTick Scheduler with I2C LCD

This project demonstrates a **bare-metal cooperative task scheduler** on an
STM32 microcontroller using **SysTick** as the time base and driving a
**16×2 I2C LCD (PCF8574)** without using an RTOS.

## Features
- Bare-metal SysTick (no HAL_Delay)
- Cooperative scheduler with priorities
- I2C LCD (4-bit mode via PCF8574)
- UART debug output
- GPIO LED heartbeat task

## Hardware Used
- STM32 (Cortex-M4)
- 16×2 LCD with I2C backpack (PCF8574)
- On-board LED (GPIO)
- UART (USB-TTL)

## Task List
| Task | Function |
| LCD Task | Displays system messages |
| UART Task | Periodic debug prints |
| LED Task | Heartbeat LED toggle |

## Scheduler Design
- Time base: **SysTick interrupt (1 ms)**
- Non-preemptive cooperative scheduler
- Tasks executed based on:
  - Period
  - Priority
  - Last run time

## Why This Project Matters
- Demonstrates RTOS-like concepts without an RTOS
- Interview-ready example of:
  - SysTick usage
  - Bare-metal timing
  - Cooperative scheduling
  - Low-level I2C LCD control

## Tools Used
- STM32CubeIDE
- Bare-metal C
- GitHub


