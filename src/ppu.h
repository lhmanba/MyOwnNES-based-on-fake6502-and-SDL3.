#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cartridge.h"

typedef struct Ppu
{
    Cartridge *cart;

    uint8_t ctrl;//$2000 PPUCTRL
    uint8_t mask;//$2001 PPUMASK
    uint8_t status;//$2002 PPUSTATUS
    uint8_t oam_addr;//$2003 OAMADDR

    uint8_t oam[256];//$2004

    uint8_t nametable[2048];
    uint8_t palette[32];

    uint16_t v;
    uint16_t t;

    uint8_t fine_x;
    bool write_toggle;

    uint8_t read_buffer;

    int scanline;
    int dot;

    bool nmi_pending;

    uint32_t frame[256*240];

}Ppu;

uint8_t ppu_bus_read(Ppu *ppu,uint16_t addr);

void ppu_bus_write(Ppu *ppu,uint16_t addr,uint8_t data);

void ppu_cpu_write(Ppu*ppu,uint8_t reg,uint8_t value);

uint8_t ppu_cpu_read(Ppu*ppu,uint8_t reg);

void ppu_render_frame(Ppu *ppu);

void ppu_tick(Ppu *ppu);

void ppu_reset(Ppu *ppu);