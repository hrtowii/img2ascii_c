// TODO:
// 1. create a blank image
// 2. take in a struct of characters
// 3. parse the rgb value from the array which contains the struct
// 4. draw the text with color onto the image
// 5. download the image onto disk
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "bitmap.h"

#pragma pack(push, 1)
struct my_BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct my_BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)
struct color
{
	int red;
	int green;
	int blue;
};

struct ascii_character
{
	char brightness;
	struct color color;
};

void draw_char(unsigned char* buf, int x, int y, char c, struct color color, int width, int height, int width_in_bytes) {
    const unsigned char* char_bitmap = get_char_bitmap(c);
    
    for (int dy = 0; dy < 8; dy++) {
        for (int dx = 0; dx < 5; dx++) {
            if (char_bitmap[dx] & (1 << dy)) {
                int px = x + dx;
                int py = y + dy;
                if (px < width && py < height) {
                    int index = (height - 1 - py) * width_in_bytes + px * 3;
                    buf[index] = color.blue;
                    buf[index + 1] = color.green;
                    buf[index + 2] = color.red;
                }
            }
        }
    }
}

int create_image_from_ascii(struct ascii_character** character, int image_height, int image_width, char* output)
{
    // int out_width = image_width * CHAR_WIDTH;
    // int out_height = image_height * CHAR_HEIGHT; // -> makes each character 1 pixel, image is huge
    int out_width = image_width;
    int out_height = image_height;
    int bitcount = 24; // 24-bit bitmap
    int width_in_bytes = ((out_width * bitcount + 31) / 32) * 4; // for padding
    uint32_t imagesize = width_in_bytes * out_height; // total image size

    // printf("Debug: Output dimensions: %d x %d\n", out_width, out_height);
    // printf("Debug: Image size: %u bytes\n", imagesize);

    struct my_BITMAPFILEHEADER filehdr = { 0 };
    struct my_BITMAPINFOHEADER infohdr = { 0 };

    filehdr.bfType = 0x4D42;
    filehdr.bfSize = 54 + imagesize; // total file size
    filehdr.bfOffBits = 54; // sizeof(filehdr) + sizeof(infohdr)

    infohdr.biSize = 40; // sizeof(infohdr)
    infohdr.biWidth = out_width;
    infohdr.biHeight = out_height;
    infohdr.biPlanes = 1;
    infohdr.biBitCount = bitcount;
    infohdr.biSizeImage = imagesize;

    unsigned char* buf = calloc(imagesize, 1);
    if (!buf) {
        printf("Error: Failed to allocate memory for image buffer\n");
        return -1;
    }
    memset(buf, 0, imagesize);

    for (int row = 0; row < image_height; row++) {
        for (int col = 0; col < image_width; col++) {
            int x = col * CHAR_WIDTH;
            int y = row * CHAR_HEIGHT;
            draw_char(buf, x, y, character[row][col].brightness, character[row][col].color, out_width, out_height, width_in_bytes);
        }
    }
    FILE *fout = fopen(output, "wb");
    if (!fout) {
        printf("Error: Failed to open file for writing: %s\n", output);
        free(buf);
        return -1;
    }
    
    fwrite(&filehdr, sizeof(filehdr), 1, fout);
    fwrite(&infohdr, sizeof(infohdr), 1, fout);
    fwrite(buf, 1, imagesize, fout);

    fclose(fout);
    free(buf);

    printf("Image successfully written to %s\n", output);
    return 0;
}