/***********************************************************************
 File Name    : 'bmp.c'
 Title        : BMP over UART
 Description  : Create and send BMP file over UART 
                3 color depths are supported:
                     24 bit (RGB888. bmp version: CORE)
                     15 bit (RGB555. bmp version: 3)
                     16 bit (RGB565. bmp version: 4) (no all software supported this format)

                To save to a file under Windows you can use the
                "RS232 Data Logger" (the best program for this),
                or the PuTTY logging features

 Author       : Serj Bashlayev [https://github.com/Serj-Bashlayev] [phreak_ua@yahoo.com]
 Created      : 08.04.2023
 Revised      : 09.04.2023
 Version      : 1.0.1
 Target MCU   : STM32
 Compiler     : Keil ARM Compiler v.6 (GNU C)
 Editor Tabs  : 4
***********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <assert.h>
#include "usart.h"
#include "bmp.h"

/**
  * @brief Sends a BMP file header to the UART
  * @note  Supports 3 formats:
  *         <li>24 bit (RGB888. CORE version)
  *         <li>15 bit (RGB555. 3 version)
  *         <li>16 bit (RGB565. 4 version (using mask) (no all software supported this format)
  *
  * @param huart  UART handle
  * @param width  width
  * @param height height
  * @param bpp    Color depth format (15, 16 or 24)
  * @retval None
  */
void send_bmp_handle(UART_HandleTypeDef *huart, uint32_t width, uint32_t height, uint32_t bpp)
{
    int32_t bytes_per_row;

    // support only 3 formats
    assert(bpp == 24 || bpp == 16 || bpp == 15);

    switch (bpp) {
    case 24:
        // bytes_per_row = (3 * (width + 1) / 4) * 4;
        // // bytes_per_row = 3 * width ;
        // // bytes_per_row += bytes_per_row % 4 ? 4 - bytes_per_row % 4 : 0;
        bytes_per_row = (3 * (width + 1)) & ~0x0003;
        break;
    case 16:
    case 15:
        bytes_per_row = (2 * (width + 1)) & ~0x0003;
        break;
    default:
        return;
    }

    // default create and fill 24bpp bitmap handle
    BMP_Header fh = {
        {
            .bfType = BMPFILETYPE,
            .bfSize = sizeof(BMP_Header) + bytes_per_row * height,
            .bfReserved1 = 0,
            .bfReserved2 = 0,
            .bfOffBits = sizeof(BMP_Header),
        }, {
            .biSize = sizeof(BMPInfoHeader),
            .biWidth = width,
            .biHeight = height,
            .biPlanes = 1,
            .biBitCount = 24,
            .biCompression = 0,
            .biSizeImage = bytes_per_row * height,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed = 0,
            .biClrImportant = 0,
        }
    };

    switch (bpp) {
    case 16:
        fh.infoheader.biBitCount = 16;
        fh.infoheader.biCompression = 3;
        fh.fileheader.bfSize += sizeof(BMPMask);
        fh.fileheader.bfOffBits += sizeof(BMPMask);
        fh.infoheader.biSize += sizeof(BMPMask);
        break;
    case 15:
        fh.infoheader.biBitCount = 16;
        break;
    }

    HAL_UART_Transmit(huart, (uint8_t *)&fh, sizeof(BMP_Header), HAL_MAX_DELAY);

    if (bpp == 16) {
        struct BMPMask mask = { 0xF800, 0x07E0, 0x001F, 0x0000 };
        HAL_UART_Transmit(huart, (uint8_t *)&mask, sizeof(BMPMask), HAL_MAX_DELAY);
    }
}


/**
  * @brief Sends bitmap data of BMP file to the UART
  * @note  Lines are transmitted from the bottom to top.<br>
  *        Transmit only one complete line at a time.<br>
  *
  * @param huart  UART handle
  * @param p_data pixels in RGB565 format
  * @param width  number of pixels per line (width)
  * @param bpp    color depth format (15, 16 or 24)
  * @retval None
  */
void send_bmp_data(UART_HandleTypeDef *huart, const uint16_t *p_data, uint32_t width, uint32_t bpp)
{
    int32_t remainder;
    int32_t pad_row_size;    // pad row size to a multiple of 4 Bytes
    int32_t bytes_per_pixel; // 24bpp = 3. 16bpp,15bpp = 2
    const uint16_t *p_tx_data;
    const uint8_t pad_byte[3] = { 0 };
    union {
        uint8_t brg[3];     // 24bpp: convert rgb565 to rgb888
        uint16_t rgb555;    // 15bpp: convert rgb565 to rgb555
    } pix_data;

    // support only 3 formats
    assert(bpp == 24 || bpp == 16 || bpp == 15);

    if (p_data == NULL || huart == NULL || width == 0)
        return;

    switch (bpp) {
    case 24:
        bytes_per_pixel = 3;
        p_tx_data = (uint16_t *)&pix_data;
        break;
    case 16:
        bytes_per_pixel = 2;
        p_tx_data = p_data;
        break;
    case 15:
        bytes_per_pixel = 2;
        p_tx_data = (uint16_t *)&pix_data;
        break;
    default:
        return;
    }

    remainder = (bytes_per_pixel * width) % 4;
    pad_row_size = remainder ? (4 - remainder) : 0;

    while (width--) {
        switch (bpp) {
        case 24:
            // in BMP format color sequence is bgr (blue - first)
            // RGB565 to 24bit conversion
            // RGB565 RRRRRGGG GGGBBBBB
            // int8_t          xxxxxxxx
            pix_data.brg[0] = (*p_data & 0x001F) << 3; // blue
            pix_data.brg[1] = (*p_data & 0x07E0) >> 3; // green
            pix_data.brg[2] = (*p_data & 0xF800) >> 8; // red
            p_data++;
            break;
        case 15:
            // RGB565 > 555
            // RGB565 RRRRRGGG GGGBBBBB
            // RGB555 0RRRRRGG GGGBBBBB
            pix_data.rgb555 = (*p_data & 0x001F) | ((*p_data >> 1) & 0x7FE0);
            p_data++;
            break;
        }

        HAL_UART_Transmit(huart, (uint8_t *)p_tx_data, bytes_per_pixel, HAL_MAX_DELAY);

        if (bpp == 16) {
            p_tx_data++;
        }
    }

    // send padding byte
    if (pad_row_size > 0) {
        HAL_UART_Transmit(huart, pad_byte, pad_row_size, HAL_MAX_DELAY);
    }
}

