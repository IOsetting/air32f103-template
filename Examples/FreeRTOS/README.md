# FreeRTOS

Current version: FreeRTOSv202112.00

## Build With FreeRTOS

Edit Makefile, change `USE_FREERTOS	?= n` to `USE_FREERTOS	?= y` will enable FreeRTOS support
```
# Build with FreeRTOS, y:yes, n:no
USE_FREERTOS	?= y
```
By default `heap_4.c` is selected, edit it if you want to switch to other memory management
```
CFILES		+= Libraries/FreeRTOS/portable/MemMang/heap_4.c
```
