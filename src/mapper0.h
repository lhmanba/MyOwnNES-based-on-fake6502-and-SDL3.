#pragma once

#include <stdint.h>
#include <cartridge.h>

uint8_t mapper0_cpu_read(const Cartridge *cart, uint16_t addr);

uint8_t mapper0_ppu_read(const Cartridge *cart, uint16_t addr);

void mapper0_ppu_write(Cartridge *cart, uint16_t addr, uint8_t data);