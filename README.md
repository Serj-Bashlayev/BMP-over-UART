# BMP-over-UART
## Simple implementation of bmp file transfer via UART..

The input data comes in RGB565 format. Through the UART is transmitted header and data for saving the receiver side of the bmp file.

Supports 15-bit, 16-bit and 24-bit color depth.

The real 16-bit BMP format stores data as RGB565. This is unchanged data from the LCD screen and therefore there is no loss of quality. But 16-bit is not supported by all viewers.

The 24-bit (RGB888) is excessive for my application, because the LCD mode is 16-bit (RGB565).

I used this code to save screenshots from the ILI9341 320x240 LCD on my computer.
The LCD is connected to STM32F407 and has commands to read CGRAM.

---
*Example application:*

```C
uint16_t row_buf[width];

void save_screenshort()
{
    const int bpp = 15; // Select BMP 15-bit color depths
    const int x  = 0;
    const int y = 0;
    const int width  = 320;
    const int height = 240;

    // create and send bmp-file handle
    send_bmp_handle(&huart1, width, height, bpp);
    
    // in bmp-file format first pixel: bottom/left
    int row = height;

    while (row--) {
        // get row data from LCD in RGB565 format
        ili9341_GetScreenShort(row_buf, x, y + row, width, 1);
        // convert and send data to PC via UART
        send_bmp_data(&huart1, row_buf, width, bpp);
    }
}
```
---
*Note: To save the received data in a file using the program "RS232 Data Logger".*

