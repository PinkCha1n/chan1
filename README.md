# chan1

## 项目定位

这是一个基于 **STM32F103 + FreeRTOS** 的“小手表/多功能界面”项目，面向 128x64 OLED 与按键交互场景。

当前主要功能：
- 时钟显示（日期 + 时间）
- 时间设置
- 手电筒（LED 开关）
- 水平仪（MPU6050 加速度角度）
- 偏航速率显示（MPU6050 陀螺仪 Z 轴）
- 秒表

## 系统分层

### 1) 硬件初始化层
- `Core/Src/main.c`
- 负责 HAL 初始化、系统时钟、GPIO/I2C1/I2C2/RTC 初始化以及 RTOS 启动。

### 2) 驱动层
- `Core/Src/OLED.c`
- `Core/Src/MPU6050.c`
- `Core/Src/rtc.c`
- `Core/Src/gpio.c`

### 3) 业务与 UI 层（主实现）
- `Core/Src/freertos.c`
- 集中管理任务、菜单状态机、页面渲染、按键行为、系统周期逻辑。

### 4) 遗留/兼容模块
- `Core/Src/menu.c`
- `Core/Src/SetTime.c`
- `Core/Src/Timer.c`
- `Core/Src/MyI2C.c`

说明：以上模块在当前架构中多为历史实现或兼容保留，主流程以 `freertos.c` 为准。

## 运行主流程

上电后执行流程：
1. `main.c` 完成外设初始化。
2. 调用 `MX_FREERTOS_Init()` 创建任务。
3. 启动调度器后由以下 4 个任务协同运行：
   - `defaultTask`：空转占位。
   - `displayTask`：负责所有界面绘制与刷新。
   - `buttonTask`：处理按键边沿、菜单切换与编辑逻辑。
   - `systemTask`：MPU 采样、秒表计时、LED 输出。

## 核心设计思路

- 使用 `menu_state / func_active_index / set_edit_mode` 构建统一状态机。
- 传感器读取集中在 `systemTask`，显示任务仅读共享值，降低 I2C 访问冲突风险。
- 秒表与水平仪页面使用局部刷新（`OLED_UpdateArea`），提升显示流畅度与效率。

## 状态机说明（简版）

### 状态定义
- `menu_state = 0`：主时钟页
- `menu_state = 1`：功能菜单页
- `menu_state = 2`：时间设置页
- `menu_state = 3`：功能运行页（由 `func_active_index` 区分子功能）

### 按键定义
- 左键：PB1（回退/左选择）
- 右键：PA6（下一个/增加）
- 确认键：PA4（进入/确认）

### 关键跳转
- 主时钟页：
  - 确认 + Menu -> 进入功能菜单页
  - 确认 + Settings -> 进入时间设置页
- 功能菜单页：
  - 右键循环选择功能
  - 确认进入功能运行页
  - 左键返回主时钟页
- 功能运行页：
  - 左键返回功能菜单页
  - 确认触发当前功能动作（如 LED 开关、秒表启停）
- 时间设置页：
  - 非编辑模式：右键切字段，确认进入编辑
  - 编辑模式：右键增量修改，确认进入下一字段或保存退出
  - 左键返回主时钟页（离开时会保存当前缓冲）

## 模块边界建议（落地约定）

为避免职责重叠，建议按以下约定继续演进：
- `freertos.c`：唯一 UI/业务状态机入口。
- `SetTime.c`、`menu.c`：不再新增主流程逻辑，仅保留兼容接口或逐步下线。
- 传感器和显示细节保持在驱动层，任务层只做调度和状态管理。

## 构建与验证说明

当前仓库主要提供 STM32CubeMX/Keil 工程文件（如 `MDK-ARM/Watch_rtos.uvprojx`）。
在本沙箱环境中未发现可直接执行的仓库级 lint/test/build 命令（如 Makefile/CMake/CI 脚本）。
