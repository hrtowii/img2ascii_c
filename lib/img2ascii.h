#ifndef IMG2ASCII_H
#define IMG2ASCII_H

#ifdef __cplusplus
extern "C" {
#endif

#define IMG2ASCII_DEFAULT_WIDTH 300

typedef struct {
    int target_width;
    int target_height;
    int fuzziness;
    int braille;
} img2ascii_opts;

char *img2ascii(const unsigned char *pixels, int width, int height,
                int channels, img2ascii_opts opts);

extern const char img2ascii_brightness[];
extern const int img2ascii_brightness_len;

#ifdef __cplusplus
}
#endif

#endif
