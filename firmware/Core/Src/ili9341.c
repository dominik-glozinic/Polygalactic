#include "stm32f1xx_hal.h"
#include "ili9341.h"
#include <stdlib.h>
#include <string.h>

static void ILI9341_Select(void) {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}
void ILI9341_Unselect(void) {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}
static void ILI9341_Reset(void) {
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_SET);
}
static void ILI9341_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ILI9341_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
}
static void ILI9341_WriteData(uint8_t *buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    while (buff_size > 0) {
        uint16_t chunk = buff_size > 32768 ? 32768 : (uint16_t)buff_size;
        HAL_SPI_Transmit(&ILI9341_SPI_PORT, buff, chunk, HAL_MAX_DELAY);
        buff += chunk;
        buff_size -= chunk;
    }
}
static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0,
                                      uint16_t x1, uint16_t y1) {
    ILI9341_WriteCommand(0x2A);
    { uint8_t d[]={x0>>8,x0&0xFF,x1>>8,x1&0xFF}; ILI9341_WriteData(d,4); }
    ILI9341_WriteCommand(0x2B);
    { uint8_t d[]={y0>>8,y0&0xFF,y1>>8,y1&0xFF}; ILI9341_WriteData(d,4); }
    ILI9341_WriteCommand(0x2C);
}

void ILI9341_Init(void) {
    ILI9341_Select();
    ILI9341_Reset();
    ILI9341_WriteCommand(0x01); HAL_Delay(1000);
    ILI9341_WriteCommand(0xCB);
    { uint8_t d[]={0x39,0x2C,0x00,0x34,0x02}; ILI9341_WriteData(d,5); }
    ILI9341_WriteCommand(0xCF);
    { uint8_t d[]={0x00,0xC1,0x30}; ILI9341_WriteData(d,3); }
    ILI9341_WriteCommand(0xE8);
    { uint8_t d[]={0x85,0x00,0x78}; ILI9341_WriteData(d,3); }
    ILI9341_WriteCommand(0xEA);
    { uint8_t d[]={0x00,0x00}; ILI9341_WriteData(d,2); }
    ILI9341_WriteCommand(0xED);
    { uint8_t d[]={0x64,0x03,0x12,0x81}; ILI9341_WriteData(d,4); }
    ILI9341_WriteCommand(0xF7);
    { uint8_t d[]={0x20}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0xC0);
    { uint8_t d[]={0x23}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0xC1);
    { uint8_t d[]={0x10}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0xC5);
    { uint8_t d[]={0x3E,0x28}; ILI9341_WriteData(d,2); }
    ILI9341_WriteCommand(0xC7);
    { uint8_t d[]={0x86}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0x36);
    { uint8_t d[]={0x48}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0x3A);
    { uint8_t d[]={0x55}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0xB1);
    { uint8_t d[]={0x00,0x18}; ILI9341_WriteData(d,2); }
    ILI9341_WriteCommand(0xB6);
    { uint8_t d[]={0x08,0x82,0x27}; ILI9341_WriteData(d,3); }
    ILI9341_WriteCommand(0xF2);
    { uint8_t d[]={0x00}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0x26);
    { uint8_t d[]={0x01}; ILI9341_WriteData(d,1); }
    ILI9341_WriteCommand(0xE0);
    { uint8_t d[]={0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,
                   0x37,0x07,0x10,0x03,0x0E,0x09,0x00}; ILI9341_WriteData(d,15); }
    ILI9341_WriteCommand(0xE1);
    { uint8_t d[]={0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,
                   0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F}; ILI9341_WriteData(d,15); }
    ILI9341_WriteCommand(0x11); HAL_Delay(120);
    ILI9341_WriteCommand(0x29);
    ILI9341_WriteCommand(0x36);
    { uint8_t d[]={ILI9341_ROTATION}; ILI9341_WriteData(d,1); }
    ILI9341_Unselect();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT) return;
    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x, y);
    uint8_t d[]={color>>8, color&0xFF};
    ILI9341_WriteData(d,2);
    ILI9341_Unselect();
}




#define PIXEL_BUF_PIXELS  2704   // 52*52 — covers radius-24 meteor + erase pad
static uint8_t pixelBuf[PIXEL_BUF_PIXELS * 2];   // 5408 bytes in BSS

void ILI9341_FillRectangle(uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height, uint16_t color)
{
    // Clip to screen
    if (x >= ILI9341_WIDTH  || y >= ILI9341_HEIGHT) return;
    if (x + width > ILI9341_WIDTH)  width = ILI9341_WIDTH  - x;
    if (y + height > ILI9341_HEIGHT) height = ILI9341_HEIGHT - y;
    if (width == 0 || height == 0) return;

    uint32_t total_pixels = (uint32_t)width * height;
    uint8_t  hi = color >> 8;
    uint8_t  lo = color & 0xFF;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x + width - 1, y + height - 1);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);

    if (total_pixels <= PIXEL_BUF_PIXELS) {
        /*
         * Should send all pixels in one go to prevent flicker --> doesnt fix the issue
         */
        uint32_t byte_count = total_pixels * 2;
        for (uint32_t i = 0; i < byte_count; i += 2) {
            pixelBuf[i]     = hi;
            pixelBuf[i + 1] = lo;
        }

        HAL_SPI_Transmit(&ILI9341_SPI_PORT, pixelBuf,
                          (uint16_t)byte_count, HAL_MAX_DELAY);
    } else {
        /*
         * Old fallback
         */
        uint16_t row_bytes = width * 2;
        for (uint16_t i = 0; i < width; i++) {
            pixelBuf[i * 2]     = hi;
            pixelBuf[i * 2 + 1] = lo;
        }
        for (uint16_t row = 0; row < height; row++) {
            HAL_SPI_Transmit(&ILI9341_SPI_PORT, pixelBuf,
                              row_bytes, HAL_MAX_DELAY);
        }
    }

    ILI9341_Unselect();
}

void ILI9341_FillScreen(uint16_t color) {
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawRectangle(uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height, uint16_t color)
{
    ILI9341_FillRectangle(x,         y,         width, 1, color);
    ILI9341_FillRectangle(x,         y + height - 1, width, 1, color);
    ILI9341_FillRectangle(x,         y,         1, height, color);
    ILI9341_FillRectangle(x + width - 1, y,         1, height, color);
}

static void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch,
                               FontDef font, uint16_t color, uint16_t bgcolor)
{
    ILI9341_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
    for (uint32_t i = 0; i < font.height; i++) {
        uint32_t b = font.data[(ch - 32) * font.height + i];
        for (uint32_t j = 0; j < font.width; j++) {
            uint16_t c = ((b << j) & 0x8000) ? color : bgcolor;
            uint8_t d[]={c>>8, c&0xFF};
            ILI9341_WriteData(d, 2);
        }
    }
}

void ILI9341_WriteString(uint16_t x, uint16_t y, const char *str,
                          FontDef font, uint16_t color, uint16_t bgcolor)
{
    ILI9341_Select();
    while (*str) {
        if (x + font.width >= ILI9341_WIDTH) {
            x = 0; y += font.height;
            if (y + font.height >= ILI9341_HEIGHT) break;
            if (*str == ' ') { str++; continue; }
        }
        ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    ILI9341_Unselect();
}

void ILI9341_DrawLine(uint16_t x0, uint16_t y0,
                       uint16_t x1, uint16_t y1, uint16_t color)
{
    int16_t dx =  abs((int16_t)x1-(int16_t)x0);
    int16_t dy = -abs((int16_t)y1-(int16_t)y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (1) {
        ILI9341_DrawPixel(x0, y0, color);
        if (x0==x1 && y0==y1) break;
        int16_t e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}
