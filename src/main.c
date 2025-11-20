#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define TARGET_WIDTH 300
#include "argparse.h"
#include "bitmap.h"
#include "image_creator.h"
#include <locale.h>
#include <wchar.h>

const char brightness[] =
    "`.-':_,^=;><+!rc*/"
    "z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

const char *get_color(int r, int g, int b) {
  static char color[27];
  sprintf(color, "\033[38;2;%d;%d;%dm", r, g, b);
  return color;
}

static inline unsigned int braille_from_dots(int d1, int d2, int d3, int d4,
                                             int d5, int d6, int d7, int d8) {
  return 0x2800 | (d1 ? 1 << 0 : 0) | (d2 ? 1 << 1 : 0) | (d3 ? 1 << 2 : 0) |
         (d4 ? 1 << 3 : 0) | (d5 ? 1 << 4 : 0) | (d6 ? 1 << 5 : 0) |
         (d7 ? 1 << 6 : 0) | (d8 ? 1 << 7 : 0);
}

char *braille_char(char buf[5], int d1, int d2, int d3, int d4, int d5, int d6,
                   int d7, int d8) {
  unsigned int cp = braille_from_dots(d1, d2, d3, d4, d5, d6, d7, d8);

  buf[0] = 0xE0 | ((cp >> 12) & 0x0F);
  buf[1] = 0x80 | ((cp >> 6) & 0x3F);
  buf[2] = 0x80 | (cp & 0x3F);
  buf[3] = '\0';

  return buf;
}

static const char *const usages[] = {
    "img2ascii [options] <input> [output] [height]",
    "img2ascii [options]",
    NULL,
};

int main(int argc, const char **argv) {
  setlocale(LC_ALL, "");
  int width, height, channels;
  int brightness_len = strlen(brightness);
  int desired_width = TARGET_WIDTH;
  int desired_height = 0;
  int terminal_mode = 0;
  int braille_mode = 0;
  int levels = 1;

  const char *input_path = NULL;
  const char *output_path = NULL;

  struct argparse_option options[] = {
      OPT_HELP(),
      OPT_GROUP("options"),
      OPT_INTEGER('w', "width", &desired_width, "target width", NULL, 0, 0),
      OPT_INTEGER('he', "height", &desired_height, "target height", NULL, 0, 0),
      OPT_INTEGER('f', "fuzziness", &levels,
                  "fuzziness: how many brightness steps share the same char",
                  NULL, 0, 0),
      OPT_STRING('o', "output", &output_path, "output image file", NULL, 0, 0),
      OPT_BOOLEAN('t', "terminal", &terminal_mode, "output to terminal only",
                  NULL, 0, 0),
      OPT_BOOLEAN('b', "braille", &braille_mode,
                  "print braille characters for smaller mapping", NULL, 0, 0),
      OPT_END(),
  };

  struct argparse argparse;
  argparse_init(&argparse, options, usages, 0);
  argparse_describe(&argparse, "\nConvert images to ASCII art.",
                    "\nIf no output file is specified, the ASCII art will be "
                    "displayed in the terminal.\nUse -w for width and -h for "
                    "height to control terminal output dimensions.");
  argc = argparse_parse(&argparse, argc, argv);

  if (argc == 0) {
    fprintf(stderr, "input image file is required\n");
    return -1;
  }

  input_path = argv[0];
  if (argc > 1 && !output_path) {
    output_path = argv[1];
  }
  if (argc > 2 && desired_height == 0) {
    char *endptr = NULL;
    desired_height = strtol(argv[2], &endptr, 10);
    if (endptr == argv[2] || *endptr != '\0') {
      fprintf(stderr, "Invalid height: %s\n", argv[2]);
      return -1;
    }
  }
  unsigned char *image = stbi_load(input_path, &width, &height, &channels, 0);
  if (image == NULL) {
    printf("error loading in image");
    return -1;
  }
  if (!output_path || terminal_mode) {
    int new_height;
    if (desired_height > 0) {
      new_height = desired_height;
    } else {
      new_height = (height * desired_width) / width;
    }
    int new_width = desired_width;
    unsigned char *resized_image = malloc(new_width * new_height * channels);
    stbir_resize_uint8(image, width, height, 0, resized_image, new_width,
                       new_height, 0, channels);
    if (braille_mode) {
      for (int j = 0; j < new_height; j += 4) {
        for (int i = 0; i < new_width; i += 2) {

          int dots[8] = {0};

          for (int dy = 0; dy < 4; dy++) {
            for (int dx = 0; dx < 2; dx++) {
              int y = j + dy;
              int x = i + dx;

              if (y >= new_height || x >= new_width)
                continue;

              int idx = dy * 2 + dx;
              int pixel = (y * new_width + x) * channels;

              unsigned char r = resized_image[pixel];
              unsigned char g = resized_image[pixel + 1];
              unsigned char b = resized_image[pixel + 2];
              int bright = (r + g + b) / 3;

              int num = (bright * 8) / 256; // 0..8

              static int order[8] = {0, 3, 1, 4, 2, 5, 6, 7};

              dots[idx] = 0; // reset
              for (int k = 0; k < num; k++) {
                dots[order[k]] = 1;
              }
            }
          }

          int mid = (j * new_width + i) * channels;
          unsigned char r = resized_image[mid];
          unsigned char g = resized_image[mid + 1];
          unsigned char b = resized_image[mid + 2];
          const char *color = get_color(r, g, b);

          char glyph[5];
          char *bchr = braille_char(glyph, dots[0], dots[1], dots[2], dots[3],
                                    dots[4], dots[5], dots[6], dots[7]);

          printf("%s%s\033[0m", color, bchr);
        }
        printf("\n");
      }

    } else {
      for (int j = 0; j < new_height; j++) {
        for (int i = 0; i < new_width; i++) {
          int pixel = (j * new_width + i) * channels;
          unsigned char r = resized_image[pixel];
          unsigned char g = resized_image[pixel + 1];
          unsigned char b = resized_image[pixel + 2];
          int avg_brightness = (r + g + b) / 3;
          int quantized = (avg_brightness + (255 / levels / 2)) /
                          (256 / levels); // proper rounding
          int index = quantized * (brightness_len - 1) / (levels - 1);
          index = index >= brightness_len ? brightness_len - 1 : index;
          char c = brightness[index];
          const char *color = get_color(r, g, b);
          printf("%s%c%s", color, c, "\033[0m");
        }
        printf("\n");
      }
    }
    stbi_image_free(image);
    free(resized_image);
    return 0;
  } else {
    struct ascii_character **ascii_image =
        malloc(height * sizeof(struct ascii_character *));
    for (int i = 0; i < height; i++) {
      ascii_image[i] = malloc(width * sizeof(struct ascii_character));
    }
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        int pixel = (j * CHAR_HEIGHT * width + i * CHAR_WIDTH) * channels;
        if (pixel >= width * height * channels) {
          continue;
        }
        unsigned char r = image[pixel];
        unsigned char g = image[pixel + 1];
        unsigned char b = image[pixel + 2];
        int avg_brightness = (r + g + b) / 3;
        int quantized = (avg_brightness + (255 / levels / 2)) /
                        (256 / levels); // proper rounding
        int index = quantized * (brightness_len - 1) / (levels - 1);
        index = index >= brightness_len ? brightness_len - 1 : index;
        char c = brightness[index];
        ascii_image[j][i].brightness = c;
        ascii_image[j][i].color.red = r;
        ascii_image[j][i].color.green = g;
        ascii_image[j][i].color.blue = b;
      }
    }
    if (desired_height > 0) {
      int orig_w = width;
      int orig_h = height;
      int new_w =
          (int)((double)orig_w * ((double)desired_height / (double)orig_h));
      create_image_from_ascii(ascii_image, desired_height, new_w, output_path);
    } else {
      create_image_from_ascii(ascii_image, height, width, output_path);
    }
    for (int i = 0; i < height; i++) {
      free(ascii_image[i]);
    }
    free(ascii_image);
    free(image);
  }
}
