##### Project #####

PROJECT 		?= app
BUILD_DIR 		= Build


##### Options #####

# Enable printf float %f support, y:yes, n:no
ENABLE_PRINTF_FLOAT	?= y
# Build with FreeRTOS, y:yes, n:no
USE_FREERTOS	?= n
# Build with CMSIS DSP functions, y:yes, n:no
USE_DSP			?= n
# Build with Helix MP3 lib, y:yes, n:no
USE_HELIX		?= n
# Programmer, jlink, stlink, cmsis-dap, pyocd
FLASH_PROGRM    ?= stlink


##### Toolchains #######

#ARM_TOOCHAIN 	?= /opt/gcc-arm/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin
#ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-eabi/bin
# path to st-flash
ST_FLASH		?= st-flash
# path to JLinkExe
JLINKEXE		?= /opt/SEGGER/JLink/JLinkExe
# options: STM32F103CB(for CBT6), STM32F103RC(for CCT6 and RPT6)
JLINK_DEVICE	?= STM32F103CB
# path to OpenOCD
OPENOCD			?= openocd
# path to PyOCD
PYOCD_EXE		?= pyocd
# options: air32f103cb, air32f103cc, air32f103rp
PYOCD_DEVICE	?= air32f103cb

##### Paths ############

# Link descript file for this chip
LDSCRIPT		= Libraries/LDScripts/air32f103cbt6.ld
# Library build flags
LIB_FLAGS       = USE_STDPERIPH_DRIVER

# C source folders
CDIRS	:= User \
		Libraries/AIR32F10xLib/src \
		Libraries/Debug
# C source files (if there are any single ones)
CFILES := 

# ASM source folders
ADIRS	:= User
# ASM single files
AFILES	:= Libraries/DeviceSupport/startup/arm-gcc/startup_air32f10x.s

# Include paths
INCLUDES	:= User \
			Libraries/CMSIS/Core/Include \
			Libraries/AIR32F10xLib/inc \
			Libraries/Debug \

##### Optional Libraries ############

ifeq ($(USE_FREERTOS),y)
CDIRS		+= Libraries/FreeRTOS \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3

CFILES		+= Libraries/FreeRTOS/portable/MemMang/heap_4.c

INCLUDES	+= Libraries/FreeRTOS/include \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3
endif

ifeq ($(USE_DSP),y)
CFILES 		+= Libraries/CMSIS/DSP/Source/BasicMathFunctions/BasicMathFunctions.c \
		Libraries/CMSIS/DSP/Source/BayesFunctions/BayesFunctions.c \
		Libraries/CMSIS/DSP/Source/CommonTables/CommonTables.c \
		Libraries/CMSIS/DSP/Source/ComplexMathFunctions/ComplexMathFunctions.c \
		Libraries/CMSIS/DSP/Source/ControllerFunctions/ControllerFunctions.c \
		Libraries/CMSIS/DSP/Source/DistanceFunctions/DistanceFunctions.c \
		Libraries/CMSIS/DSP/Source/FastMathFunctions/FastMathFunctions.c \
		Libraries/CMSIS/DSP/Source/FilteringFunctions/FilteringFunctions.c \
		Libraries/CMSIS/DSP/Source/InterpolationFunctions/InterpolationFunctions.c \
		Libraries/CMSIS/DSP/Source/MatrixFunctions/MatrixFunctions.c \
		Libraries/CMSIS/DSP/Source/QuaternionMathFunctions/QuaternionMathFunctions.c \
		Libraries/CMSIS/DSP/Source/StatisticsFunctions/StatisticsFunctions.c \
		Libraries/CMSIS/DSP/Source/SupportFunctions/SupportFunctions.c \
		Libraries/CMSIS/DSP/Source/SVMFunctions/SVMFunctions.c \
		Libraries/CMSIS/DSP/Source/TransformFunctions/TransformFunctions.c

INCLUDES	+= Libraries/CMSIS/DSP/PrivateInclude \
			Libraries/CMSIS/DSP/Include

endif

ifeq ($(USE_HELIX),y)
CDIRS		+= Libraries/Helix \
		Libraries/Helix/real

INCLUDES	+= Libraries/Helix \
		Libraries/Helix/pub \
		Libraries/Helix/real
endif

include ./rules.mk
