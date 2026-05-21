#pragma once
#include "types.h"
#include "boot.h"

#define FB_FONT_8_16_CHARS 256
typedef struct fb_font
{
    uint8_t width;
    uint8_t height;
    uint8_t h_space;
    uint8_t v_space;
    uint8_t num_chars;
    uint8_t* data;
} fb_font_t;


void fb_init(framebuffer_t *fbs, uint8_t count);
void fb_draw_pixel(uint8_t screen, uint32_t x, uint32_t y, uint32_t color);
void fb_fill_rect(uint8_t screen, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
void fb_putchar(uint8_t screen, uint32_t x, uint32_t y, unsigned char chr, uint32_t fg, uint32_t bg, fb_font_t* font);