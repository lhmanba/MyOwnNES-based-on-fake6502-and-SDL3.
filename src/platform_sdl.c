#include "platform_sdl.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>

struct PlatformSdl
{
    SDL_Window *window;
    SDL_Renderer *render;
    SDL_Texture *texture;
};

PlatformSdl *platform_sdl_creat(void)
{
    PlatformSdl *platform=calloc(1,sizeof(*platform));
    if(!platform)
    {
        return NULL;
    }
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        fprintf(stderr,"SDL_Init:%s\n",SDL_GetError());
        free(platform);
        return NULL;
    }
    platform->window=SDL_CreateWindow("Minines",768,720,0);
    if(!platform->window)
    {
        platform_sdl_destroy(platform);
        return NULL;
    }
    platform->render=SDL_CreateRenderer(platform->window,NULL);
    if(!platform->render)
    {
        platform_sdl_destroy(platform);
        return NULL;
    }
    platform->texture=SDL_CreateTexture(platform->render,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,256,240);
    if(!platform->texture)
    {
        platform_sdl_destroy(platform);
        return NULL;
    }

    SDL_SetTextureScaleMode(platform->texture,SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(platform->render,256,240,SDL_LOGICAL_PRESENTATION_LETTERBOX);

    return platform;
}

bool platform_sdl_poll(PlatformSdl *platform,Controller *pad)
{
    (void)platform;

    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_EVENT_QUIT)
        {
            return false;
        }
    }
    const bool *keys=SDL_GetKeyboardState(NULL);
    uint8_t buttons=0;
        if (keys[SDL_SCANCODE_X])
        buttons |= BUTTON_A;

    if (keys[SDL_SCANCODE_Z])
        buttons |= BUTTON_B;

    if (keys[SDL_SCANCODE_RSHIFT])
        buttons |= BUTTON_SEL;

    if (keys[SDL_SCANCODE_RETURN])
        buttons |= BUTTON_STA;

    if (keys[SDL_SCANCODE_UP])
        buttons |= BUTTON_UP;

    if (keys[SDL_SCANCODE_DOWN])
        buttons |= BUTTON_DOWN;

    if (keys[SDL_SCANCODE_LEFT])
        buttons |= BUTTON_LEFT;

    if (keys[SDL_SCANCODE_RIGHT])
        buttons |= BUTTON_RIGHT;

    /*
     * Up + Down 同时按下 → 两个都取消
     */
    if ((buttons & BUTTON_UP) &&
        (buttons & BUTTON_DOWN))
    {
        buttons &= (uint8_t)~(
            BUTTON_UP | BUTTON_DOWN);
    }

    /*
     * Left + Right 同时按下 → 两个都取消
     */
    if ((buttons & BUTTON_LEFT) &&
        (buttons & BUTTON_RIGHT))
    {
        buttons &= (uint8_t)~(
            BUTTON_LEFT | BUTTON_RIGHT);
    }

    pad->buttons = buttons;
    return true;
}

bool platform_sdl_present(PlatformSdl *platform,const uint32_t *frame)
{
    if(!SDL_UpdateTexture(platform->texture,NULL,frame,256*sizeof(uint32_t)))
    {
        fprintf(stderr,"SDL_updatetexture:%s\n",SDL_GetError());
        return false;
    }
    if(!SDL_SetRenderDrawColor(platform->render,0,0,0,255))
    {
        fprintf(stderr,"SDL_SetRenderDrawcolor:%s\n",SDL_GetError());
        return false;
    }
    if(!SDL_RenderClear(platform->render))
    {
        fprintf(stderr,"SDL_RenderClear:%s\n",SDL_GetError());
        return false;
    }
    if(!SDL_RenderTexture(platform->render,platform->texture,NULL,NULL))
    {
        fprintf(stderr,"SDL_RenderTexture:%s\n",SDL_GetError());
        return false;
    }
    if(!SDL_RenderPresent(platform->render))
    {
        fprintf(stderr,"SDL_Renderpresent:%s\n",SDL_GetError());
        return false;
    }

    return true;
}

void platform_sdl_destroy(PlatformSdl *platform)
{
    if(!platform)
    {
        return;
    }
    if(platform->texture)
    {
        SDL_DestroyTexture(platform->texture);
    }
    if(platform->render)
    {
        SDL_DestroyRenderer(platform->render);
    }
    if(platform->window)
    {
        SDL_DestroyWindow(platform->window);
    }
    SDL_Quit();

    free(platform);
}
