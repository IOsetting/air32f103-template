PROJECT 		?= app
#ARM_TOOCHAIN 	?= /opt/gcc-arm/gcc-arm-11.2-2022.02-x86_64-arm-none-eabi/bin
ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-eabi/bin
#ARM_TOOCHAIN 	?= /opt/gcc-arm/arm-gnu-toolchain-12.2.mpacbti-bet1-x86_64-arm-none-eabi/bin
# path to st-flash (or should be specified in PATH)
ST_FLASH		?= st-flash
BUILD_DIR 		= Build

# Link descript file for this chip
LDSCRIPT		= Libraries/LDScripts/air32f103cbt6.ld
# Library build flags
LIB_FLAGS       = USE_STDPERIPH_DRIVER
# Uncomment this to enable FreeRTOS support
#LIB_FLAGS       += USE_FREERTOS

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
AFILES	:= Libraries/CORE/startup_air32f10x_md.s

# Include paths
INCLUDES	:= Libraries/CORE \
			Libraries/AIR32F10xLib/inc \
			Libraries/Debug \
			User

ifneq ($(filter USE_FREERTOS,$(LIB_FLAGS)),)
CDIRS		+= Libraries/FreeRTOS \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3

CFILES		+= Libraries/FreeRTOS/portable/MemMang/heap_4.c

INCLUDES	+= Libraries/FreeRTOS/include \
			Libraries/FreeRTOS/portable/GCC/ARM_CM3
endif

include ./rules.mk
