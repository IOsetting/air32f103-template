#ifndef __ST7735_H
#define __ST7735_H			    
#include "air32f10x.h"

// CS
#define ST7735_CS_PORT      GPIOA
#define ST7735_CS_PIN       GPIO_Pin_3
// SCK
#define ST7735_SCK_PORT     GPIOA
#define ST7735_SCK_PIN      GPIO_Pin_5
// MOSI
#define ST7735_MOSI_PORT    GPIOA
#define ST7735_MOSI_PIN     GPIO_Pin_7
// RESET
#define ST7735_RES_PORT     GPIOA
#define ST7735_RES_PIN      GPIO_Pin_6
// DATA/COMMAND
#define ST7735_DC_PORT      GPIOA
#define ST7735_DC_PIN       GPIO_Pin_4
// BACK LIGHT
#define ST7735_BL_PORT      GPIOA
#define ST7735_BL_PIN       GPIO_Pin_2

#define ST7735_CS_LOW           GPIO_ResetBits(ST7735_CS_PORT, ST7735_CS_PIN)
#define ST7735_CS_HIGH          GPIO_SetBits(ST7735_CS_PORT, ST7735_CS_PIN)
#define ST7735_BL_LOW           GPIO_ResetBits(ST7735_BL_PORT, ST7735_BL_PIN)
#define ST7735_BL_HIGH          GPIO_SetBits(ST7735_BL_PORT, ST7735_BL_PIN)
#define ST7735_DC_LOW           GPIO_ResetBits(ST7735_DC_PORT, ST7735_DC_PIN)
#define ST7735_DC_HIGH          GPIO_SetBits(ST7735_DC_PORT, ST7735_DC_PIN)
#define ST7735_RESET_LOW        GPIO_ResetBits(ST7735_RES_PORT, ST7735_RES_PIN)
#define ST7735_RESET_HIGH       GPIO_SetBits(ST7735_RES_PORT, ST7735_RES_PIN)

#define ST7735_TYPEB            0
#define ST7735_TYPER            1

// ST7789V-based 2.4" display, default orientation
/*
#define ST7735_WIDTH  240
#define ST7735_HEIGHT 320
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB)
*/

// ST7735-based 1.8" display, default orientation
/*
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY)
*/

// ST7735-based 1.8" display, rotate right
/*
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV)
*/

// ST7735-based 1.8" display, rotate left
/*
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV)
*/

// ST7735-based 1.8" display, upside down
/*
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (0)
*/

// WaveShare ST7735S-based 1.8" display, default orientation

#define ST7735_TYPE   ST7735_TYPER
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ST7735_XSTART 2
#define ST7735_YSTART 1
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB)


// WaveShare ST7735S-based 1.8" display, rotate right
/*
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128
#define ST7735_XSTART 1
#define ST7735_YSTART 2
#define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB)
*/

// WaveShare ST7735S-based 1.8" display, rotate left
/*
#define ST7735_WIDTH  160
#define ST7735_HEIGHT 128
#define ST7735_XSTART 1
#define ST7735_YSTART 2
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB)
*/

// WaveShare ST7735S-based 1.8" display, upside down
/*
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ST7735_XSTART 2
#define ST7735_YSTART 1
#define ST7735_ROTATION (ST7735_MADCTL_RGB)
*/

#define ST7735_CMD_DELAY 0x80 // special signifier for command lists

#define ST7735_NOP       0x00
#define ST7735_SWRESET   0x01
#define ST7735_RDDID     0x04
#define ST7735_RDDST     0x09

#define ST7735_SLPIN     0x10
#define ST7735_SLPOUT    0x11
#define ST7735_PTLON     0x12
#define ST7735_NORON     0x13

#define ST7735_INVOFF    0x20
#define ST7735_INVON     0x21
#define ST7735_DISPOFF   0x28
#define ST7735_DISPON    0x29
#define ST7735_CASET     0x2A
#define ST7735_RASET     0x2B
#define ST7735_RAMWR     0x2C
#define ST7735_RAMRD     0x2E

#define ST7735_PTLAR     0x30
#define ST7735_TEOFF     0x34
#define ST7735_TEON      0x35
#define ST7735_MADCTL    0x36
#define ST7735_COLMOD    0x3A

#define ST7735_MADCTL_MY 0x80
#define ST7735_MADCTL_MX 0x40
#define ST7735_MADCTL_MV 0x20
#define ST7735_MADCTL_ML 0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

#define ST7735_RDID1     0xDA
#define ST7735_RDID2     0xDB
#define ST7735_RDID3     0xDC
#define ST7735_RDID4     0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST7735_BLACK     0x0000
#define ST7735_WHITE     0xFFFF
#define ST7735_RED       0x00F8
#define ST7735_GREEN     0xE007
#define ST7735_BLUE      0x1F00
#define ST7735_CYAN      0xFF07
#define ST7735_MAGENTA   0x1FF8
#define ST7735_YELLOW    0xE0FF
#define ST7735_ORANGE    0x00FC
#define ST7735_BROWN     0X40BC

#define ST7735_FRMCTR1    0xB1
#define ST7735_FRMCTR2    0xB2
#define ST7735_FRMCTR3    0xB3
#define ST7735_INVCTR     0xB4
#define ST7735_DISSET5    0xB6

#define ST7735_PWCTR1     0xC0
#define ST7735_PWCTR2     0xC1
#define ST7735_PWCTR3     0xC2
#define ST7735_PWCTR4     0xC3
#define ST7735_PWCTR5     0xC4
#define ST7735_VMCTR1     0xC5

#define ST7735_PWCTR6     0xFC

#define ST7735_GMCTRP1    0xE0
#define ST7735_GMCTRN1    0xE1

//offset
#define LCD_OFFSET_X 1
#define LCD_OFFSET_Y 2

void ST7735_Init(void);
void ST7735_WriteData(const uint8_t *data, uint16_t len);
void ST7735_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void ST7735_FillAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

#endif
