#include "ppu.h"
#include "mapper0.h"


static uint16_t mirror_nametable(const Cartridge*cart,uint16_t addr)
{
    uint16_t v=(uint16_t)(addr-0x2000u)&0x0FFFu;

    uint16_t table =v/0x400u;

    uint16_t offset=v&0x03FFu;

    if(cart->mirror==MIRROR_VERTICAL)
    {
        table &=1 ;
    }
    else
    {
        table = (uint16_t)(table >> 1);
    }

    return (uint16_t)(table * 0x0400u +offset);
}

static uint8_t palette_index(uint16_t addr)
{
    uint8_t i=(uint8_t)(addr & 0x1Fu);

    if((i & 0x13u) == 0x10u)
    {
        i &= 0x0Fu;
    }
    
/*
对nespalette做特殊处理
$3F10 → $3F00
$3F14 → $3F04
$3F18 → $3F08
$3F1C → $3F0C
*/
    return i;
}


uint8_t ppu_bus_read(Ppu *ppu,uint16_t addr)
{
    addr &= 0x3FFFu;

    if(addr <= 0x1FFFu)
    {
        return mapper0_ppu_read(ppu->cart,addr);
    }
    else if(addr <= 0x3EFFu)
    {
        uint16_t index = mirror_nametable(ppu->cart,addr);
        
        return ppu->nametable[index];
    }
    return ppu->palette[palette_index(addr)];
}

void ppu_bus_write(Ppu*ppu,uint16_t addr,uint8_t data)
{
    addr &= 0x3FFFu;

    if(addr <= 0x1FFFu)
    {
        mapper0_ppu_write(ppu->cart,addr,data);
    }
    else if(addr <= 0x3EFFu)
    {
        uint16_t index = mirror_nametable(ppu->cart,addr);

        ppu->nametable[index]=data;
    }
    else
    {
        ppu->palette[palette_index(addr)]=data;
    }
    return;
}

uint8_t ppu_cpu_read(Ppu*ppu,uint8_t reg)
{
    switch(reg)
    {
        case 2://PPUSTATUS
        {
            uint8_t result=ppu->status;
            ppu->status&=(uint8_t)~0x80u;
            ppu->write_toggle=false;
            return result;
        }
        case 4://OAMDATA
        {
            return ppu->oam[ppu->oam_addr];
        }
        case 7://PPUDATA
        {
            uint16_t addr = ppu->v & 0x3FFFu;
            uint8_t value = ppu_bus_read(ppu,addr);
            uint8_t result;
            if(addr >= 0x3F00u)
            {
                //palette立刻返回
                result =value;
            }
            else
            {
                //普通ppu延迟一次
                result = ppu->read_buffer;
                ppu->read_buffer=value;
            }
            if(ppu->ctrl & 0x04u)
            {
                ppu->v += 32;
            }
            else
            {
                ppu->v += 1u;
            }

            ppu->v &= 0x3FFFu;

            return result;
        }
        default:return 0;
    }
}

void ppu_cpu_write(Ppu*ppu,uint8_t reg,uint8_t data)
{
    switch(reg)
    {
        case 0://PPUSTATUS
        {
            ppu->ctrl = data;

            ppu->t=(uint16_t)((ppu->t & ~0x0C00u)|((data & 3u)<<10));

            break;
        }
        case 1://PPUMASK
        {
            ppu->mask=data;
            break;
        }
        case 3://PPU OAMADDR
        {
            ppu->oam_addr=data;
            break;
        }
        case 4://OAMDATA
        {
            ppu->oam[ppu->oam_addr]=data;

            ++(ppu->oam_addr);

            break;
        }
        case 5://PPUSCROLL
        {
            if(!ppu->write_toggle)
            {
                ppu->fine_x=data & 7u;
                ppu->t=(uint16_t)((ppu->t & ~0x001Fu)|(data)>>3);
                ppu->write_toggle = true;
            }
            else
            {
                ppu->t=(uint16_t)((ppu->t & ~0x73E0u) | ((uint16_t)(data & 7u)<<12)| ((uint16_t)(data & 0xF8u)<<2));
                ppu->write_toggle=false;
            }
            break;
        }
        case 6://PPUADDR
        {
            if(!ppu->write_toggle)
            {
                ppu->t=(uint16_t)((ppu->t & 0x00FFu)|(data & 0x3Fu)<<8);
                ppu->write_toggle=true;
            }
            else
            {
                ppu->t=(uint16_t)((ppu->t & 0x7F00u) | data);
                ppu->v=ppu->t;
                ppu->write_toggle=false;
            }
            break;
        }
        case 7://PPUDATA
        {
            ppu_bus_write(ppu,ppu->v,data);
            if(ppu->ctrl & 0x04u)
            {
                ppu->v+=32;//竖着走
            }
            else
            {
                ppu->v+=1u;//横着走
            }
            ppu->v &= 0x3FFFu;
            break;
        }
        default: break;
    }
}

