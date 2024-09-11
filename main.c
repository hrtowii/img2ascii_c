#include <stdio.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "color.h"
#define TARGET_WIDTH 500
const char brightness[] = "`.',-~:;=+*#%@";

const char* get_color(unsigned char r, unsigned char g, unsigned char b) {
    if (r > g && r > b) return RED;
    if (g > r && g > b) return GREEN;
    if (b > r && b > g) return BLUE;
    if (r > 200 && g > 200 && b > 200) return WHITE;
    if (r < 50 && g < 50 && b < 50) return BLACK;
    if (abs(r - g) < 30 && abs(g - b) < 30 && abs(r - b) < 30) return BGRAY;
    if (r > b && g > b && abs(r - g) < 50) return YELLOW;
    if (r > g && b > g && abs(r - b) < 50) return MAGENTA;
    if (g > r && b > r && abs(g - b) < 50) return CYAN;
    return WHITE;
}

int main(int argc, char* argv[]) {
	int width, height, channels;
	int brightness_len = strlen(brightness);
	if (argc != 2) {
		printf("usage: %s <imageinput>", argv[0]);
		return -1;
	}
	char* input = argv[1];
	unsigned char *image = stbi_load(input, &width, &height, &channels, 0);
	if (image == NULL) {
		printf("error loading in image");
		return -1;
	}

	int new_height = (height * TARGET_WIDTH) / width / 2;
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
            // printf("%c", c);
			const char* color = get_color(r, g, b);
            printf("%s%c%s", color, c, RESET);
		}
		printf("\n");
	}
	// printf("image: %dpx, %dpx, %d channels\n", width, height, channels);
	// for (int j = 0; j < height; j++) {
	// 	for (int i = 0; i < width; i++) {
	// 		int pixel = (j * width + i) * channels;
	// 		unsigned char r = image[pixel];
	// 		unsigned char g = image[pixel + 1];
	// 		unsigned char b = image[pixel + 2];
	// 		int avg_brightness = (r + g + b) / 3;
	// 		char c = brightness[avg_brightness * (brightness_len - 1) / 255];
    //         // printf("%c", c);
	// 		const char* color = get_color(r, g, b);
    //         printf("%s%c%s", color, c, RESET);
	// 	}
	// 	printf("\n");
	// }
	stbi_image_free(image);
	return 0;
}
