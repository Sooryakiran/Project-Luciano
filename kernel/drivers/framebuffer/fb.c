#include "drivers/framebuffer/fb.h"
#include "debug.h"

static framebuffer_t *framebuffers;
static uint8_t framebuffer_count;

void fb_init(framebuffer_t *fbs, uint8_t count)
{
    k_log("[DRV][FB] Initializing framebuffers...");
    framebuffers = fbs;
    framebuffer_count = count;
    k_log("[DRV][FB] Done.");
}

void fb_draw_pixel(uint8_t screen, uint32_t x, uint32_t y, uint32_t color)
{
    if (screen >= framebuffer_count)
    {
        k_error("[FRV][FB] Drawing to non existent screen.");
        return;
    }
    framebuffer_t *fb = &framebuffers[screen];
    if (x >= fb->width || y >= fb->height)
        return;
    uint32_t *target = (uint32_t *)(fb->address + y * fb->pitch + x * (fb->bpp / 8));
    *target = color;
}

void fb_fill_rect(uint8_t screen, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    if (screen >= framebuffer_count)
    {
        k_error("[FRV][FB] Drawing to non existent screen.");
        return;
    }
    framebuffer_t *fb = &framebuffers[screen];

    for (uint32_t i = x; i < x + w; i++)
    {
        for (uint32_t j = y; j < y + h; j++)
        {
            fb_draw_pixel(screen, i, j, color);
        }
    }
}

void fb_putchar(
    uint8_t screen,
    uint32_t x,
    uint32_t y,
    unsigned char chr,
    uint32_t fg,
    uint32_t bg,
    fb_font_t *font)
{
    uint8_t bytes_per_row = (font->width + 7) / 8;
    for (uint8_t row = 0; row < font->height; row++)
    {
        for (uint8_t col = 0; col < font->width; col++)
        {
            uint32_t target_x = x + col;
            uint32_t target_y = y + row;
            uint32_t byte_idx = (chr * font->height + row) * bytes_per_row + (col / 8);
            uint8_t bit_idx = 7 - (col % 8);
            uint8_t glyph = font->data[byte_idx];
            uint32_t color = (glyph & (1 << bit_idx)) ? fg : bg;

            fb_draw_pixel(screen, target_x, target_y, color);
        }
    }
}