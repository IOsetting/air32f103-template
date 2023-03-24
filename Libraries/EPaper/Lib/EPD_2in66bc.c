/*****************************************************************************
* | File      	:   EPD_2in66bc.c
* | Author      :   
* | Function    :   2.66inch e-paper b&c
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2023-03-25
* | Info        :
* -----------------------------------------------------------------------------
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include "EPD_2in66bc.h"

#ifdef EPD_2IN66BC

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_2IN66BC_Reset(void)
{
    EPD_Digital_Write(EPD_RST_PIN, 1);
    EPD_Delay_ms(200);
    EPD_Digital_Write(EPD_RST_PIN, 0);
    EPD_Delay_ms(2);
    EPD_Digital_Write(EPD_RST_PIN, 1);
    EPD_Delay_ms(200);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_2IN66BC_SendCommand(UBYTE Reg)
{
    EPD_Digital_Write(EPD_DC_PIN, 0);
    EPD_Digital_Write(EPD_CS_PIN, 0);
    EPD_SPI_WriteByte(Reg);
    EPD_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_2IN66BC_SendData(UBYTE Data)
{
    EPD_Digital_Write(EPD_DC_PIN, 1);
    EPD_Digital_Write(EPD_CS_PIN, 0);
    EPD_SPI_WriteByte(Data);
    EPD_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD_2IN66BC_ReadBusy(void)
{
    EPD_Printf("e-Paper busy");
    while(EPD_Digital_Read(EPD_BUSY_PIN) == 0) {
        EPD_Printf(".");
        EPD_Delay_ms(100);
    }
    EPD_Printf("idle\r\n");
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_2IN66BC_TurnOnDisplay(void)
{
    EPD_2IN66BC_SendCommand(0x12);		 //DISPLAY REFRESH
    EPD_Delay_ms(10);

    EPD_2IN66BC_ReadBusy();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_2IN66BC_Init(void)
{
    EPD_2IN66BC_Reset();

    EPD_2IN66BC_SendCommand(0x06); // BOOSTER_SOFT_START
    EPD_2IN66BC_SendData(0x17);
    EPD_2IN66BC_SendData(0x17);
    EPD_2IN66BC_SendData(0x17);
	
    EPD_2IN66BC_SendCommand(0x04); // POWER_ON
    EPD_2IN66BC_ReadBusy();
	
    EPD_2IN66BC_SendCommand(0x00); // PANEL_SETTING
    EPD_2IN66BC_SendData(0x8F);
	
    EPD_2IN66BC_SendCommand(0x50); // VCOM_AND_DATA_INTERVAL_SETTING
    EPD_2IN66BC_SendData(0xF0);
    EPD_2IN66BC_SendCommand(0x61); // RESOLUTION_SETTING
    EPD_2IN66BC_SendData(EPD_2IN66BC_WIDTH); // width: 104
    EPD_2IN66BC_SendData(EPD_2IN66BC_HEIGHT >> 8); // height: 212
    EPD_2IN66BC_SendData(EPD_2IN66BC_HEIGHT & 0xFF);
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_2IN66BC_Clear(void)
{
    UWORD Width = (EPD_2IN66BC_WIDTH % 8 == 0)? (EPD_2IN66BC_WIDTH / 8 ): (EPD_2IN66BC_WIDTH / 8 + 1);
    UWORD Height = EPD_2IN66BC_HEIGHT;
    
    //send black data
    EPD_2IN66BC_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN66BC_SendData(0xFF);
        }
    }
    EPD_2IN66BC_SendCommand(0x92); 

    //send red data
    EPD_2IN66BC_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN66BC_SendData(0xFF);
        }
    }
    EPD_2IN66BC_SendCommand(0x92); 
    
    EPD_2IN66BC_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_2IN66BC_Display(const UBYTE *blackimage, const UBYTE *ryimage)
{
    UWORD Width, Height;
    Width = (EPD_2IN66BC_WIDTH % 8 == 0)? (EPD_2IN66BC_WIDTH / 8 ): (EPD_2IN66BC_WIDTH / 8 + 1);
    Height = EPD_2IN66BC_HEIGHT;
    
    EPD_2IN66BC_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN66BC_SendData(blackimage[i + j * Width]);
        }
    }
    EPD_2IN66BC_SendCommand(0x92); 
    
    EPD_2IN66BC_SendCommand(0x13);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2IN66BC_SendData(ryimage[i + j * Width]);
        }
    }
    EPD_2IN66BC_SendCommand(0x92); 
    
    EPD_2IN66BC_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_2IN66BC_Sleep(void)
{
    EPD_2IN66BC_SendCommand(0x02); // POWER_OFF
    EPD_2IN66BC_ReadBusy();
    EPD_2IN66BC_SendCommand(0x07); // DEEP_SLEEP
    EPD_2IN66BC_SendData(0xA5); // check code
}

#endif
