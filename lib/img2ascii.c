#include "img2ascii.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

const char img2ascii_brightness[] =
    "`.-':_,^=;><+!rc*/"
    "z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";
const int img2ascii_brightness_len = sizeof(img2ascii_brightness) - 1;

static const char *get_color(int r, int g, int b) {
    static char color[27];
    snprintf(color, sizeof(color), "\033[38;2;%d;%d;%dm", r, g, b);
    return color;
}

static unsigned int braille_from_dots(int d1, int d2, int d3, int d4,
                                       int d5, int d6, int d7, int d8) {
    return 0x2800 | (d1 ? 1 << 0 : 0) | (d2 ? 1 << 1 : 0) | (d3 ? 1 << 2 : 0) |
           (d4 ? 1 << 3 : 0) | (d5 ? 1 << 4 : 0) | (d6 ? 1 << 5 : 0) |
           (d7 ? 1 << 6 : 0) | (d8 ? 1 << 7 : 0);
}

static char *braille_char(char buf[4], int d1, int d2, int d3, int d4,
                           int d5, int d6, int d7, int d8) {
    unsigned int cp = braille_from_dots(d1, d2, d3, d4, d5, d6, d7, d8);
    buf[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
    buf[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
    buf[2] = (char)(0x80 | (cp & 0x3F));
    buf[3] = '\0';
    return buf;
}

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} sb_t;

static int sb_init(sb_t *sb, size_t initial) {
    sb->buf = (char *)malloc(initial);
    if (!sb->buf) return -1;
    sb->len = 0;
    sb->cap = initial;
    sb->buf[0] = '\0';
    return 0;
}

static int sb_grow(sb_t *sb, size_t needed) {
    if (sb->len + needed + 1 <= sb->cap) return 0;
    size_t newcap = sb->cap ? sb->cap : 4096;
    while (newcap < sb->len + needed + 1) newcap *= 2;
    char *tmp = (char *)realloc(sb->buf, newcap);
    if (!tmp) return -1;
    sb->buf = tmp;
    sb->cap = newcap;
    return 0;
}

static int sb_append(sb_t *sb, const char *s) {
    size_t slen = strlen(s);
    if (sb_grow(sb, slen) != 0) return -1;
    memcpy(sb->buf + sb->len, s, slen);
    sb->len += slen;
    sb->buf[sb->len] = '\0';
    return 0;
}

char *img2ascii(const unsigned char *pixels, int width, int height,
                int channels, img2ascii_opts opts) {
    if (!pixels || width <= 0 || height <= 0) return NULL;
    if (channels < 3) return NULL;

    int brightness_len = img2ascii_brightness_len;

    int new_width = opts.target_width;
    int new_height = opts.target_height;

    if (new_width <= 0 && new_height <= 0) {
        new_width = IMG2ASCII_DEFAULT_WIDTH;
    }

    if (new_width > 0 && new_height <= 0) {
        new_height = (height * new_width) / width;
    } else if (new_height > 0 && new_width <= 0) {
        new_width = (width * new_height) / height;
    }

    if (new_width < 1) new_width = 1;
    if (new_height < 1) new_height = 1;

    int fuzziness = opts.fuzziness;
    if (fuzziness < 1) fuzziness = 1;

    const unsigned char *src = pixels;
    unsigned char *owned = NULL;

    if (new_width != width || new_height != height) {
        owned = (unsigned char *)malloc((size_t)new_width * new_height * channels);
        if (!owned) return NULL;
        stbir_resize_uint8(pixels, width, height, 0,
                           owned, new_width, new_height, 0, channels);
        src = owned;
    }

    sb_t sb;
    size_t est = (size_t)new_height * ((size_t)new_width * 33 + 1);
    if (sb_init(&sb, est < 4096 ? 4096 : est) != 0) {
        free(owned);
        return NULL;
    }

    if (opts.braille) {
        for (int j = 0; j < new_height; j += 4) {
            for (int i = 0; i < new_width; i += 2) {
                int dots[8] = {0};
                for (int dy = 0; dy < 4; dy++) {
                    for (int dx = 0; dx < 2; dx++) {
                        int y = j + dy;
                        int x = i + dx;
                        if (y >= new_height || x >= new_width) continue;
                        int idx = dy * 2 + dx;
                        int pixel = (y * new_width + x) * channels;
                        unsigned char r = src[pixel];
                        unsigned char g = src[pixel + 1];
                        unsigned char b = src[pixel + 2];
                        int bright = (r + g + b) / 3;
                        int num = (bright * 8) / 256;
                        static int order[8] = {0, 3, 1, 4, 2, 5, 6, 7};
                        dots[idx] = 0;
                        for (int k = 0; k < num; k++) {
                            dots[order[k]] = 1;
                        }
                    }
                }
                int mid = (j * new_width + i) * channels;
                unsigned char r = src[mid];
                unsigned char g = src[mid + 1];
                unsigned char b = src[mid + 2];
                const char *color = get_color(r, g, b);
                char glyph[4];
                braille_char(glyph, dots[0], dots[1], dots[2], dots[3],
                             dots[4], dots[5], dots[6], dots[7]);
                sb_append(&sb, color);
                sb_append(&sb, glyph);
                sb_append(&sb, "\033[0m");
            }
            sb_append(&sb, "\n");
        }
    } else {
        int levels = fuzziness;
        for (int j = 0; j < new_height; j++) {
            for (int i = 0; i < new_width; i++) {
                int pixel = (j * new_width + i) * channels;
                unsigned char r = src[pixel];
                unsigned char g = src[pixel + 1];
                unsigned char b = src[pixel + 2];
                int avg_brightness = (r + g + b) / 3;
                char c;
                if (levels == 1) {
                    c = img2ascii_brightness[brightness_len / 2];
                } else {
                    int quantized = (avg_brightness + (255 / levels / 2)) /
                                    (256 / levels);
                    if (quantized >= levels) quantized = levels - 1;
                    int index = quantized * (brightness_len - 1) / (levels - 1);
                    if (index >= brightness_len) index = brightness_len - 1;
                    c = img2ascii_brightness[index];
                }
                const char *color = get_color(r, g, b);
                sb_append(&sb, color);
                char ch[2] = {c, '\0'};
                sb_append(&sb, ch);
                sb_append(&sb, "\033[0m");
            }
            if (j < new_height - 1) {
                sb_append(&sb, "\n");
            }
        }
    }

    free(owned);
    return sb.buf;
}
