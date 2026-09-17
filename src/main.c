#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cartridge.h"
#include "mapper0.h"

#define NES_WIDTH 256
#define NES_HEIGHT 240
//定义一个静态的帧缓冲区，用于存储NES游戏的像素数据。帧缓冲区的大小为NES_WIDTH * NES_HEIGHT，即256 * 240个像素点，每个像素点使用32位无符号整数表示颜色值。
static uint32_t nes_framebuffer[NES_WIDTH * NES_HEIGHT];
static const char*mirror_name(MirrorMode mirror)
{
    switch(mirror)
    {
        case MIRROR_HORIZONTAL:
            return "Horizontal";
        case MIRROR_VERTICAL:
            return "Vertical";
        case MIRROR_FOUR_SCREEN:
            return "Four Screen";
        default:
            return "Unknown";
    }
}

int main(int argc, char *argv[])
{
   if(argc < 2)
   {
       printf("Usage: minines <NESrom.nes>\n");
       return 1;
   }

   Cartridge cart;
   char err[256];

   if(!cartridge_load(&cart, argv[1], err, sizeof(err)))
   {
       printf("Failed to load NES ROM: %s\n", err);
       return 1;
   }

   printf("PRG:%zu bytes\n",cart.prg_size);
   printf("CHR:%zu bytes(%s)\n",cart.chr_size,cart.chr_is_ram ? "RAM" : "ROM");
   printf("Mapper:%d\n",cart.mapper_id);
   printf("Mirror:%s\n",mirror_name(cart.mirror));

   //拼接resetVector
   uint8_t lo=mapper0_cpu_read(&cart,0xFFFC);
   uint8_t hi=mapper0_cpu_read(&cart,0xFFFD);

   uint16_t reset_vector=((uint16_t)hi<<8) | ((uint16_t)lo);
   printf("Reset Vector: 0x%04X\n", reset_vector);

   return 0;
}