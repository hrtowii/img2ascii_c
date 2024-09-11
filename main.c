#include <stdio.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define TARGET_WIDTH 300

const char brightness[] = "`.',-~:;=+*#%@";

const char* get_color(int r, int g, int b) {
	static char color[27];
	sprintf(color, "\033[38;2;%d;%d;%dm]", r, g, b);
	return color;
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

	int new_height = (height * TARGET_WIDTH) / width;
    unsigned char *resized_image = malloc(TARGET_WIDTH * new_height * channels);
	stbir_resize_uint8(image, width, height, 0, resized_image, TARGET_WIDTH, new_height, 0, channels);
    int max_buffer_size = new_height * TARGET_WIDTH * (27 + 2 + 10);
    char *art_buffer = malloc(max_buffer_size);
    if (art_buffer == NULL) {
        printf("Failed to allocate memory for art buffer\n");
        stbi_image_free(image);
        free(resized_image);
        return -1;
    }
    art_buffer[0] = '\0';
	for (int j = 0; j < new_height; j++) {
		for (int i = 0; i < TARGET_WIDTH; i++) {
			int pixel = (j * TARGET_WIDTH + i) * channels;
			unsigned char r = resized_image[pixel];
			unsigned char g = resized_image[pixel + 1];
			unsigned char b = resized_image[pixel + 2];
			int avg_brightness = (r + g + b) / 3;
			char c = brightness[avg_brightness * (brightness_len - 1) / 255];
			const char* color = get_color(r, g, b);
			snprintf(art_buffer + strlen(art_buffer), max_buffer_size - strlen(art_buffer), "%s%c%s", color, c, "\033[0m");
            // printf("%s%c%s", color, c, "\033[0m");
		}
		// printf("\n");
		snprintf(art_buffer + strlen(art_buffer), max_buffer_size - strlen(art_buffer), "\n");
	}
	printf("%s", art_buffer);
	stbi_image_free(image);
	free(resized_image);
	free(art_buffer);
	return 0;
}
