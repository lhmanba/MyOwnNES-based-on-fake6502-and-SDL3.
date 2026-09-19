#pragma once

#include <stdint.h>
#include "nes.h"

uint8_t bus_read(Nes *nes, uint16_t addr);

void bus_write(Nes *nes, uint16_t addr, uint8_t data);
