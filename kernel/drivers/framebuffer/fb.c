#include "drivers/framebuffer/fb.h"
#include "debug.h"

static framebuffer_t * framebuffers;
static uint8_t framebuffer_count;

void fb_init(framebuffer_t *fbs, uint8_t count) {
    k_log("[DRV][FB] Initializing framebuffers...");
    framebuffers = fbs;
    framebuffer_count = count;
    k_log("[DRV][FB] Done.");
}

void fb_draw_pixel(uint8_t screen, uint32_t x, uint32_t y, uint32_t color) {
    if (screen >= framebuffer_count) {
        k_error("[FRV][FB] Drawing to non existent screen.");
        return;
    }
    framebuffer_t *fb = &framebuffers[screen];
    if (x >= fb->width || y >= fb->height) return;
    uint32_t *target = (uint32_t *)(fb->address + y * fb->pitch + x * (fb->bpp / 8));
    *target = color;
}

void fb_fill_rect(uint8_t screen, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    if (screen >= framebuffer_count) {
        k_error("[FRV][FB] Drawing to non existent screen.");
        return;
    }
    framebuffer_t *fb = &framebuffers[screen];

    for (uint32_t i = x; i < x + w; i++) {
        for (uint32_t j = y; j < y + h; j++) {
            fb_draw_pixel(screen, i, j, color);
        }
    }
}