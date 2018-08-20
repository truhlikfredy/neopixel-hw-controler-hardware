#ifndef ANTON_NEOPIXEL_DRIVER
#define ANTON_NEOPIXEL_DRIVER

#include <stdint.h>

class NeoPixelDriver {
  private:
    uint32_t base;
    uint32_t pixels;

    void    writeRegister(uint16_t pixel, uint8_t value);
    uint8_t readRegister( uint16_t pixel);

  public:
    NeoPixelDriver(uint32_t base, uint32_t pixels);

    void setPixelLength(uint16_t pixels);
    // void setPixel(uint32_t color);
    // void setPixel(uint8_t red, uint8_t green, uint8_t blue);
    // void setPixelRaw(uint32_t color);

    void writeRegisterMax(uint16_t value);

    void writeRegisterCtrl(uint8_t value);

    uint8_t readRegisterCtrl();

    uint8_t testRegisterCtrl(uint8_t mask);
};

#endif