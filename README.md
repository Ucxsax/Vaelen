# Vaelen Linux Kernel

一个自研的Linux内核项目，仅CLI界面，无桌面环境。

## 项目结构

```
Vaelen/
├── arch/          # 架构相关代码
│   └── x86_64/    # x86_64架构
├── boot/          # 引导加载程序
│   └── boot.s     # Multiboot引导汇编
├── kernel/        # 内核核心代码
│   ├── kernel.c   # 内核主入口
│   └── shell.c    # CLI Shell
├── fs/            # 文件系统
├── init/          # 初始化代码
├── lib/           # 库函数
│   └── string.c   # 字符串操作
├── include/       # 头文件
│   ├── kernel.h
│   ├── vga.h
│   ├── string.h
│   ├── stdint.h
│   └── shell.h
├── linker.ld      # 链接脚本
├── Makefile       # 构建配置
└── Task.md        # 项目任务说明
```

## 已实现功能

- Multiboot兼容引导程序
- VGA文本模式终端
- 基本shell命令：ls, cd, mkdir, rm, echo, help, clear
- 键盘输入支持

## 编译要求

需在Linux环境下编译，需要以下工具：

1. **交叉编译器**
   ```bash
   # Debian/Ubuntu
   sudo apt install gcc-multilib g++-multilib
   # 或安装i686-elf工具链
   ```

2. **GRUB工具**
   ```bash
   sudo apt install grub-pc-bin xorriso
   ```

## 编译命令

```bash
# 编译内核
make all

# 生成ISO镜像
make iso

# 清理
make clean
```

## 运行

生成的ISO文件为 `Vaelen-linux-0.1-TEST.iso`，可用于物理机引导。

## 注意事项

- 项目设计用于物理机运行，不保证虚拟机兼容性
- 测试配置：Intel Core i5, 8GB RAM, 495GB HDD