#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "argparse.h"
#include "bitmap.h"
#include "image_creator.h"
#include "img2ascii.h"
#include <locale.h>
#include <wchar.h>

static const char *const usages[] = {
    "img2ascii [options] <input> [output] [height]",
    "img2ascii [options]",
    NULL,
};

int main(int argc, const char **argv) {
  setlocale(LC_ALL, "");
  int width, height, channels;
  int brightness_len = img2ascii_brightness_len;
  int desired_width = IMG2ASCII_DEFAULT_WIDTH;
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
      OPT_INTEGER('h', "height", &desired_height, "target height", NULL, 0, 0),
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
    img2ascii_opts opts;
    opts.target_width = desired_width;
    opts.target_height = desired_height;
    opts.fuzziness = levels;
    opts.braille = braille_mode;

    char *result = img2ascii(image, width, height, channels, opts);
    if (result) {
      printf("%s", result);
      free(result);
    }
    stbi_image_free(image);
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
