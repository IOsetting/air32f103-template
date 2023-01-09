# Summary of DMA1 requests for each channel

| Peripherals | Channel 1 | Channel 2 |        Channel 3 |                   Channel 4 |    Channel 5 |          Channel 6 |         Channel 7 |
|      ------ |     ----- |     ----- |            ----- |                       ----- |        ----- |              ----- |              ---- |
|        ADC1 |      ADC1 |         - |                - |                           - |            - |                  - |                 - |
|     SPI/I2S |         - |   SPI1_RX |          SPI1_TX |                SPI2/I2S2_RX | SPI2/I2S2_TX |                  - |                 - |
|       USART |         - | USART3_TX |        USART3_RX |                   USART1_TX |    USART1_RX |          USART2_RX |         USART2_TX |
|         I2C |         - |         - |                  |                     I2C2_TX |      I2C2_RX |            I2C1_TX |           I2C1_RX |
|        TIM1 |         - |  TIM1_CH1 |                - | TIM1_CH4 TIM1_TRIG TIM1_COM |      TIM1_UP |           TIM1_CH3 |                 - |
|        TIM2 |  TIM2_CH3 |   TIM2_UP |                - |                           - |     TIM2_CH1 |                  - | TIM2_CH2 TIM2_CH4 |
|        TIM3 |         - |  TIM3_CH3 | TIM3_CH4 TIM3_UP |                           - |            - | TIM3_CH1 TIM3_TRIG |                 - |
|        TIM4 |  TIM4_CH1 |         - |                - |                    TIM4_CH2 |     TIM4_CH3 |                  - |           TIM4_UP |

# Summary of DMA2 requests for each channel

|       Peripherals |          Channel 1 |                   Channel 2 |            Channel 3 |            Channel 4 | Channel 5 |
|            ------ |             ------ |                      ------ |               ------ |               ------ |    ------ |
|              ADC3 |                  - |                           - |                    - |                    - |      ADC3 |
|          SPI/I2S3 |        SPI/I2S3_RX |                 SPI/I2S3_TX |                    - |                    - |         - |
|             UART4 |                  - |                           - |             UART4_RX |                    - |  UART4_TX |
|              SDIO |                  - |                           - |                    - |                 SDIO |         - |
|              TIM5 | TIM5_CH4 TIM5_TRIG |            TIM5_CH3 TIM5_UP |                    - |             TIM5_CH2 |  TIM5_CH1 |
| TIM6/DAC_Channel1 |                  - |                           - | TIM6_UP/DAC_Channel1 |                    - |         - |
|              TIM7 |                  - |                           - |                    - | TIM7_UP/DAC_Channel2 |         - |
|              TIM8 |   TIM8_CH3 TIM8_UP | TIM8_CH4 TIM8_TRIG TIM8_COM |             TIM8_CH1 |                    - |  TIM8_CH2 |

# Problems

## 8-bit ADC DMA workaround doesn't work

On STM32 you can set ADC result left-aligned and use '(uint32_t)&ADC1->DR + 1' to set the 8-bit array in DMA, but this doesn't work on AIR32, AIR32 doesn't support unaligned 32-bit memory access.

# 96K RAM Hack

Thanks to @deividAlfa, RAM hack has been integrated into startup. Nothing need to change.
