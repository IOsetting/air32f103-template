#include "st7735.h"
#include "debug.h"

static uint8_t SPI_Write_Then_Read(uint8_t data)
{
  uint8_t retry = 0;
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET && ++retry);
  SPI_I2S_SendData(SPI1, data);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET && ++retry);
  return SPI_I2S_ReceiveData(SPI1);
}

void ST7735_WriteCommand(const uint8_t cmd)
{
  ST7735_DC_LOW;
  SPI_Write_Then_Read(cmd);
  ST7735_DC_HIGH;
}

void ST7735_SendData(const uint8_t cmd)
{
  SPI_Write_Then_Read(cmd);
}

void ST7735_WriteData(const uint8_t *data, uint16_t len)
{
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  for(uint8_t i = 0; i < len; i ++)
  {
    SPI_I2S_SendData(SPI1, data[i]);
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
  }
}

void ST7735_WriteDataBurst(const uint16_t data, uint32_t len)
{
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  for(uint32_t i = 0; i < len * 2; i ++) 
  {
    SPI_I2S_SendData(SPI1, *((uint8_t *)&data + (i % 2)));
    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
  }
}

static void ST7735_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // column address set
    ST7735_WriteCommand(ST7735_CASET);
    x1 = x1 + ST7735_XSTART;
    x2 = x2 + ST7735_XSTART;
    uint8_t data[] = { x1 >> 8, x1 & 0xFF, x2 >> 8, x2 % 0xFF };
    ST7735_WriteData(data, sizeof(data));

    // row address set
    ST7735_WriteCommand(ST7735_RASET);
    y1 = y1 + ST7735_YSTART;
    y2 = y2 + ST7735_YSTART;
    data[0] = y1 >> 8;
    data[1] = y1 & 0xFF;
    data[2] = y2 >> 8;
    data[3] = y2 & 0xFF;
    ST7735_WriteData(data, sizeof(data));

    // write to RAM
    ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Fill(uint16_t color)
{
  uint32_t len = ST7735_WIDTH * ST7735_HEIGHT;
  ST7735_CS_LOW;
  ST7735_SetAddrWindow(0, 0, ST7735_WIDTH - 1, ST7735_HEIGHT - 1);
  ST7735_WriteDataBurst(color, len);
  ST7735_CS_HIGH;
}

static const uint8_t
    init_cmds_b[] = {                   // Init commands for 7735B screens
        18,                             // 18 commands in list:
        ST7735_SWRESET, ST7735_CMD_DELAY, //  1: Software reset, no args, w/delay
        50,                               //     50 ms delay
        ST7735_SLPOUT, ST7735_CMD_DELAY,  //  2: Out of sleep mode, no args, w/delay
        255,                              //     255 = max (500 ms) delay
        ST7735_COLMOD, 1|ST7735_CMD_DELAY,//  3: Set color mode, 1 arg + delay:
        0x05,                             //     16-bit color
        10,                               //     10 ms delay
        ST7735_FRMCTR1, 3|ST7735_CMD_DELAY, //  4: Frame rate control, 3 args + delay:
        0x00,                             //     fastest refresh
        0x06,                             //     6 lines front porch
        0x03,                             //     3 lines back porch
        10,                               //     10 ms delay
        ST7735_MADCTL,  1,                //  5: Mem access ctl (directions), 1 arg:
        0x08,                             //     Row/col addr, bottom-top refresh
        ST7735_DISSET5, 2,                //  6: Display settings #5, 2 args:
        0x15,                             //     1 clk cycle nonoverlap, 2 cycle gate
                                          //     rise, 3 cycle osc equalize
        0x02,                             //     Fix on VTL
        ST7735_INVCTR,  1,                //  7: Display inversion control, 1 arg:
        0x0,                              //     Line inversion
        ST7735_PWCTR1,  2|ST7735_CMD_DELAY, //  8: Power control, 2 args + delay:
        0x02,                             //     GVDD = 4.7V
        0x70,                             //     1.0uA
        10,                               //     10 ms delay
        ST7735_PWCTR2,  1,                //  9: Power control, 1 arg, no delay:
        0x05,                             //     VGH = 14.7V, VGL = -7.35V
        ST7735_PWCTR3,  2,                // 10: Power control, 2 args, no delay:
        0x01,                             //     Opamp current small
        0x02,                             //     Boost frequency
        ST7735_VMCTR1,  2|ST7735_CMD_DELAY, // 11: Power control, 2 args + delay:
        0x3C,                             //     VCOMH = 4V
        0x38,                             //     VCOML = -1.1V
        10,                               //     10 ms delay
        ST7735_PWCTR6,  2,                // 12: Power control, 2 args, no delay:
        0x11, 0x15,
        ST7735_GMCTRP1,16,                // 13: Gamma Adjustments (pos. polarity), 16 args + delay:
        0x09, 0x16, 0x09, 0x20,           //     (Not entirely necessary, but provides
        0x21, 0x1B, 0x13, 0x19,           //      accurate colors)
        0x17, 0x15, 0x1E, 0x2B,
        0x04, 0x05, 0x02, 0x0E,
        ST7735_GMCTRN1,16|ST7735_CMD_DELAY, // 14: Gamma Adjustments (neg. polarity), 16 args + delay:
        0x0B, 0x14, 0x08, 0x1E,           //     (Not entirely necessary, but provides
        0x22, 0x1D, 0x18, 0x1E,           //      accurate colors)
        0x1B, 0x1A, 0x24, 0x2B,
        0x06, 0x06, 0x02, 0x0F,
        10,                               //     10 ms delay
        ST7735_CASET,   4,                // 15: Column addr set, 4 args, no delay:
        0x00, 0x02,                       //     XSTART = 2
        0x00, 0x81,                       //     XEND = 129
        ST7735_RASET,   4,                // 16: Row addr set, 4 args, no delay:
        0x00, 0x02,                       //     XSTART = 1
        0x00, 0x81,                       //     XEND = 160
        ST7735_NORON, ST7735_CMD_DELAY,   // 17: Normal display on, no args, w/delay
        10,                               //     10 ms delay
        ST7735_DISPON, ST7735_CMD_DELAY,  // 18: Main screen turn on, no args, delay
        255                               //     255 = max (500 ms) delay
    },

    init_cmds_r[] = {                      // Init for 7735R, part 1 (red or green tab)
        15,                               // 15 commands in list:
        ST7735_SWRESET, ST7735_CMD_DELAY, //  1: Software reset, 0 args, w/delay
        150,                              //     150 ms delay
        ST7735_SLPOUT, ST7735_CMD_DELAY,  //  2: Out of sleep mode, 0 args, w/delay
        255,                              //     500 ms delay
        ST7735_FRMCTR1, 3,                //  3: Frame rate ctrl - normal mode, 3 args:
        0x05, 0x3C, 0x3C,                 //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR2, 3,                //  4: Frame rate control - idle mode, 3 args:
        0x05, 0x3C, 0x3C,                 //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
        ST7735_FRMCTR3, 6,                //  5: Frame rate ctrl - partial mode, 6 args:
        0x05, 0x3C, 0x3C,                 //     Dot inversion mode
        0x05, 0x3C, 0x3C,                 //     Line inversion mode
        ST7735_INVCTR , 1,                //  6: Display inversion ctrl, 1 arg, no delay:
        0x03,                             //     
        ST7735_PWCTR1 , 3,                //  7: Power control, 3 args, no delay:
        0x28,
        0x08,                             //     
        0x04,                             //     
        ST7735_PWCTR2 , 1,                //  8: Power control, 1 arg, no delay:
        0xC0,                             //     
        ST7735_PWCTR3 , 2,                //  9: Power control, 2 args, no delay:
        0x0D,                             //     
        0x00,                             //     
        ST7735_PWCTR4 , 2,                // 10: Power control, 2 args, no delay:
        0x8D,                             //
        0x2A,  
        ST7735_PWCTR5 , 2,                // 11: Power control, 2 args, no delay:
        0x8D, 0xEE,
        ST7735_VMCTR1 , 1,                // 12: Power control, 1 arg, no delay:
        0x1A,
        ST7735_INVOFF , 0,                // 13: Don't invert display, no args, no delay
        ST7735_MADCTL , 1,                // 14: Memory access control (directions), 1 arg:
        ST7735_ROTATION,                  //     
        ST7735_COLMOD , 1,                // 15: set color mode, 1 arg, no delay:
        0x05                              //     16-bit color
    },

    init_cmds2[] = {
        2,                                //  2 commands in list:
        ST7735_CASET, 4,                  //  1: Column addr set, 4 args, no delay:
        0x00, ST7735_XSTART,              //     XSTART
        0x00, ST7735_XSTART + ST7735_WIDTH - 1,  // XEND
        ST7735_RASET  , 4,                //  2: Row addr set, 4 args, no delay:
        0x00, ST7735_YSTART,              //     YSTART
        0x00, ST7735_YSTART + ST7735_HEIGHT - 1, // YEND
    },

    init_cmds3[] = {
        4,                                //  4 commands in list:
        ST7735_GMCTRP1, 16,               //  1: Magical unicorn dust, 16 args, no delay:
        0x04, 0x22, 0x07, 0x0A,
        0x2E, 0x30, 0x25, 0x2A,
        0x28, 0x26, 0x2E, 0x3A,
        0x00, 0x01, 0x03, 0x13,
        ST7735_GMCTRN1, 16,             //  2: Sparkles and rainbows, 16 args, no delay:
        0x04, 0x16, 0x06, 0x0D,
        0x2D, 0x26, 0x23, 0x27,
        0x27, 0x25, 0x2D, 0x3B,
        0x00, 0x01, 0x04, 0x13,
        ST7735_NORON, ST7735_CMD_DELAY, //  3: Normal display on, no args, w/delay
        10,                             //     10 ms delay
        ST7735_DISPON, ST7735_CMD_DELAY, // 4: Main screen turn on, no args w/delay
        100                             //     100 ms delay
    };

void ST7735_ExecuteCommandList(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    ST7735_CS_LOW;
    numCommands = *addr++;
    while (numCommands--)
    {
        uint8_t cmd = *addr++;
        ST7735_WriteCommand(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & ST7735_CMD_DELAY;
        numArgs &= ~ST7735_CMD_DELAY;
        if (numArgs)
        {
            ST7735_WriteData(addr, numArgs);
            addr += numArgs;
        }

        if (ms)
        {
            ms = *addr++;
            if (ms == 255)
                ms = 500;
            Delay_Ms(ms);
        }
    }
    ST7735_CS_HIGH;
}

void ST7735_Reset(void)
{
  ST7735_BL_LOW;
  ST7735_RESET_LOW;
  Delay_Ms(5);
  ST7735_RESET_HIGH;
  ST7735_BL_HIGH;
}

void ST7735_Init(void)
{
    ST7735_Reset();
    ST7735_ExecuteCommandList(init_cmds_r);
    ST7735_ExecuteCommandList(init_cmds2);
    ST7735_ExecuteCommandList(init_cmds3);
}
