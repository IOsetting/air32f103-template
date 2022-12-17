##### Project #####

PROJECT 		?= app
BUILD_DIR 		= Build


##### Options #####

# Enable printf float %f support, y:yes, n:no
ENABLE_PRINTF_FLOAT	?= y
# Build with FreeRTOS, y:yes, n:no
USE_FREERTOS	?= n
# Build with Helix MP3 lib, y:yes, n:no
USE_HELIX		?= n
# Programmer, jlink, stlink, cmsis-dap, pyocd
FLASH_PROGRM    ?= stlink


##### Toolchains #######

#ARM_TOOCHAIN 	?= /opt/gcc-arm/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin
ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
#ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-12.2.mpacbti-bet1-x86_64-arm-none-eabi/bin
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

# Link descript file for this chip
LDSCRIPT		= Libraries/LDScripts/air32f103cbt6.ld
# Library build flags
LIB_FLAGS       = USE_STDPERIPH_DRIVER

# C source folders
CDIRS	:= User \
		Libraries/CORE \
		Libraries/AIR32F10xLib/src \
		Libraries/Debug
# C source files (if there are any single ones)
CFILES := 

# ASM source folders
ADIRS	:= User
# ASM single files
AFILES	:= Libraries/DeviceSupport/startup/arm-gcc/startup_air32f10x.s

# Include paths
INCLUDES	:= Libraries/CORE \
			Libraries/AIR32F10xLib/inc \
			Libraries/Debug \
			User

ifeq ($(USE_FREERTOS),y)
CDIRS		+= Libraries/FreeRTOS \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3

CFILES		+= Libraries/FreeRTOS/portable/MemMang/heap_4.c

INCLUDES	+= Libraries/FreeRTOS/include \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3
endif

ifeq ($(USE_HELIX),y)
CDIRS		+= Libraries/Helix \
		Libraries/Helix/real

INCLUDES	+= Libraries/Helix \
		Libraries/Helix/pub \
		Libraries/Helix/real
endif

include ./rules.mk
