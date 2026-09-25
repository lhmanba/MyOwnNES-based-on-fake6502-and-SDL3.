#include "bus.h"
#include "mapper0.h"


uint8_t bus_read(Nes *nes, uint16_t addr)
{
    if(addr <= 0x1FFF)
    {
        return nes->ram[addr &0x07FFu];//二进制为 0000 0111 1111 1111，表示取低11位，这么做可以实现2KB的RAM镜像映射到0x0000-0x1FFF地址范围。
    }

    if(addr <= 0x3FFF)
    {
        return ppu_cpu_read(&nes->ppu,addr&7u);//&7是因为ppu只对cpu暴露8个寄存器
    }
    if(addr == 0x4016)
    {
        return controller_read(&nes->pad1);
    }
    if(addr >= 0x8000)
    {
        return mapper0_cpu_read(&nes->cart, addr);
    }

    //其他地址范围暂时返回0
    return 0;
}

void bus_write(Nes*nes,uint16_t addr,uint8_t data)
{
    if(addr <= 0x1FFF)
    {
        nes->ram[addr & 0x07FFu] = data;
        return;
    }
    else if(addr <= 0x3FFF)
    {
        ppu_cpu_write(&nes->ppu,addr&7u,data);
        return;
    }
    else if(addr == 0x4014u)
    {
        nes_oam_dma(nes,data);
        
        return;
    }
    else if(addr == 0x4016)
    {
        controller_write(&nes->pad1,data);
        return;
    }
    else if(addr >= 0x8000)
    {
        return;//0x8000及以上的地址范围是ROM区域，通常是只读的，因此在这里不进行写操作。
    }
}
