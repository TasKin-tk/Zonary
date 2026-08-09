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
Written in C, it is distributed as a single binary file with no external dependencies.  

### Features

- **Binary Registers**: Registers are named using binary numbers (e.g., 00001010).  
- **Configurable Bit-Width**: Program bit-width can be set via preprocessor directive.  
- **Fixed-Length Instructions**: Every instruction has a fixed bit length.  
- **Pure Binary Code**: Code is composed entirely of 0s and 1s.  
- **Preprocessor Directives**: `/00` and `/01` for custom characters and bit-width.  
- **Memory Optimization**: Registers never assigned or cleared to 0 occupy no memory.  
- **Esoteric Fun**: A language designed to be challenging and amusing to write.  

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


### Quick Start

First, download the appropriate executable and installation script from the [Releases](https://github.com/TasKin-tk/Zonary/releases) page.  
Then run the installation script to install NalTool. During installation, the script will ask for the path to the executable; enter the path of the downloaded executable.  

If you cannot find a suitable executable in Releases, try cloning the repository and compiling it manually.  

### Usage Examples

| Command | Description |
|---------|-------------|
| `zonary examples/example.zonary` | Run the example program |
| `zonary -h` | Show help information |
| `zonary -v` | Show version information |

### Prebuilt Platforms

This repository provides prebuilt executables for the following platforms, located in the `bin` directory:

- Windows (x86_64)
- macOS (x86_64, aarch64)
- Linux (x86_64, aarch64)

### License

MIT License  

### Author

TasKin  

GitHub: https://github.com/TasKin-tk  
Email: tnailkogns@hotmail.com  

---

## 中文

### 简介

Zonary 是一个基于二进制的极简整活编程语言，由 TasKin 制作。  
它的设计目的是探索语言设计中极简主义的边界。  
使用 C 语言编写，以单文件二进制形式分发，无需外部依赖。  

### 特点

- **二进制寄存器**：寄存器使用二进制数字命名。  
- **可配置位宽**：可通过预处理语句设置程序位数。  
- **定长指令**：每条指令长度固定。  
- **纯二进制代码**：代码全部由 0 和 1 组成。  
- **预处理语句**：`/00` 和 `/01` 用于自定义字符和位宽。  
- **内存优化**：从未赋值或被清零的寄存器不占用内存。  
- **整活乐趣**：一门旨在挑战书写难度和带来乐趣的语言。  

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

### 快速开始

请先到 [Releases](https://github.com/TasKin-tk/Zonary/releases) 页面下载合适的可执行文件和安装脚本。  
然后运行安装脚本进行安装。安装时，安装脚本会提示输入可执行文件的路径，输入下载的可执行文件的路径即可。  

如果你没有在 Releases 里找到合适的可执行文件，请尝试克隆仓库后手动编译。  

### 使用示例

| 命令 | 说明 |
|------|------|
| `zonary examples/example.zonary` | 运行示例程序 |
| `zonary -h` | 显示帮助信息 |
| `zonary -v` | 显示版本信息 |

### 预编译平台

本仓库提供以下平台的预编译二进制文件，存放在 `bin` 目录下：

- Windows（x86_64）
- macOS（x86_64、aarch64）
- Linux（x86_64、aarch64）

### 开源协议

MIT 协议  

### 作者

TasKin  

GitHub：https://github.com/TasKin-tk  
邮箱：tnailkogns@hotmail.com
