#include "avg.h"

uint16_t avg16(uint16_t *array, uint32_t size)
{
    uint16_t value = 0, tmp, sub;
    uint32_t total = 0, mod = 0;

    if (size == 0) return 0;

    while(size--)
    {
        uint16_t current = *array++;
        total++;
        if (current >= value)
        {
            mod += current - value;
            value += mod / total;
            mod %= total;
        }
        else
        {
            tmp = value - current;
            if (mod >= tmp)
            {
                mod -= tmp;
            }
            else
            {
                tmp = tmp - mod;
                if (tmp % total == 0)
                {
                    sub = tmp / total;
                }
                else
                {
                    sub = (tmp / total) + 1;
                }
                value -= sub;
                mod = (sub * total) - tmp;
            }
        }
    }
    return (mod > (total / 2))? value + 1: value;
}

uint32_t avg32(uint32_t *array, uint32_t size)
{
    uint32_t total = 0, value = 0, mod = 0, tmp, sub;

    if (size == 0) return 0;

    while(size--)
    {
        uint32_t current = *array++;
        total++;
        if (current >= value)
        {
            mod += current - value;
            value += mod / total;
            mod %= total;
        }
        else
        {
            tmp = value - current;
            if (mod >= tmp)
            {
                mod -= tmp;
            }
            else
            {
                tmp = tmp - mod;
                if (tmp % total == 0)
                {
                    sub = tmp / total;
                }
                else
                {
                    sub = (tmp / total) + 1;
                }
                value -= sub;
                mod = (sub * total) - tmp;
            }
        }
    }
    return (mod > (total / 2))? value + 1: value;
}