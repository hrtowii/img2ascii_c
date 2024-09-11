#include <stdio.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define TARGET_WIDTH 500
// each character is 7 wide, 13 tall
// convert each image to 500 px wide -> use the new width to print the image. im probably gonna ask claude for help for resizing the image
char brightness[] = "`.',-~:;=+*#%@";
int main(int argc, char* argv[]) {
	int width, height, channels;
	int brightness_len = strlen(brightness);
//	printf("%s", brightness);
	if (argc != 2) {
		printf("usage: %s <imageinput> <fileoutput>", argv[0]);
		return -1;
	}
	char* input = argv[1];
	unsigned char *image = stbi_load(input, &width, &height, &channels, 0);
	if (image == NULL) {
		printf("error loading in image");
		return -1;
	}
	
	printf("image: %dpx, %dpx, %d channels\n", width, height, channels);
	// int new_height = (height * TARGET_WIDTH) / width / 2;
    // unsigned char *resized_image = malloc(TARGET_WIDTH * new_height * channels);
    
    // stbir_resize_uint8(image, width, height, 0, resized_image, TARGET_WIDTH, new_height, 0, channels);
	// for (int j = 0; j < height; j++) {
	// 	for (int i = 0; i < width; i++) {
	// 		int index = j * width + i;
	// 		unsigned char r = image[index];
	// 		unsigned char g = image[index + 1];
	// 		unsigned char b = image[index + 2];
	// 		int avg_brightness = (r + g + b) / 3;
	// 		char c = brightness[avg_brightness * (brightness_len - 1) / 255];
            
    //         printf("%c", c);
	// 	}
	// 	printf("\n");
	// }
	stbi_image_free(image);
	return 0;
}
