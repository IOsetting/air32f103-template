This demo requires 225 KBytes flash, not for AIR32F103CBT6.

Following changes are needed to run this example:

1. Edit Makefile

Change LDSCRIPT to `air32f103cct6.ld`
```
# Link descript file for this chip
LDSCRIPT		= Libraries/LDScripts/air32f103cct6.ld
```

Set USE_LVGL to `y`
```
# Build with lvgl, y:yes, n:no
USE_LVGL		?= y
```

2. Edit rules.mk, set OPT to `-O3`

```c
OPT			?= -O3
```
