#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum MirrorMode
{
    MIRROR_HORIZONTAL,
    MIRROR_VERTICAL,
    MIRROR_FOUR_SCREEN,
} MirrorMode;

typedef struct Cartridge
{
    uint8_t *prg;
    size_t prg_size;

    uint8_t *chr;
    size_t chr_size;

    bool chr_is_ram;

    uint8_t mapper_id;
    MirrorMode mirror;
} Cartridge;

bool cartridge_load(Cartridge *cart, const char *path, char *err, size_t err_cap);