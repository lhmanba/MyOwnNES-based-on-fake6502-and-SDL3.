#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define NES_WIDTH 256
#define NES_HEIGHT 240
//定义一个静态的帧缓冲区，用于存储NES游戏的像素数据。帧缓冲区的大小为NES_WIDTH * NES_HEIGHT，即256 * 240个像素点，每个像素点使用32位无符号整数表示颜色值。
static uint32_t nes_framebuffer[NES_WIDTH * NES_HEIGHT];

static void fill_test_pattern()
{
    for(int y=0;y<NES_HEIGHT;y++)
    {
        for(int x=0;x<NES_WIDTH;x++)
        {
            int block_x=x/16;
            int block_y=y/16;

            bool white=((block_x+block_y)%2)==0;

            if(white)
            {
                nes_framebuffer[y*NES_WIDTH+x]=0xFFFFFFFF; //白色
            }
            else
            {
                nes_framebuffer[y*NES_WIDTH+x]=0xFF202020; //黑色
            }
        }
    }
}

int main()
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("MiniNES",768,720,0);
    //创建一个SDL窗口，标题为"MiniNES"，宽度为768像素，高度为720像素，窗口标志为0（表示默认窗口样式）。如果窗口创建失败，程序会输出错误信息并返回1。
    if(window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    
    if(renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, NES_WIDTH, NES_HEIGHT);
    //创建一个SDL纹理，用于在窗口中显示NES游戏的图像。纹理的像素格式为RGBA8888（每个像素使用32位表示颜色值），访问方式为流式访问（表示纹理数据可以频繁更新），宽度为NES_WIDTH，高度为NES_HEIGHT。如果纹理创建失败，程序会输出错误信息，销毁窗口和渲染器，并返回1。
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    //邻近缩放3x
    SDL_SetRenderLogicalPresentation(renderer, NES_WIDTH, NES_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    //逻辑呈现模式为信箱模式，保持原始宽高比
    fill_test_pattern();

    bool running = true;

    while(running)
    {
        SDL_Event event;
       
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        SDL_UpdateTexture(texture, NULL, nes_framebuffer, NES_WIDTH * sizeof(uint32_t));
        //每帧上传framebuffer到纹理，最后设置framebuffer占用总字节为NES_WIDTH * sizeof(uint32_t)，即每行像素的字节数。
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        //设置渲染器的绘制颜色为黑色
        SDL_RenderClear(renderer);
        //清空上一帧
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        //渲染纹理
        SDL_RenderPresent(renderer);
        //显示渲染结果
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}