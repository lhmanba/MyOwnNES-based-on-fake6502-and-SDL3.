#include "mapper0.h"

uint8_t mapper0_cpu_read(const Cartridge *cart, uint16_t addr)
{
    if(addr <0x8000)
    {
        return 0;
    }
    uint32_t offset = (uint32_t)(addr - 0x8000);
    if(cart->prg_size==16u*1024)
    {
        offset &= 0x3FFFu;//二进制为 0011 1111 1111 1111，表示取低14位，这么做可以实现16KB的PRG ROM镜像映射到0x8000-0xBFFF和0xC000-0xFFFF两个地址范围。
    }
    //32KB的PRG ROM直接映射到0x8000-0xFFFF地址范围，无需做任何处理。
    return cart->prg[offset];
}

uint8_t mapper0_ppu_read(const Cartridge *cart, uint16_t addr)
{
    return cart->chr[addr & 0x1FFFu];//二进制为 0001 1111 1111 1111，表示取低13位，这么做可以实现8KB的CHR ROM映射到0x0000-0x1FFF地址范围。
}

void mapper0_ppu_write(Cartridge *cart, uint16_t addr, uint8_t data)
{
    if(cart->chr_is_ram)
    {
        cart->chr[addr & 0x1FFFu] = data;
    }
}

