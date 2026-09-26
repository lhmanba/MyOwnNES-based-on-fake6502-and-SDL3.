#include "ppu.h"
#include "mapper0.h"
#include <string.h>

static const uint32_t nes_palette[64] =
{
    0xFF757575u, 0xFF271B8Fu, 0xFF0000ABu, 0xFF47009Fu,
    0xFF8F0077u, 0xFFAB0013u, 0xFFA70000u, 0xFF7F0B00u,
    0xFF432F00u, 0xFF004700u, 0xFF005100u, 0xFF003F17u,
    0xFF1B3F5Fu, 0xFF000000u, 0xFF000000u, 0xFF000000u,

    0xFFBCBCBCu, 0xFF0073EFu, 0xFF233BEFu, 0xFF8300F3u,
    0xFFBF00BFu, 0xFFE7005Bu, 0xFFDB2B00u, 0xFFCB4F0Fu,
    0xFF8B7300u, 0xFF009700u, 0xFF00AB00u, 0xFF00933Bu,
    0xFF00838Bu, 0xFF000000u, 0xFF000000u, 0xFF000000u,

    0xFFFFFFFFu, 0xFF3FBFFFu, 0xFF5F97FFu, 0xFFA78BFDu,
    0xFFF77BFFu, 0xFFFF77B7u, 0xFFFF7763u, 0xFFFF9B3Bu,
    0xFFF3BF3Fu, 0xFF83D313u, 0xFF4FDF4Bu, 0xFF58F898u,
    0xFF00EBDBu, 0xFF000000u, 0xFF000000u, 0xFF000000u,

    0xFFFFFFFFu, 0xFFABE7FFu, 0xFFC7D7FFu, 0xFFD7CBFFu,
    0xFFFFC7FFu, 0xFFFFC7DBu, 0xFFFFBFB3u, 0xFFFFDBABu,
    0xFFFFE7A3u, 0xFFE3FFA3u, 0xFFABF3BFu, 0xFFB3FFCFu,
    0xFF9FFFF3u, 0xFF000000u, 0xFF000000u, 0xFF000000u
};

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

static void background_position(Ppu*ppu,uint16_t x,uint16_t y,uint16_t *nametable_base,uint8_t *tile_x,uint8_t *tile_y,uint8_t *fine_x,uint8_t *fine_y)
{
    uint16_t coarse_x=ppu->t & 0x001Fu;
    uint16_t coarse_y =(ppu->t >> 5) & 0x001Fu;
    uint16_t nt =(ppu->t >> 10) & 0x03u;
    uint16_t scroll_x =coarse_x * 8u + ppu->fine_x;
    uint16_t scroll_y =coarse_y * 8u +((ppu->t >> 12) & 7u);
    uint16_t nt_x = nt & 1u;
    uint16_t nt_y = (nt >> 1) & 1u;
    uint16_t world_x =(uint16_t)(nt_x * 256u + scroll_x + x);uint16_t world_y =(uint16_t)(nt_y * 240u + scroll_y + y);
    uint16_t table_x =(world_x / 256u) & 1u;
    uint16_t table_y =(world_y / 240u) & 1u;
    uint16_t local_x =world_x % 256u;
    uint16_t local_y =world_y % 240u;
    uint16_t table =table_y * 2u + table_x;
    *nametable_base =(uint16_t)(0x2000u + table * 0x0400u);
    *tile_x = (uint8_t)(local_x / 8u);
    *tile_y = (uint8_t)(local_y / 8u);
    *fine_x = (uint8_t)(local_x & 7u);
    *fine_y = (uint8_t)(local_y & 7u);
}

static uint8_t pattern_pixel(Ppu*ppu,uint8_t tile_id,uint8_t pixel_x,uint8_t pixel_y)
{
    uint16_t pattern_base=(ppu->ctrl & 0x10) ? 0x1000u : 0x0000u;
    uint16_t tile_addr=(uint16_t)(pattern_base + (uint16_t)tile_id*16u + pixel_y);
    uint8_t plane0=ppu_bus_read(ppu,tile_addr);
    uint8_t plane1=ppu_bus_read(ppu,(uint16_t)(tile_addr+8u));
    uint8_t bit=(uint8_t)(7u - (pixel_x & 7u));
    uint8_t lo=(uint8_t)((plane0 >> bit) & 1u);
    uint8_t hi=(uint8_t)((plane1 >> bit) & 1u);
    return (uint8_t)(lo | (hi<<1));
}

static uint8_t background_pattern_pixel(Ppu*ppu,uint16_t x,uint16_t y)
{
    uint8_t tile_x;
    uint8_t tile_y;
    uint8_t fine_x;
    uint8_t fine_y;

    uint16_t nametable_base;
    background_position(ppu,x,y,&nametable_base,&tile_x,&tile_y,&fine_x,&fine_y);
    uint16_t nametable_addr=(uint16_t)(nametable_base + tile_y*32u + tile_x);
    uint8_t tile_id=ppu_bus_read(ppu,nametable_addr);

    return pattern_pixel(ppu,tile_id,fine_x,fine_y);
}

static uint8_t background_palette_group(Ppu*ppu,uint16_t nametable_base,uint8_t tile_x,uint8_t tile_y)
{
    uint16_t attribute_addr=(uint16_t)(nametable_base+0x03C0u+(tile_y/4u)*8u+(tile_x/4u));
    uint8_t attribute=ppu_bus_read(ppu,attribute_addr);
    uint8_t shift=(uint8_t)(((tile_y & 2u) ? 4u:0u)+((tile_x & 2u) ? 2u:0u));
    return (uint8_t)(attribute >> shift)&3u;
}

static uint8_t background_nes_color(Ppu*ppu,uint8_t pattern_color,uint8_t palette_group)
{
    uint16_t palette_addr;
    if(pattern_color == 0)
    {
        palette_addr = 0x3F00u;
    }
    else
    {
        palette_addr=(uint16_t)(0x3F00u+(uint16_t)palette_group*4u + pattern_color);
    }

    return (uint8_t)(ppu_bus_read(ppu,palette_addr) & 0x3Fu);
        
}

static uint8_t background_pixel_color(Ppu *ppu,uint16_t x,uint16_t y)
{
    uint8_t tile_x;
    uint8_t tile_y;
    uint16_t nametable_base;
    uint8_t fine_x;
    uint8_t fine_y;
    background_position(ppu,x,y,&nametable_base,&tile_x,&tile_y,&fine_x,&fine_y);
    uint8_t pattern_color=background_pattern_pixel(ppu,x,y);
    uint8_t palette_group=background_palette_group(ppu,nametable_base,tile_x,tile_y);

    return background_nes_color(ppu,pattern_color,palette_group);
}

static uint32_t nes_color_to_argb(uint8_t color)
{
    return nes_palette[color & 0x3Fu];
}

static void render_background(Ppu *ppu)
{
    for(uint16_t y=0;y<240u;++y)
    {
        for(uint16_t x=0;x<256u;++x)
        {
            uint8_t nes_color=background_pixel_color(ppu,x,y);
            uint32_t argb=nes_color_to_argb(nes_color);

            ppu->frame[y*256u+x]=argb;
        }
    }
}

static uint8_t sprite_pattern_pixel(Ppu *ppu,uint8_t tile_id,uint8_t pixel_x,uint8_t pixel_y)
{
    uint16_t pattern_base=(ppu->ctrl & 0x08u)?0x1000u:0x0000u;
    uint16_t tile_addr=(uint16_t)(pattern_base+(uint16_t)tile_id*16u+pixel_y);
    uint8_t plane0=ppu_bus_read(ppu,tile_addr);
    uint8_t plane1=ppu_bus_read(ppu,(uint16_t)(tile_addr+8u));
    uint8_t bit=(uint8_t)(7u - pixel_x);
    uint8_t lo=(uint8_t)((plane0 >> bit) & 1u);
    uint8_t hi=(uint8_t)((plane1 >> bit) & 1u);
    return (uint8_t)(lo | (hi << 1));
}

static void render_sprites(Ppu *ppu)
{
    for(int sprite =63;sprite >=0;--sprite)
    {
        uint16_t base=(uint16_t)sprite*4u;
        uint8_t sprite_y=ppu->oam[base+0];
        uint8_t tile_id=ppu->oam[base+1];
        uint8_t attr=ppu->oam[base+2];
        uint8_t sprite_x=ppu->oam[base+3];
        uint8_t palette_group=attr & 0x03u;
        for(uint8_t py=0;py<8;++py)
        {
            for(uint8_t px=0;px<8;++px)
            {
                int x=sprite_x + px;
                int y=sprite_y + py;

                if(x>=246 || y>=240)
                {
                    continue;
                }
                uint8_t pattern_color=sprite_pattern_pixel(ppu,tile_id,px,py);
                if(pattern_color == 0)
                {
                    continue;
                }
                uint8_t palette_addr=(uint16_t)(0x3F10u + palette_group *4u+pattern_color);
                uint8_t nes_color=ppu_bus_read(ppu,palette_addr)&0x3Fu;
                ppu->frame[y*256 + x]=nes_palette[nes_color];
            }
        }
            
    }
}

static void ppu_find_sprite0_hit(Ppu *ppu)
{
    ppu->sprite0_hit_valid =false;
    //背景和sprite都必须开启
    if((ppu->mask & 0x18u) != 0x18u)
    {
        return;
    }
    int sprite_x=ppu->oam[3];
    int sprite_y=(int)ppu->oam[0]+1;
    uint8_t tile_id=ppu->oam[1];
    uint8_t attr=ppu->oam[2];
    bool flip_h=(attr&0x40u)!=0;
    bool flip_v=(attr&0x80u)!=0;
    for(uint8_t py=0;py<8;++py)
    {
        for(uint8_t px=0;px<8;++px)
        {
            int x=sprite_x+px;
            int y=sprite_y+py;
            if(x<0||x>=256||y<0||y>=240)
            {
                continue;
            }
            if(x == 255)
            {
                continue;
            }
            //左8像素裁剪
            if(x<8)
            {
                if((ppu->mask & 0x02u)==0||(ppu->mask & 0x04u)==0)
                {
                    continue;
                }
            }
            uint8_t sx=flip_h?(uint8_t)(7u-px):px;
            uint8_t sy=flip_v?(uint8_t)(7u-py):py;
            uint8_t sprite_color=sprite_pattern_pixel(ppu,tile_id,sx,sy);
            if(sprite_color==0)
            {
                continue;
            }
            uint8_t bg_color=background_pattern_pixel(ppu,(uint16_t)x,(uint16_t)y);
            if(bg_color==0)
            {
                continue;
            }
            //找到第一次重叠
            ppu->sprite0_hit_x=(uint16_t)x;
            ppu->sprite0_hit_y=(uint16_t)y;
            ppu->sprite0_hit_valid=true;
            return;
        }
    }
}

void ppu_tick(Ppu *ppu)
{
    ++ppu->dot;
    if(ppu->dot >= 341)
    {
        ppu->dot=0;
        ++ppu->scanline;
        if(ppu->scanline >= 262)
        {
            ppu->scanline=0;
            //新一帧开始先寻找本帧sprite0hit位置
            ppu_find_sprite0_hit(ppu);
        }
    }
    if(ppu->sprite0_hit_valid && (ppu->status & 0x40u)==0 && ppu->scanline==(int)ppu->sprite0_hit_y && ppu->dot==(int)ppu->sprite0_hit_x+1)
    {
        ppu->status |= 0x40u;
        printf("SPRITE0 HIT x=%u y=%u\n",ppu->sprite0_hit_x,ppu->sprite0_hit_y);
    }
    if (ppu->scanline == 241 &&ppu->dot == 1)
    {
        ppu->status |= 0x80u;

        if (ppu->ctrl & 0x80u)
        {
            ppu->nmi_pending = true;
        }
    }

    if (ppu->scanline == 261 &&
        ppu->dot == 1)
    {
        ppu->status &= (uint8_t)~0xE0u;
    }
}

void ppu_render_frame(Ppu *ppu)
{
    render_background(ppu);
    render_sprites(ppu);
}

void ppu_reset(Ppu *ppu)
{
    Cartridge *cart=ppu->cart;
    memset(ppu,0,sizeof(*ppu));
    ppu->cart=cart;
}







