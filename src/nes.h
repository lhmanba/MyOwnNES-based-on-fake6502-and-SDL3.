#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cartridge.h"
#include "ppu.h"

typedef struct Nes
{
    uint8_t ram[2u * 1024u]; // 2KB RAM

    Cartridge cart;
    Ppu ppu;

    uint64_t cpu_cycles;
    bool quit;
    
}Nes;

void nes_oam_dma(Nes *nes,uint8_t page);