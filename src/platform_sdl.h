#ifndef PLATFORM_SDL
#define PLATFORM_SDL

#include <stdbool.h>
#include <stdint.h>
#include "controller.h"

typedef struct PlatformSdl PlatformSdl;

PlatformSdl *platform_sdl_creat(void);
bool platform_sdl_present(PlatformSdl *platform,const uint32_t *frame);
bool platform_sdl_poll(PlatformSdl *platform,Controller *pad);
void platform_sdl_destroy(PlatformSdl *platform);

#endif
