#include "controller.h"

void controller_reset(Controller *pad)
{
    pad->buttons =0;
    pad->shift =0;
    pad->strobe = false;
}

void controller_write(Controller *pad,uint8_t value)
{
    bool new_strobe=(value & 1u)!=0;
    pad -> strobe=new_strobe;
    if(new_strobe)
    {
        pad->shift=pad->buttons;
    }
}

uint8_t controller_read(Controller *pad)
{
    if(pad->strobe)
    {
        pad->shift = pad->buttons;
    }

    uint8_t bit=pad->shift & 1u;
    printf("4016 read: %u\n", bit);
    if(!pad->strobe)
    {
        pad->shift=(uint8_t)((pad->shift >> 1) | 0x80u);
    }
    return (uint8_t)(0x40u | bit);
}