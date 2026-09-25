#include "cpu6502_adapter.h"
#include "bus.h"
#include "nes.h"
#include <assert.h>
#include <stdint.h>

#include "fake6502.h"

static Nes *g_nes = NULL;//直接通过g_nes模拟器找到模拟器，为static类型。

void cpu6502_bind(Nes *nes)
{
    g_nes = nes;
}

uint8 read6502(ushort address)
{
    return bus_read(g_nes,(uint16_t)address);
}

void write6502(ushort address,uint8 value)
{
    bus_write(g_nes,(uint16_t)address,(uint8_t)value);
}

uint32_t cpu6502_step(void)
{
    return (uint32_t)step6502();
}

uint16_t cpu6502_get_pc(void)
{
    return (uint16_t)pc;
}

Cpu6502state cpu6502_get_state(void)
{
    Cpu6502state state;

    state.pc=(uint16_t) pc;

    state.a=(uint8_t)a;
    state.x=(uint8_t)x;
    state.y=(uint8_t)y;

    state.sp=(uint8_t)sp;
    state.p=(uint8_t)status;

    return state;
}

uint32_t cpu6502_step_and_sync(Nes*nes)
{
    uint32_t cpu_cycles = step6502();

    nes->cpu_cycles += cpu_cycles;
    for(uint32_t i=0;i<cpu_cycles*3u;++i)
    {
        ppu_tick(&nes->ppu);
    }
    if(nes->ppu.nmi_pending)
    {
        nes->ppu.nmi_pending=false;
        nmi6502();
    }

    return cpu_cycles;
}

void cpu6502_reset(void)
{
    reset6502();
}