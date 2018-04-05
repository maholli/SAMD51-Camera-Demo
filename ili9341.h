/*
 * ili9341.h
 *
 *  Created on: Apr 4, 2018
 *      Author: deanm
 */

#ifndef ILI9341_H_
#define ILI9341_H_

#include <SPI.h>

static SPISettings ili9341Settings(48000000, MSBFIRST, SPI_MODE0);

#define SPI_WRITE32(l)          _spi->transfer((l) >> 24); _spi->transfer((l) >> 16); _spi->transfer((l) >> 8); _spi->transfer(l)

#define ILI9341_TFTWIDTH   240
#define ILI9341_TFTHEIGHT  320

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

class ili9341{
public:
    ili9341(int8_t _CS, int8_t _DC, int8_t _RST, SPIClass *_SPI) : _cs(_CS), _dc(_DC), _rst(_RST), _spi(_SPI) {}
    ~ili9341() {}

    void begin(){

        //_spi->begin();

        // Control Pins
        pinMode(_dc, OUTPUT);
        digitalWrite(_dc, LOW);
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);

        // toggle RST low to reset
        if (_rst >= 0) {
            pinMode(_rst, OUTPUT);
            digitalWrite(_rst, HIGH);
            delay(100);
            digitalWrite(_rst, LOW);
            delay(100);
            digitalWrite(_rst, HIGH);
            delay(200);
        }

        startWrite();

        writeCommand(0xEF);
        spiWrite(0x03);
        spiWrite(0x80);
        spiWrite(0x02);

        writeCommand(0xCF);
        spiWrite(0x00);
        spiWrite(0XC1);
        spiWrite(0X30);

        writeCommand(0xED);
        spiWrite(0x64);
        spiWrite(0x03);
        spiWrite(0X12);
        spiWrite(0X81);

        writeCommand(0xE8);
        spiWrite(0x85);
        spiWrite(0x00);
        spiWrite(0x78);

        writeCommand(0xCB);
        spiWrite(0x39);
        spiWrite(0x2C);
        spiWrite(0x00);
        spiWrite(0x34);
        spiWrite(0x02);

        writeCommand(0xF7);
        spiWrite(0x20);

        writeCommand(0xEA);
        spiWrite(0x00);
        spiWrite(0x00);

        writeCommand(ILI9341_PWCTR1);    //Power control
        spiWrite(0x23);   //VRH[5:0]

        writeCommand(ILI9341_PWCTR2);    //Power control
        spiWrite(0x10);   //SAP[2:0];BT[3:0]

        writeCommand(ILI9341_VMCTR1);    //VCM control
        spiWrite(0x3e);
        spiWrite(0x28);

        writeCommand(ILI9341_VMCTR2);    //VCM control2
        spiWrite(0x86);  //--

        writeCommand(ILI9341_MADCTL);    // Memory Access Control
        spiWrite(0x48);

        writeCommand(ILI9341_VSCRSADD); // Vertical scroll
        _spi->transfer16(0);                 // Zero

        writeCommand(ILI9341_PIXFMT);
        spiWrite(0x55);

        writeCommand(ILI9341_FRMCTR1);
        spiWrite(0x00);
        spiWrite(0x18);

        writeCommand(ILI9341_DFUNCTR);    // Display Function Control
        spiWrite(0x08);
        spiWrite(0x82);
        spiWrite(0x27);

        writeCommand(0xF2);    // 3Gamma Function Disable
        spiWrite(0x00);

        writeCommand(ILI9341_GAMMASET);    //Gamma curve selected
        spiWrite(0x01);

        writeCommand(ILI9341_GMCTRP1);    //Set Gamma
        spiWrite(0x0F);
        spiWrite(0x31);
        spiWrite(0x2B);
        spiWrite(0x0C);
        spiWrite(0x0E);
        spiWrite(0x08);
        spiWrite(0x4E);
        spiWrite(0xF1);
        spiWrite(0x37);
        spiWrite(0x07);
        spiWrite(0x10);
        spiWrite(0x03);
        spiWrite(0x0E);
        spiWrite(0x09);
        spiWrite(0x00);

        writeCommand(ILI9341_GMCTRN1);    //Set Gamma
        spiWrite(0x00);
        spiWrite(0x0E);
        spiWrite(0x14);
        spiWrite(0x03);
        spiWrite(0x11);
        spiWrite(0x07);
        spiWrite(0x31);
        spiWrite(0xC1);
        spiWrite(0x48);
        spiWrite(0x08);
        spiWrite(0x0F);
        spiWrite(0x0C);
        spiWrite(0x31);
        spiWrite(0x36);
        spiWrite(0x0F);

        writeCommand(ILI9341_SLPOUT);    //Exit Sleep
        delay(120);
        writeCommand(ILI9341_DISPON);    //Display on
        delay(120);
        endWrite();

        _width  = ILI9341_TFTWIDTH;
        _height = ILI9341_TFTHEIGHT;
    }

    void window (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    {
        uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
        uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
        writeCommand(ILI9341_CASET); // Column addr set
        SPI_WRITE32(xa);
        writeCommand(ILI9341_PASET); // Row addr set
        SPI_WRITE32(ya);
        writeCommand(ILI9341_RAMWR); // write to RAM
    }

    void writeColor(uint16_t color, uint32_t len){
        uint8_t hi = color >> 8, lo = color;
        for (uint32_t t=len; t; t--){
            spiWrite(hi);
            spiWrite(lo);
        }
    }

    void setRotation(uint8_t m) {
        rotation = m % 4; // can't be higher than 3
        switch (rotation) {
            case 0:
                m = (MADCTL_MX | MADCTL_BGR);
                _width  = ILI9341_TFTWIDTH;
                _height = ILI9341_TFTHEIGHT;
                break;
            case 1:
                m = (MADCTL_MV | MADCTL_BGR);
                _width  = ILI9341_TFTHEIGHT;
                _height = ILI9341_TFTWIDTH;
                break;
            case 2:
                m = (MADCTL_MY | MADCTL_BGR);
                _width  = ILI9341_TFTWIDTH;
                _height = ILI9341_TFTHEIGHT;
                break;
            case 3:
                m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
                _width  = ILI9341_TFTHEIGHT;
                _height = ILI9341_TFTWIDTH;
                break;
        }

        startWrite();
        writeCommand(ILI9341_MADCTL);
        spiWrite(m);
        endWrite();
    }

    void fillrect(int x, int y, int w, int h, int color)
    {

        if((x >= _width) || (y >= _height)) return;
        int16_t x2 = x + w - 1, y2 = y + h - 1;
        if((x2 < 0) || (y2 < 0)) return;

        // Clip left/top
        if(x < 0) {
            x = 0;
            w = x2 + 1;
        }
        if(y < 0) {
            y = 0;
            h = y2 + 1;
        }

        // Clip right/bottom
        if(x2 >= _width)  w = _width  - x;
        if(y2 >= _height) h = _height - y;

        int32_t len = (int32_t)w * h;
        window(x, y, w, h);
        writeColor(color, len);
    }

    void startWrite(void)
    {
        _spi->beginTransaction(ili9341Settings);
        digitalWrite(_cs, LOW);
    }

    void endWrite(void)
    {
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();
    }

    int8_t      _cs, _dc, _rst;
    SPIClass    *_spi;
    uint16_t _width, _height;
    uint8_t rotation;

    void writeCommand(uint8_t cmd){
        digitalWrite(_dc, LOW);
        digitalWrite(_cs, LOW);
        _spi->transfer(cmd);
        digitalWrite(_dc, HIGH);
    }
    void        spiWrite(uint8_t v){
        _spi->transfer(v);
    }
};


#endif /* ILI9341_H_ */
