#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum ControllerButton
{
    BUTTON_A = 1u<<0,
    BUTTON_B = 1u<<1,
    BUTTON_SEL = 1u<<2,
    BUTTON_STA = 1u<<3,
    BUTTON_UP = 1u<<4,
    BUTTON_DOWN = 1u<<5,
    BUTTON_LEFT = 1u<<6,
    BUTTON_RIGHT = 1u<<7,
}ControllerButton;

typedef struct Controller
{
    uint8_t buttons;//当前真实按键
    uint8_t shift;//CPU正在读的那份快照
    bool strobe;//现在是否处于锁存状态
} Controller;

void controller_reset(Controller *pad);
void controller_write(Controller *pad,uint8_t value);
uint8_t controller_read(Controller *pad);

#endif
