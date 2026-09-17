#include "cartridge.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//读取NES游戏ROM文件的头部信息，并进行验证，确保其符合INES文件格式规范。
bool cartridge_load(Cartridge *cart, const char *path, char *err, size_t err_cap)
{
    memset(cart, 0, sizeof(Cartridge));

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) 
    {
        snprintf(err, err_cap, "Cannot open file: %s", path);
        return false;   
    }

    uint8_t h[16];
    if(fread(h,1,16,fp) != sizeof(h))
    {
        snprintf(err, err_cap, "ROM header is not 16 bytes: %s", path);
        fclose(fp);
        return false;
    }
    if(h[0]!='N' || h[1]!='E' || h[2]!='S' || h[3]!=0x1A)
    {
        snprintf(err, err_cap, "ROM is not a valid ines file: %s", path);
        fclose(fp);
        return false;
    }

    printf("Valid ines file!\n");

    size_t prg_size = (size_t)h[4] * 16u * 1024u;
    size_t chr_size = (size_t)h[5] * 8u * 1024u;
    //mapperID被拆分到两个字节中，低四位在h[6]的高四位，高四位在h[7]的低四位。通过位运算将其组合成一个完整的mapperID。
    uint8_t mapper=((h[6] >> 4) & 0x0F) | (h[7] & 0xF0);
    //判断是否含有trainer，检查h[6]的第3位是否为1，如果是，则表示存在trainer数据。
    bool trainer=(h[6] & 0x04) != 0;

    if(mapper != 0)
    {
        snprintf(err, err_cap, "Unsupported mapper: %d", mapper);

        fclose(fp);
        return false;
    }
    //解析mirroring
    if(h[6] & 0x08)
    {
        cart->mirror = MIRROR_FOUR_SCREEN;
    }
    else if(h[6] & 0x01)
    {
        cart->mirror = MIRROR_VERTICAL;
    }
    else
    {
        cart->mirror = MIRROR_HORIZONTAL;
    }
    //记录数据
    cart->prg_size = prg_size;
    cart->chr_size = chr_size;
    cart->mapper_id = mapper;
    //处理trainer数据，如果存在trainer，则需要跳过前512字节的数据，因为trainer数据位于ROM文件的开头部分。
    if(trainer)
    {
        if(fseek(fp, 512, SEEK_CUR) != 0)//从当前位置往后跳512字节，跳过trainer数据
        {
            snprintf(err, err_cap, "Failed to skip trainer data: %s", path);
            fclose(fp);
            return false;
        }
    }
    //给PRG数据分配内存空间，大小为prg_size字节
    cart->prg = malloc(prg_size);
    if(cart->prg == NULL)
    {
        snprintf(err, err_cap, "Failed to allocate memory for PRG data: %s", path);
        fclose(fp);
        return false;
    }
    //读取PRG数据到cart->prg中
    if(fread(cart->prg,1,prg_size,fp)!= prg_size)
    {
        snprintf(err, err_cap, "Failed to read PRG data: %s,PRG Data is incomplete", path);
        fclose(fp);
        return false;
    }

    //给CHR数据分配内存空间,分两种情况
    //ROM文件中有CHR数据时，分配chr_size字节的内存空间，并读取CHR数据到cart->chr中。
    if(chr_size >0)
    {
        cart->chr = malloc(chr_size);
        if(cart->chr == NULL)
        {
            snprintf(err, err_cap, "Failed to allocate memory for CHR data: %s", path);
            fclose(fp);
            return false;
        }
        //读取CHR数据到cart->chr中
        if(fread(cart->chr,1,chr_size,fp)!= chr_size)
        {
            snprintf(err, err_cap, "Failed to read CHR data: %s,CHR Data is incomplete", path);
            fclose(fp);
            return false;
        }

        cart->chr_is_ram = false;
    }
    //没有CHR ROM，创建CHR RAM
    else
    {
        cart->chr_size = 8u * 1024;
    }


    fclose(fp);

    return true;
}

void cartridge_free(Cartridge *cart)
{
    if(cart->prg)
    {
        free(cart->prg);
        cart->prg = NULL;
    }
    if(cart->chr)
    {
        free(cart->chr);
        cart->chr = NULL;
    }

    memset(cart, 0, sizeof(Cartridge));
    return;
}
