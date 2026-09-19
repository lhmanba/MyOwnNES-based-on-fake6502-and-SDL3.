#include "bus.h"
#include "mapper0.h"

uint8_t bus_read(Nes *nes, uint16_t addr)
{
    if(addr < 0x1FFF)
    {
        return nes->ram[addr &0x07FFu];//二进制为 0000 0111 1111 1111，表示取低11位，这么做可以实现2KB的RAM镜像映射到0x0000-0x1FFF地址范围。
    }

    if(addr <= 0x3FFF)
    {
        return 0;//PPU寄存器区域，目前占位
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
    if(addr < 0x1FFF)
    {
        nes->ram[addr & 0x07FFu] = data;
        return;
    }
    else if(addr <= 0x3FFF)
    {
        //PPU寄存器区域，目前占位
    }
    else if(addr >= 0x8000)
    {
        return;//0x8000及以上的地址范围是ROM区域，通常是只读的，因此在这里不进行写操作。
    }
}
