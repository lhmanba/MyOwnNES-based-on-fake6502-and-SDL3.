#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cartridge.h"
#include "mapper0.h"
#include "bus.h"
#include "nes.h"
#include "cpu6502_adapter.h"
#include "ppu.h"
#include "platform_sdl.h"

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
   printf("--Below is basic infoemation--\n");
   if(argc < 2)
   {
       printf("Usage: minines <NESrom.nes>\n");
       return 1;
   }

   Nes nes={0};
   char err[256];

   if(!cartridge_load(&nes.cart, argv[1], err, sizeof(err)))
   {
       printf("Failed to load NES ROM: %s\n", err);
       return 1;
   }

   nes.ppu.cart = &nes.cart;//加载卡带，很重要。

   cpu6502_bind(&nes);

   printf("PRG:%zu bytes\n",nes.cart.prg_size);
   printf("CHR:%zu bytes(%s)\n",nes.cart.chr_size,nes.cart.chr_is_ram ? "RAM" : "ROM");
   printf("Mapper:%d\n",nes.cart.mapper_id);
   printf("Mirror:%s\n",mirror_name(nes.cart.mirror));

   //拼接resetVector
   uint8_t lo=mapper0_cpu_read(&nes.cart,0xFFFC);
   uint8_t hi=mapper0_cpu_read(&nes.cart,0xFFFD);

   uint16_t reset_vector=((uint16_t)hi<<8) | ((uint16_t)lo);
   printf("Reset Vector: 0x%04X\n", reset_vector);

   cpu6502_reset();
   printf("CPU PC after reset:%04X\n",cpu6502_get_pc());
   printf("--Below is trace--\n");
   bool trace_enable = true;
   for (int i=0;i<20;++i)
    {
        Cpu6502state state=cpu6502_get_state();
        uint8_t opcode = bus_read(&nes,state.pc);
        if(trace_enable)
        {
            printf("0x%04X op:%02X  ""A:%02X X:%02X Y:%02X ""SP:%02X P:%02X\n",state.pc,opcode,state.a,state.x,state.y,state.sp,state.p);
        }

        cpu6502_step();
    }

    printf("--PPU TEST START--\n");

    /*
    Ppu ppu={0};
    ppu.cart=&nes.cart;
    ppu_bus_write(&ppu,0x2005,0x42);
    printf("At 0x2805 is %02X\n",ppu_bus_read(&ppu,0x2805));
    ppu_bus_write(&ppu,0x3F05,0x33);
    printf("$3F25 = %02X\n",
    ppu_bus_read(&ppu, 0x3F25));
    ppu_bus_write(&ppu,0x3F00,0x42);
    printf("$3F10 = %02X\n",ppu_bus_read(&ppu,0x3F10));
    ppu_bus_write(&ppu,0x3F14,0x66);
    printf("$3F04 = %02X\n",ppu_bus_read(&ppu,0x3F04));
    
    nes.ppu.status=0x80;
    nes.ppu.write_toggle=true;
    uint8_t result=bus_read(&nes,0x2002);
    printf("read=%02X status=%02X toggle=%d ",result,nes.ppu.status,nes.ppu.write_toggle);
    nes.ppu.status = 0x80;
    printf("$200A=%02X\n",bus_read(&nes,0x200A));
    */
    printf("--Test Scroll Data--\n");
    nes.ppu.t=0;
    nes.ppu.fine_x=0;
    nes.ppu.write_toggle=false;
    ppu_cpu_write(&nes.ppu,5,0x2B);
    printf("after first $2005:""t=%04X fine_x=%u toggle=%d\n",nes.ppu.t,nes.ppu.fine_x,nes.ppu.write_toggle);
    ppu_cpu_write(&nes.ppu,5,0x16);
    printf("after second $2005: ""t=%04X fine_x=%u toggle=%d\n",nes.ppu.t,nes.ppu.fine_x,nes.ppu.write_toggle);

    printf("--PPU ADDR TEST--\n");
    nes.ppu.t=0;
    nes.ppu.v=0;
    nes.ppu.write_toggle=false;
    ppu_cpu_write(&nes.ppu,6,0x3F);
    printf("after first $2006:""t=%04X v=%04X toggle=%d\n",nes.ppu.t,nes.ppu.v,nes.ppu.write_toggle);
    ppu_cpu_write(&nes.ppu,6,0x05);
    printf("after sencond $2006:""t=%04X v=%04X toggle=%d\n",nes.ppu.t,nes.ppu.v,nes.ppu.write_toggle);

    printf("--PPU DATA TEST--\n");
    nes.ppu.ctrl=0x00;
    nes.ppu.v=0x3F05;
    ppu_cpu_write(&nes.ppu,7,0x33);
    printf("+1:palette=%02X v=%04X\n",ppu_bus_read(&nes.ppu,0x3F05),nes.ppu.v);
    nes.ppu.ctrl=0x04;
    nes.ppu.v=0x2000;
    printf("beforecpuwrite\n");
    ppu_cpu_write(&nes.ppu,7,0x42);
    printf("beforePrintf\n");
    printf("+32:nametable=%02X v=%04X\n",ppu_bus_read(&nes.ppu,0x2000),nes.ppu.v);
    
    printf("-- PPU DATA Print--\n");
    ppu_bus_write(&nes.ppu,0x2000,0x42);
    nes.ppu.v=0x2000;
    nes.ppu.read_buffer=0xAA;
    nes.ppu.ctrl=0;
    uint8_t r=ppu_cpu_read(&nes.ppu,7);
    printf("result=%02X buffer=%02X v=%04X\n",r,nes.ppu.read_buffer,nes.ppu.v);
    ppu_bus_write(&nes.ppu,0x3F05,0x33);
    nes.ppu.v=0x3F05;
    nes.ppu.read_buffer=0xAA;
    nes.ppu.ctrl=0;
    r=ppu_cpu_read(&nes.ppu,7);
    printf("palette result=%02X buffer=%02X v=%04X\n",r,nes.ppu.read_buffer,nes.ppu.v);
   
    printf("--DMA TEST--\n");
    for(uint16_t i=0;i<256;++i)
    {
        bus_write(&nes,(uint16_t)(0x0200u+i),(uint8_t)i);
    }
    nes.ppu.oam_addr=0;
    uint64_t old_cycles=nes.cpu_cycles;
    bus_write(&nes,0x4014,0x02);
    printf("OAM[00]=%02X\n",nes.ppu.oam[0x00]);
    printf("OAM[01]=%02X\n",nes.ppu.oam[0x01]);
    printf("OAM[80]=%02X\n",nes.ppu.oam[0x80]);
    printf("OAM[FF]=%02X\n",nes.ppu.oam[0xFF]);
    printf("DMA cycles=%llu\n",(unsigned long long)(nes.cpu_cycles - old_cycles));

    //SDL程序开始
    printf("before SDL create\n");
    PlatformSdl *platform = platform_sdl_creat();
    if(!platform)
    {
        fprintf(stderr, "platform_sdl_create failed\n");
        return;
    }
    printf("SDL created\n");
    bool running = true;
    while(running)
    {
        printf("loop begin\n");
        running=platform_sdl_poll(platform);
        if(!running)
        {
            printf("poll requested quit\n");
            break;
        }
        printf("before ppu render\n");
        ppu_render_frame(&nes.ppu);
        printf("after ppu render\n");
        if(!platform_sdl_present(platform,nes.ppu.frame))
        {
            printf("present failed\n");
            running=false;
        }
        printf("after present\n");
    }
    printf("leaving main loop\n");
    platform_sdl_destroy(platform);
    printf("SDL destroyed\n");

   return 0;
}

//cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
//.\build\minines .\roms\Fine.nes