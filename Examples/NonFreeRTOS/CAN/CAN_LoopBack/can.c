#include "can.h"

/**
 * CAN Initializaion
 * 
 * tsjw: synchronisation jump width, CAN_SJW_1tq - CAN_SJW_4tq
 * tbs2: time quantum in bit segment 2, CAN_BS2_1tq - CAN_BS2_8tq
 * tbs1: time quantum in bit segment 1, CAN_BS1_1tq - CAN_BS1_16tq
 * brp:  clock prescaler. 1~1024, tq = (brp)*tpclk1, baudrate = Fpclk1/((tbs1+1+tbs2+1+1)*brp);
 * mode CAN mode, normal, loop back, silent or silent loop back
 * 
 * When frequency of PCLK1 = 36M, 
 * - CAN_Mode_Init(CAN_SJW_1tq, CAN_BS2_8tq, CAN_BS1_9tq, 4, x) -> baud = 36M / ((8+9+1) * 4) = 500Kbps
*/
uint8_t CAN_Mode_Init(uint8_t tsjw, uint8_t tbs2, uint8_t tbs1, uint16_t brp, uint8_t mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  CAN_InitTypeDef CAN_InitStructure;
  CAN_FilterInitTypeDef CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE
  NVIC_InitTypeDef NVIC_InitStructure;
#endif

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

  GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
  // PB9 CANTX, output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  // PB8 CANRX, input
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  CAN_InitStructure.CAN_TTCM = DISABLE; // time triggered communication mode off
  CAN_InitStructure.CAN_ABOM = DISABLE; // automatic bus-off management off
  CAN_InitStructure.CAN_AWUM = DISABLE; // automatic wake-up mode off, wakeup by software cleaar CAN->MCR SLEEP bit
  CAN_InitStructure.CAN_NART = ENABLE;  // no-automatic retransmission mode on
  CAN_InitStructure.CAN_RFLM = DISABLE; // rx FIFO Locked mode off
  CAN_InitStructure.CAN_TXFP = DISABLE; // transmit FIFO priority off
  CAN_InitStructure.CAN_Mode = mode;
  // Set baud rate
  CAN_InitStructure.CAN_SJW = tsjw;      // synchronisation_jump_width = tsjw+1
  CAN_InitStructure.CAN_BS1 = tbs1;      // number of time quanta in Bit Segment 1 = tbs1+1
  CAN_InitStructure.CAN_BS2 = tbs2;      // number of time quanta in Bit Segment 2 = tbs2+1
  CAN_InitStructure.CAN_Prescaler = brp; // clock prescaler = brp+1
  CAN_Init(CAN1, &CAN_InitStructure);

  // CAN filter
  CAN_FilterInitStructure.CAN_FilterNumber = 0;                    // the filter to be initialized, ranges from 0 to 13
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  // filter mode to be initialized
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // filter scale 32 bit
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;               // filter identification number, MSBs for a 32-bit configuration, 
                                                                   // first one for a 16-bit configuration
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;           // filter mask number or identification number
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; // the FIFO (0 or 1) which will be assigned to the filter
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

  CAN_FilterInit(&CAN_FilterInitStructure);

#if CAN_RX0_INT_ENABLE
  CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE); // Enable interrupt

  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
  return 0;
}

#if CAN_RX0_INT_ENABLE
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  int i = 0;
  CanRxMsg RxMessage;

  CAN_Receive(CAN1, 0, &RxMessage);
  for (i = 0; i < 8; i++)
  {
    printf("rxbuf[%d]:%d\r\n", i, RxMessage.Data[i]);
  }
}
#endif

/**
 * Send data with fixed ID:0X12, data frame
*/
uint8_t CAN_SendData(uint8_t *msg, uint8_t len)
{
  uint8_t mbox;
  uint16_t i = 0;
  CanTxMsg txmessage;
  txmessage.StdId = 0x12;           // standard identifier
  txmessage.ExtId = 0x12;           // extended identifier
  txmessage.IDE = CAN_Id_Standard;  // type of identifier
  txmessage.RTR = CAN_RTR_Data;     // type of frame for the message
  txmessage.DLC = len;              // data length
  for (i = 0; i < len; i++)
  {
    txmessage.Data[i] = msg[i];
  }

  mbox = CAN_Transmit(CAN1, &txmessage);
  i = 0;
  while ((CAN_TransmitStatus(CAN1, mbox) == CAN_TxStatus_Failed) && (i < 0XFFF))
  {
    i++;
  }
  if (i >= 0XFFF)
  {
    return 1;
  }
  return 0;
}

/**
 * CAN receive data from buffer
*/
uint8_t CAN_ReceiveData(uint8_t *buf)
{
  uint32_t i;
  CanRxMsg rxMessage;

  if (CAN_MessagePending(CAN1, CAN_FIFO0) == 0)
  {
    return 0;
  }

  CAN_Receive(CAN1, CAN_FIFO0, &rxMessage);
  for (i = 0; i < 8; i++)
  {
    buf[i] = rxMessage.Data[i];
  }
  return rxMessage.DLC;
}
