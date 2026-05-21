#pragma once
#include "types.h"
#include "boot.h"


void fb_init(framebuffer_t *fbs, uint8_t count);
void fb_draw_pixel(uint8_t screen, uint32_t x, uint32_t y, uint32_t color);
void fb_fill_rect(uint8_t screen, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);