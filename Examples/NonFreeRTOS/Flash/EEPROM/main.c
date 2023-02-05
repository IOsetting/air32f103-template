/*
 * @Note Flash EEPROM Test
*/
#include <string.h>
#include <air32f10x_adc.h>
#include <eeprom.h>
#include "debug.h"

/**
 * flash start address, must not smaller than the size of this code
 * 0X08008000 = 32K, 0X08010000 = 64K, 0X08020000 = 128K
*/
#define FLASH_ADDR 0X08008000

/**
 * Data size > 2048 bytes
*/
const uint8_t test_text[] = 
    "Shall I compare thee to a summer's day? Thou art more lovely and more temperate. Rough winds do shake the darling buds of May, "
    "And summer's lease hath all too short a date. Sometime too hot the eye of heaven shines, And often is his gold complexion dimmed; "
    "And every fair from fair sometime declines, By chance, or nature's changing course, untrimmed; But thy eternal summer shall not fade, "
    "Nor lose possession of that fair thou ow'st, Nor shall death brag thou wand'rest in his shade, When in eternal lines to Time thou grow'st. "
    "So long as men can breathe, or eyes can see, So long lives this, and this gives life to thee.\r\n"
    "Let me not to the marriage of true minds, Admit impediments. Love is not love, Which alters when it alteration finds,"
    "Or bends with the remover to remove: O, no! it is an ever-fixed mark, That looks on tempests and is never shaken;"
    "It is the star to every wandering bark, Whose worth's unknown, although his height be taken. Love's not Time's fool, "
    "though rosy lips and cheeks, Within his bending sickle's compass come; Love alters not with his brief hours and weeks, "
    "But bears it out even to the edge of doom. If this be error, and upon me prov'd, I never writ, nor no man ever lov'd.\r\n"
    "My mistress' eyes are nothing like the sun; Coral is far more red than her lips' red; If snow be white, why then her breasts are dun; "
    "If hairs be wires, black wires grow on her head. I have seen roses damasked, red and white, But no such roses see I in her cheeks; "
    "And in some perfumes is there more delight, Than in the breath that from my mistress reeks. I love to hear her speak, yet well I know "
    "That music hath a far more pleasing sound; I grant I never saw a goddess go; My mistress when she walks treads on the ground."
    "And yet, by heaven, I think my love as rare As any she belied with false compare. \r\n"
    "When, in disgrace with fortune and men's eyes, I all alone beweep my outcast state, And trouble deaf heaven with my bootless cries, "
    "And look upon myself and curse my fate, Wishing me like to one more rich in hope, Featured like him, like him with friends possessed, "
    "Desiring this man's art and that man's scope, With what I most enjoy contented least; Yet in these thoughts myself almost despising, "
    "Haply I think on thee, and then my state, (Like to the lark at break of day arising From sullen earth) sings hymns at heaven's gate; "
    "For thy sweet love remembered such wealth brings That then I scorn to change my state with kings.\r\n";


int main(void)
{
    uint16_t size16 = sizeof(test_text)/2 + (sizeof(test_text)%2);
    uint16_t buff[size16], i;
    RCC_ClocksTypeDef clocks;

    Delay_Init();
    USART_Printf_Init(115200);
    RCC_GetClocksFreq(&clocks);

    printf("\n");
    printf("SYSCLK: %3.1fMhz, HCLK: %3.1fMhz, PCLK1: %3.1fMhz, PCLK2: %3.1fMhz, ADCCLK: %3.1fMhz\n",
           (float)clocks.SYSCLK_Frequency / 1000000, (float)clocks.HCLK_Frequency / 1000000,
           (float)clocks.PCLK1_Frequency / 1000000, (float)clocks.PCLK2_Frequency / 1000000, 
		   (float)clocks.ADCCLK_Frequency / 1000000);
    printf("AIR32F103 Flash EEPROM Test\n");

    AIRFLASH_Write(FLASH_ADDR, (uint16_t *)test_text, size16);   // Write to flash
    memset(buff, 0x0000, size16);
    AIRFLASH_Read(FLASH_ADDR, (uint16_t *)buff, size16);         // Read from flash
    for (i = 0; i < size16; i++)
    {
        printf("%c%c", buff[i] & 0xFf, buff[i] >> 8);
    }
    printf("Done.\r\n");

    while (1);
}
