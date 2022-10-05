# FreeRTOS

Current version: FreeRTOSv202112.00

## Build With FreeRTOS

Edit Makefile, uncomment following line will enable FreeRTOS support
```
#LIB_FLAGS       += USE_FREERTOS
```
By default `heap_4.c` is selected, edit it if you want to switch to other memory management
```
CFILES		+= Libraries/FreeRTOS/portable/MemMang/heap_4.c
```
