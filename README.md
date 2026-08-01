# Zonary

---

A minimalist binary-based esoteric programming language.  
一个基于二进制的极简整活编程语言。  

**Warning / 警告**:  
Zonary is an esolang designed for fun and educational purposes. NOT for production.  
Zonary 是一门整活语言（Esolang），专为娱乐和学习设计，请勿用于实际项目。  

---

## English

### Description

Zonary is a minimalist binary-based esoteric programming language created by TasKin.  
It is designed to explore the boundaries of minimalism in language design.  

### Features

- **Binary Registers**: Registers are named using binary numbers (e.g., 00001010), pushing the limits of memorization.  
- **Configurable Bit-Width**: Program bit-width can be set via preprocessor directive, affecting registers, immediates, and labels.  
- **Fixed-Length Instructions**: Every instruction has a fixed bit length, making parsing straightforward.  
- **Pure Binary Code**: Code is composed entirely of 0s and 1s; spaces, newlines, and comments are ignored.  
- **No Negative Numbers**: All values are unsigned integers within the program's bit-width range.  
- **Preprocessor Directives**: { ... } blocks control the runtime environment before program execution.  
- **Memory Optimization**: Registers that are never assigned or cleared to 0 occupy no memory.  
- **Esoteric Fun**: A language designed to be challenging and amusing to write, not for practical use.  

### Instruction Set

| Opcode | Mnemonic | Description |
|--------|----------|-------------|
| 000    | SET      | Assign a value to a register |
| 001    | ADD      | Add a value to a register |
| 010    | SUB      | Subtract a value from a register |
| 011    | JMP      | Unconditional jump or define a label |
| 100    | IFZ      | Conditional jump if register is zero |
| 101    | OUT      | Output a value (binary/decimal/hex/ASCII) |
| 110    | INP      | Read input into a register |
| 111    | SYS      | System call (exit or delay) |

### Requirements

- Python 3.6 or higher.  
- No external dependencies.  

### Files in the Repository

This repository contains the Zonary interpreter, documentation, and test files.  
- `Zonary-Interpreter-version.py` - The Zonary interpreter (Python 3).  
- `Zonary-Documentation-version.txt` - The complete language specification.  
- `test.zonary` - A sample Zonary file for testing.  

### Download Zonary

Download the latest version of the interpreter and documentation from the [Releases](https://github.com/TasKin-tk/Zonary/releases) page.  

### Quick Start

```
git clone https://github.com/TasKin-tk/Zonary.git
cd Zonary
python Zonary-Interpreter-version.py test.zonary
```

Replace `version` with the actual version number.

### License

MIT License  

### Author

TasKin  

Github: https://github.com/TasKin-tk  
Email: tnailkogns@hotmail.com  

---

## 中文

### 简介

Zonary 是一个基于二进制的极简整活编程语言，由 TasKin 制作。  
它的设计目的是探索语言设计中极简主义的边界。  

### 特点

- **二进制寄存器**：寄存器使用二进制数字命名（例如 00001010），挑战你的记忆极限。  
- **可配置位宽**：可通过预处理语句设置程序位数，影响寄存器、立即数和标记。  
- **定长指令**：每条指令长度固定，解析简单。  
- **纯二进制代码**：代码全部由 0 和 1 组成，空格、换行和注释会被忽略。  
- **没有负数**：所有值都在程序位宽所决定的无符号整数范围内。  
- **预处理语句**：`{ ... }` 块在主程序执行前控制运行时环境。  
- **内存优化**：从未赋值或被清零的寄存器不占用内存。  
- **整活乐趣**：一门旨在挑战书写难度和带来乐趣的语言，不适合实际使用。  

### 指令集

| 操作码 | 助记符 | 说明 |
|--------|--------|------|
| 000    | SET    | 给寄存器赋值 |
| 001    | ADD    | 寄存器加一个值 |
| 010    | SUB    | 寄存器减一个值 |
| 011    | JMP    | 无条件跳转或定义标记 |
| 100    | IFZ    | 寄存器为零时跳转 |
| 101    | OUT    | 输出值（二进制/十进制/十六进制/ASCII） |
| 110    | INP    | 从输入读取值到寄存器 |
| 111    | SYS    | 系统调用（退出或延迟） |

### 运行要求

- Python 3.6 及以上版本。  
- 无需安装任何第三方依赖。  

### 仓库文件

本仓库包含 Zonary 解释器，文档，和用于测试的 Zonary 文件。  
- `Zonary-Interpreter-version.py` - Zonary 解释器（Python 3）。  
- `Zonary-Documentation-version.txt` - 完整的语言规范文档。  
- `test.zonary` - 用于测试的 Zonary 文件。  

### 下载 Zonary

请到 [Releases](https://github.com/TasKin-tk/Zonary/releases) 页面下载最新版本的解释器和文档。  

### 快速开始

```
git clone https://github.com/TasKin-tk/Zonary.git
cd Zonary
python Zonary-Interpreter-version.py test.zonary
```

将 version 替换为实际版本号。  

### 开源协议

MIT 协议  

### 作者

TasKin  

GitHub: https://github.com/TasKin-tk  
邮箱: tnailkogns@hotmail.com  
