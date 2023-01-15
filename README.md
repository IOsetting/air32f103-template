# About

A build template for projects using AIR32F103x MCU and GNU Arm Embedded Toolchain.

# AIR32F103x Types

| TYPE  | AIR32F103CBT6 | AIR32F103CCT6 | AIR32F103RPT6 |
| ----- | ------------- | ------------- | ------------- |
| Flash | 128K          | 256K          | 256K          |
| RAM   | 32K           | 64K           | 96K           |
| Pack  | lqfp48        | lqfp48        | lqfp64        |

## Documents

[WIKI](https://wiki.luatos.com/chips/air32f103/index.html) | [Datasheet](https://cdn.openluat-luatcommunity.openluat.com/attachment/20220605164850945_AIR32F103%E8%8A%AF%E7%89%87%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8C1.0.0.pdf) | [Migrate From STM32](https://wiki.luatos.com/chips/air32f103/switchFromSxx.html) | [Keil SDK And Demos](https://gitee.com/openLuat/luatos-soc-air32f103)

## LuatOS AIR32F103CBT6 EVB

[EVB Schematis](https://cdn.openluat-luatcommunity.openluat.com/attachment/20220605164915340_AIR32CBT6.pdf) | [PCB](https://wiki.luatos.com/_static/bom/Air32F103.html)

# Project Structure

```
├── Build                       # Build results
├── Examples                    # Example code
│   ├── FreeRTOS                  # FreeRTOS examples
│   └── NonFreeRTOS               # Non-FreeRTOS examples
├── Libraries                   
│   ├── AIR32F10xLib            # AIR32F103 Peripheral library
│   │   ├── inc                 
│   │   ├── lib
│   │   │   └── cryptlib
│   │   └── src                 
│   ├── CMSIS                   # Coretex M core files
│   ├── Debug                   # For delay and printf support
│   ├── DeviceSupport           # gcc arm startup files
│   │   └── startup
│   │       └── arm-gcc
│   ├── FreeRTOS                # FreeRTOS library
│   └── LDScripts               # Link scripts
├── Makefile
├── Misc
│   └── flash.jlink             # JLink script
├── README.cn.md
├── README.md
├── rules.mk                    # Make rules
└── User                        # User application code
```

# Quick Start

## 1. Install GNU Arm Embedded Toolchain

Download toolchains from [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) and uncompress 
```bash
tar xvf gcc-arm-11.2-2022.02-x86_64-arm-none-eabi.tar.xz
cd /opt/gcc-arm/
sudo mv ~/Backup/linux/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/ .
sudo chown -R root:root gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/
```
## 2. Intall SEGGER J-Link Or ST-Flash

Download JLink from [J-Link / J-Trace Downloads](https://www.segger.com/downloads/jlink/) and install it

```bash
# installation command for .deb
sudo dpkg -i JLink_Linux_V770a_x86_64.deb
```
The default installation path is */opt/SEGGER*

## 3. Check Out This Repository

```bash
git clone https://github.com/IOsetting/hk32f030m-template.git
```

## 4. Edit Makefile

Edit Makefile, make sure ARM_TOOCHAIN and JLINKEXE point to the correct paths

```makefile
##### Project #####
# Project name
PROJECT 		?= app
# Result folder
BUILD_DIR 		= Build

##### Options #####

# Enable printf float %f support, y:yes, n:no
ENABLE_PRINTF_FLOAT	?= y
# Build with FreeRTOS, y:yes, n:no
USE_FREERTOS	?= n
# Programmer, jlink, stlink, cmsis-dap, pyocd
FLASH_PROGRM    ?= stlink

##### Toolchains #######

ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-12.2.mpacbti-bet1-x86_64-arm-none-eabi/bin
# path to st-flash
ST_FLASH		?= st-flash
# path to JLinkExe
JLINKEXE		?= /opt/SEGGER/JLink/JLinkExe
JLINK_DEVICE	?= STM32F103CB
# path to OpenOCD
OPENOCD			?= openocd
# path to PyOCD
PYOCD_EXE		?= pyocd
PYOCD_DEVICE	?= air32f103cb

##### Paths ############

# Link script for current chip
LDSCRIPT		= Libraries/LDScripts/air32f103cbt6.ld
```
## 5. Compile and Flash

```bash
# Clean
make clean
# Build
make
# Build verbosely
V=1 make
# Flash
make flash
```

# Examples

More examples can be found in Examples folder, copy and replace the files under User folder to try different examples.
