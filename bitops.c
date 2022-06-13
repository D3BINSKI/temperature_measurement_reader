//
// Created by debin on 26.05.2022.
//

#include <stdint.h>
#include "bitops.h"

int16_t lfj14(int16_t x)
{
    return (int16_t)(x<<2);
}

uint16_t right_justifyu16(uint16_t x)
{
    while ((x & 0x0001) == 0)
        x >>= 1;
    return x;
}
uint32_t right_justifyu32(uint32_t x){
    while ((x & 0x00000001) == 0)
        x >>= 1;
    return x;
}

int16_t right_justify16(int16_t x){
    if((x&0x2000) != 0) return x;
    while ((x & 0x0001) == 0)
        x >>= 1;
    return x;
}
int32_t right_justify32(int32_t x){
    if((x&0x80000000) != 0) return x;
    while ((x & 0x00000001) == 0)
        x >>= 1;
    return x;
}

uint16_t left_justifyu16(uint16_t x)
{
    while ((x & 0x8000) == 0)
        x <<= 1;
    return x;
}

uint32_t left_justifyu32(uint32_t x)
{
    while ((x & 0x80000000) == 0)
        x <<= 1;
    return x;
}

int16_t left_justify16(int16_t x)
{
    if((x&0x8000) != 0) return x;
    while ((x & 0x0800) == 0) x <<= 1;
    return x;
}

int32_t left_justify32(int32_t x)
{
    if((x&0x80000000) != 0) return x;
    while ((x & 0x08000000) == 0) x <<= 1;
    return x;
}