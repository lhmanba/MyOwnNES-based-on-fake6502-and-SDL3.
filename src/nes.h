#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cartridge.h"

typedef struct Nes
{
    uint8_t ram[2u * 1024u]; // 2KB RAM

    Cartridge cart;

    uint64_t cpu_cycles;

    bool quit;
    
}Nes;