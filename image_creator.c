// TODO:
// 1. create a blank image
// 2. take in a struct of characters
// 3. parse the rgb value from the array which contains the struct
// 4. draw the text with color onto the image
// 5. download the image onto disk
#include <stdio.h>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

int create_image_from_ascii(struct ascii_character** character, int image_height, int image_width, char* output)
{
    int bitcount = 24; // 24-bit bitmap
    int width_in_bytes = ((image_width * bitcount + 31) / 32) * 4; // for padding
    uint32_t imagesize = width_in_bytes * image_height; // total image size

    struct my_BITMAPFILEHEADER filehdr = { 0 };
    struct my_BITMAPINFOHEADER infohdr = { 0 };

    memcpy(&filehdr, "BM", 2); // bitmap signature
    filehdr.bfSize = 54 + imagesize; // total file size
    filehdr.bfOffBits = 54; // sizeof(filehdr) + sizeof(infohdr)

    infohdr.biSize = 40; // sizeof(infohdr)
    infohdr.biPlanes = 1; // number of planes
    infohdr.biWidth = image_width;
    infohdr.biHeight = image_height;
    infohdr.biBitCount = bitcount;
    infohdr.biSizeImage = imagesize;

    unsigned char* buf = malloc(imagesize);
    if (!buf) {
        printf("Error allocating memory for image\n");
        return -1;
    }

    for (int row = image_height - 1; row >= 0; row--) {
        for (int col = 0; col < image_width; col++) {
            int pixel_index = row * width_in_bytes + col * 3;

            // Get the character's color from the struct array
            buf[pixel_index + 0] = character[row][col].color.blue;  // Blue channel
            buf[pixel_index + 1] = character[row][col].color.green; // Green channel
            buf[pixel_index + 2] = character[row][col].color.red;   // Red channel
        }
    }

    FILE *fout = fopen(output, "wb");
    if (!fout) {
        printf("Error opening file for writing\n");
        free(buf);
        return -1;
    }
    
    fwrite(&filehdr, sizeof(filehdr), 1, fout);
    fwrite(&infohdr, sizeof(infohdr), 1, fout);
    fwrite((char*)buf, 1, imagesize, fout);
    fclose(fout);

    free(buf);

    printf("Image successfully written to %s\n", output);
    return 0;
}

// int create_bitmap(int width, int height, char* filename)
// {
//     if(sizeof(struct my_BITMAPFILEHEADER) != 14 &&
//         sizeof(struct my_BITMAPINFOHEADER) != 40)
//     {
//         printf("bitmap structures not packed properly\n");
//         return 0;
//     }
//     int bitcount = 24; //<- 24-bit bitmap
//     int width_in_bytes = ((width * bitcount + 31) / 32) * 4;    //for padding
//     uint32_t imagesize = width_in_bytes * height;   //total image size

//     struct my_BITMAPFILEHEADER filehdr = { 0 };
//     struct my_BITMAPINFOHEADER infohdr = { 0 };

//     memcpy(&filehdr, "BM", 2);//bitmap signature
//     filehdr.bfSize = 54 + imagesize;//total file size
//     filehdr.bfOffBits = 54; //sizeof(filehdr) + sizeof(infohdr)

//     infohdr.biSize = 40; //sizeof(infohdr)
//     infohdr.biPlanes = 1; //number of planes is usually 1
//     infohdr.biWidth = width;
//     infohdr.biHeight = height;
//     infohdr.biBitCount = bitcount;
//     infohdr.biSizeImage = imagesize;

//     unsigned char* buf = malloc(imagesize);
//     for(int row = height - 1; row >= 0; row--)
//     {
//         for(int col = 0; col < width; col++)
//         {
//             buf[row * width_in_bytes + col * 3 + 0] = 255;//blue
//             buf[row * width_in_bytes + col * 3 + 1] = 0;//red
//             buf[row * width_in_bytes + col * 3 + 2] = 0;//green
//         }
//     }

//     FILE *fout = fopen(filename, "wb");
//     fwrite(&filehdr, sizeof(filehdr), 1, fout);
//     fwrite(&infohdr, sizeof(infohdr), 1, fout);
//     fwrite((char*)buf, 1, imagesize, fout);
//     fclose(fout);
//     free(buf);

//     return 0;
// }