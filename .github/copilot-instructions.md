# Project Guidelines

## Code Style
- Keep edits inside STM32CubeMX user regions only: comments between USER CODE BEGIN and USER CODE END.
- Preserve existing module naming patterns:
  - Initialization: MX_<MODULE>_Init
  - Task entry: Start<Name>Task
  - Driver APIs: <Module>_<Action>
- Match current C style in Core/Src and Core/Inc (HAL + CMSIS-RTOS2 patterns, no large refactors unless requested).

## Architecture
- This project is an STM32F103C8 firmware using HAL + FreeRTOS (CMSIS-RTOS v2 wrapper).
- Main startup flow is in Core/Src/main.c:
  HAL_Init -> clock setup -> MX_GPIO/I2C/RTC init -> osKernelInitialize -> MX_FREERTOS_Init -> osKernelStart.
- Core runtime logic is task-based in Core/Src/freertos.c:
  - Display task: OLED rendering and menu UI
  - Button task: key scanning and menu navigation
  - System task: MPU6050 data acquisition
- Display and sensor stacks use different I2C implementations:
  - I2C1: HAL driver path (OLED)
  - I2C2/PB10-PB11: software bit-banged I2C in Core/Src/MyI2C.c

## Build and Test
- Primary build environment is Keil MDK-ARM project:
  - MDK-ARM/Watch_rtos.uvprojx (target: Watch_rtos)
- Preferred build method:
  - Open the uvprojx in Keil uVision and build target Watch_rtos.
- Optional CLI build (only if Keil CLI tools are installed and configured):
  - UV4.exe -b MDK-ARM/Watch_rtos.uvprojx -t Watch_rtos
- There is no host-side unit test suite in this repository. Validate behavior on hardware after firmware changes.

## Conventions
- FreeRTOS memory is tight on STM32F103C8 (see Core/Inc/FreeRTOSConfig.h):
  - configTOTAL_HEAP_SIZE is 3072 bytes.
  - Avoid introducing large stacks, large static buffers, or extra tasks unless necessary.
- In task context, prefer osDelay over HAL_Delay.
- Treat shared cross-task state carefully:
  - Existing code uses volatile globals in multiple modules.
  - Keep updates simple/atomic for small scalar values, and avoid adding race-prone multi-step writes.
- Be conservative when modifying Core/Src/MyI2C.c timing loops:
  - Software I2C delay constants are hardware/timing sensitive.

## Key Files
- Core/Src/main.c: startup and init order
- Core/Src/freertos.c: task creation and application logic
- Core/Src/MyI2C.c: software I2C implementation
- Core/Inc/FreeRTOSConfig.h: RTOS memory and kernel config
- MDK-ARM/Watch_rtos.uvprojx: compiler/build target configuration
- Watch_rtos.ioc: CubeMX source configuration
