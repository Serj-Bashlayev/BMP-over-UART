#ifndef _BMP_H
#define _BMP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define BMPFILETYPE 0x4D42

#pragma pack(push, 2)
typedef struct BMPFileHeader {
    uint16_t bfType;            // magic number 'B''M'
    uint32_t bfSize;            // FileSize
    uint16_t bfReserved1;       // 0
    uint16_t bfReserved2;       // 0
    uint32_t bfOffBits;         // ImageDataOffset
} BMPFileHeader;

typedef struct BMPInfoHeader {
    uint32_t biSize;            // HeaderSize
    uint32_t biWidth;           // ImageWidth
    uint32_t biHeight;          // ImageHeight
    uint16_t biPlanes;          // NumberOfImagePlanes (alvays 1)
    uint16_t biBitCount;        // BitsPerPixel (...15, 16, 24..)
    uint32_t biCompression;     // CompressionMethod (0)
    uint32_t biSizeImage;       // SizeOfBitmap. conventional value for uncompressed images
    uint32_t biXPelsPerMeter;   // HorizonalResolution
    uint32_t biYPelsPerMeter;   // VerticalResolution
    uint32_t biClrUsed;         // NumberOfColorsUsed (0, dummy value)
    uint32_t biClrImportant;    // every color is important (0)
} BMPInfoHeader;

typedef struct BMPMask {
    uint32_t r_mask;            // 0xF800
    uint32_t g_mask;            // 0x07E0
    uint32_t b_mask;            // 0x001F
    uint32_t a_mask;            // 0x0000
} BMPMask;

typedef struct BMP_Header {
    BMPFileHeader	fileheader;
    BMPInfoHeader	infoheader;
}BMP_Header;

typedef struct BMP_Header_16bpp {
    BMPFileHeader	fileheader;
    BMPInfoHeader	infoheader;
    BMPMask         mask;
}BMP_Header_16bpp;
#pragma pack(pop)


void send_bmp_handle(UART_HandleTypeDef *huart, uint32_t width, uint32_t height, uint32_t bpp);
void send_bmp_data(UART_HandleTypeDef *huart, const uint16_t *p_data, uint32_t width, uint32_t bpp);

#ifdef __cplusplus
}
#endif

#endif /* _BMP_H */

