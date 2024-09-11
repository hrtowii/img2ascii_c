#include <stdio.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define TARGET_WIDTH 300
#include "image_creator.h"
#include "bitmap.h"
const char brightness[] = "`.',-~:;=+*#%@";

const char* get_color(int r, int g, int b) {
	static char color[27];
	sprintf(color, "\033[38;2;%d;%d;%dm]", r, g, b);
	return color;
}

int main(int argc, char* argv[]) {
	int width, height, channels;
	int brightness_len = strlen(brightness);
	if (argc < 2) {
		printf("usage: %s <imageinput> <imageoutput (optional)>", argv[0]);
		return -1;
	}
	char* input = argv[1];
	unsigned char *image = stbi_load(input, &width, &height, &channels, 0);
	if (image == NULL) {
		printf("error loading in image");
		return -1;
	}
	if (argv[2] == NULL) {
		int new_height = (height * TARGET_WIDTH) / width;
		unsigned char *resized_image = malloc(TARGET_WIDTH * new_height * channels);
		stbir_resize_uint8(image, width, height, 0, resized_image, TARGET_WIDTH, new_height, 0, channels);
		for (int j = 0; j < new_height; j++) {
			for (int i = 0; i < TARGET_WIDTH; i++) {
				int pixel = (j * TARGET_WIDTH + i) * channels;
				unsigned char r = resized_image[pixel];
				unsigned char g = resized_image[pixel + 1];
				unsigned char b = resized_image[pixel + 2];
				int avg_brightness = (r + g + b) / 3;
				char c = brightness[avg_brightness * (brightness_len - 1) / 255];
				const char* color = get_color(r, g, b);
				printf("%s%c%s", color, c, "\033[0m");
			}
			printf("\n");
		}
		stbi_image_free(image);
		free(resized_image);
		return 0;
	} else {
		char* output_path = argv[2];

        int ascii_width = width / CHAR_WIDTH;
        int ascii_height = height / CHAR_HEIGHT;
		// idea: create a hashmap that stores <character, rgb as a struct>
		struct ascii_character** ascii_image = malloc(height * sizeof(struct ascii_character*));
        for (int i = 0; i < height; i++) {
            ascii_image[i] = malloc(width * sizeof(struct ascii_character));
        }
		// struct ascii_character ascii_image[height][width];
        for (int j = 0; j < ascii_height; j++) {
            for (int i = 0; i < ascii_width; i++) {
                int pixel = (j * CHAR_HEIGHT * width + i * CHAR_WIDTH) * channels;
                unsigned char r = image[pixel];
                unsigned char g = image[pixel + 1];
                unsigned char b = image[pixel + 2];
                int avg_brightness = (r + g + b) / 3;
                char c = brightness[avg_brightness * (brightness_len - 1) / 255];
                ascii_image[j][i].brightness = c;
                ascii_image[j][i].color.red = r;
                ascii_image[j][i].color.green = g;
                ascii_image[j][i].color.blue = b;
            }
        }

        create_image_from_ascii(ascii_image, ascii_height, ascii_width, output_path);
		for (int i = 0; i < height; i++) {
            free(ascii_image[i]);
        }
        free(ascii_image);
		// create_bitmap(316, 316, "test.bmp");
	}
}
