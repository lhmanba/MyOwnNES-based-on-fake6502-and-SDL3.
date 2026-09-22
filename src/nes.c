#include "nes.h"
#include "bus.h"

void nes_oam_dma(Nes*nes,uint8_t page)
{
    uint16_t base=(uint16_t)(page<<8);
    for(uint16_t i =0;i<256;++i)
    {
        uint8_t value = bus_read(nes,(uint16_t)(base+i));
        nes->ppu.oam[nes->ppu.oam_addr++]=value;
    }

    nes->cpu_cycles+=513;
}