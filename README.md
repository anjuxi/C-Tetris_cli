# 俄罗斯方块（命令行版）

[![GCC](https://img.shields.io/badge/GCC-11.5.0-blue?style=flat-square)](https://gcc.gnu.org/)
[![Windows](https://img.shields.io/badge/Windows-10%2F11-green?style=flat-square)](https://www.microsoft.com/windows/)
[![C Standard](https://img.shields.io/badge/C-C99-orange?style=flat-square)](https://en.cppreference.com/w/c/99)

命令行版本的经典俄罗斯方块游戏，使用纯C语言编写，在Windows控制台中运行。

## 项目简介

本项目是一个功能完整的俄罗斯方块游戏，通过命令行界面实现。游戏采用增量绘制技术优化性能，只更新变化的区域，而非重绘整个屏幕。

## 功能特性

- **7种经典方块**：I、O、T、S、Z、L、J 共7种俄罗斯方块
- **4种旋转状态**：每种方块支持4个方向的旋转
- **计分系统**：
  - 消除1行：100分
  - 消除2行：300分
  - 消除3行：500分
  - 消除4行：800分
- **等级系统**：每500分提升一个等级，速度逐渐加快
- **下一个方块预览**：右侧面板显示下一个即将出现的方块
- **游戏控制**：支持方向键和WASD双模式控制
- **暂停功能**：支持随时暂停/继续游戏

## 游戏配置

| 配置项       | 默认值 | 说明                 |
| ------------ | ------ | -------------------- |
| BOARD_WIDTH  | 15     | 游戏区域宽度（格）   |
| BOARD_HEIGHT | 30     | 游戏区域高度（格）   |
| BLOCK_SIZE   | 2      | 每格字符宽度         |
| INIT_SPEED   | 300    | 初始下落间隔（毫秒） |

## 操作说明

### 移动控制

| 按键  | 功能     |
| ----- | -------- |
| ← / A | 左移     |
| → / D | 右移     |
| ↑ / W | 旋转     |
| ↓ / S | 加速下落 |

### 其他控制

| 按键   | 功能                   |
| ------ | ---------------------- |
| 空格键 | 硬降（直接落底）       |
| P      | 暂停/继续              |
| X      | 退出游戏               |
| R      | 重新开始（游戏结束后） |

## 编译与运行

### 环境要求

- Windows 操作系统
- GCC 编译器（或兼容的C编译器）

### 编译命令

```bash
gcc 俄罗斯方块（命令行版）.cpp -o 俄罗斯方块.exe
```

### 运行

```bash
俄罗斯方块.exe
```

## 技术实现

- **纯C语言**：无任何外部依赖库
- **Windows API**：使用控制台API实现光标控制、颜色设置、窗口大小调整
- **增量绘制**：采用高效的增量更新策略，只重绘变化的区域
- **结构体封装**：使用Game结构体统一管理游戏状态

## 依赖

本项目使用的标准库头文件：

- `stdio.h` - 标准输入输出
- `stdlib.h` - 标准库函数
- `conio.h` - 控制台输入输出
- `windows.h` - Windows API
- `time.h` - 时间函数
- `stdbool.h` - 布尔类型支持

---

## 作者信息

**谙弆悕博士（Ailan Anjuxi）**

- 邮箱：anjuxi.ME@outlook.com
- SIP电话：sip:anjuxi@sip.linphone.org

