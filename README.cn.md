# 关于

适用于 GCC Arm Toolchains 环境下的 AIR32F103x 系列 MCU 项目开发模板

# AIR32F103x 系列

| TYPE  | AIR32F103CBT6 | AIR32F103CCT6 | AIR32F103RPT6 |
| ----- | ------------- | ------------- | ------------- |
| Flash | 128K          | 256K          | 256K          |
| RAM   | 32K           | 64K           | 96K           |
| Pack  | lqfp48        | lqfp48        | lqfp64        |

## MCU 资料

[资料入口/WIKI](https://wiki.luatos.com/chips/air32f103/index.html) | [数据手册](https://cdn.openluat-luatcommunity.openluat.com/attachment/20220605164850945_AIR32F103%E8%8A%AF%E7%89%87%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8C1.0.0.pdf) | [与STM32的差异](https://wiki.luatos.com/chips/air32f103/switchFromSxx.html) | [Keil SDK 示例代码](https://gitee.com/openLuat/luatos-soc-air32f103)

## 合宙AIR32F103CBT6 开发板资料

[开发板原理图](https://cdn.openluat-luatcommunity.openluat.com/attachment/20220605164915340_AIR32CBT6.pdf) | [PCB交互图](https://wiki.luatos.com/_static/bom/Air32F103.html)

# 项目结构

```
├── Build                       # 编译结果
├── Examples                    # 示例代码
│   ├── FreeRTOS                  # FreeRTOS示例代码
│   └── NonFreeRTOS               # 非FreeRTOS示例代码
├── Libraries                   
│   ├── AIR32F10xLib            # AIR32F103外设层库代码
│   │   ├── inc                   # .h头文件
│   │   ├── lib
│   │   │   └── cryptlib
│   │   └── src                   # .c源文件
│   ├── CMSIS                   # Coretex M 核心外设层源文件
│   ├── Debug                   # delay和printf支持
│   ├── DeviceSupport           # AIR32F103的gcc arm startup文件
│   │   └── startup
│   │       └── arm-gcc
│   ├── FreeRTOS                # FreeRTOS 库代码
│   └── LDScripts               # 连接脚本
├── Makefile
├── Misc
│   └── flash.jlink             # JLink烧录脚本
├── README.cn.md
├── README.md
├── rules.mk                    # make规则
└── User                        # 用户项目代码
```

# 快速上手

## 1. 安装 GNU Arm Embedded Toolchain

从 [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) 下载工具链, 并解压到合适的目录
```bash
tar xvf gcc-arm-11.2-2022.02-x86_64-arm-none-eabi.tar.xz
cd /opt/gcc-arm/
sudo mv ~/Backup/linux/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/ .
sudo chown -R root:root gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/
```
## 2. 安装 SEGGER J-Link 或 ST-Flash

从 [J-Link / J-Trace Downloads](https://www.segger.com/downloads/jlink/) 下载 JLink并安装

```bash
# installation command for .deb
sudo dpkg -i JLink_Linux_V770a_x86_64.deb
```
默认的安装目录为 */opt/SEGGER*

## 3. 导出本项目

```bash
git clone https://github.com/IOsetting/hk32f030m-template.git
```

## 4. 根据本地环境修改 Makefile

修改 Makefile 设置, 确保 ARM_TOOCHAIN 和 JLINKEXE 指向正确的路径

```makefile
##### Project #####
# 项目名称
PROJECT 		?= app
# 编译结果目录
BUILD_DIR 		= Build

##### Options #####

# 是否使用 FreeRTOS, y:yes, n:no
USE_FREERTOS	?= n
# 烧录工具, jlink 或 stlink
FLASH_PROGRM    ?= stlink

##### Toolchains #######
# 根据本地环境, 设置工具链路径
ARM_TOOCHAIN 	?= /opt/gcc-arm/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin

# st-flash 可执行文件路径
ST_FLASH		?= st-flash
# JLinkExe 可执行文件路径和设备类型
JLINKEXE		?= /opt/SEGGER/JLink/JLinkExe
JLINK_DEVICE	?= STM32F103CB

##### Paths ############

# 当前芯片的连接脚本
LDSCRIPT		= Libraries/LDScripts/air32f103cbt6.ld
```
## 5. 编译并烧录

```bash
# 清理
make clean
# 编译
make
# 带详细输出的编译
V=1 make
# 烧录
make flash
```

# 示例代码

示例代码位于 Examples 目录下, 如果需要运行示例代码, 将代码复制到 User 目录下覆盖原有文件, 然后清空后重新编译和烧录
