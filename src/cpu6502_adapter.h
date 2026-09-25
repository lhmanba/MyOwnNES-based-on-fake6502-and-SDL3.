#pragma once
#include <stdint.h>
struct  Nes;


typedef struct Cpu6502state
{
    uint16_t pc;

    uint8_t a;
    uint8_t x;
    uint8_t y;

    uint8_t sp;
    uint8_t p;
}Cpu6502state;

void cpu6502_bind(struct Nes *nes);

uint32_t cpu6502_step(void);

uint16_t cpu6502_get_pc(void);

Cpu6502state cpu6502_get_state(void);

uint32_t cpu6502_step_and_sync(struct Nes *nes);

void cpu6502_reset(void);
