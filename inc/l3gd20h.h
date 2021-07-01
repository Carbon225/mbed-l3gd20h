#ifndef _LIB_L3GD20H_H_
#define _LIB_L3GD20H_H_


#include <mbed.h>

class L3GD20H
{
public:
    L3GD20H(PinName mosi, PinName miso, PinName sck, PinName cs);

    int init();

private:
    SPI _spi;

    uint8_t who_am_i();

    int read_reg(uint8_t reg, uint8_t data[], int count, bool increment);
    int write_reg(uint8_t reg, const uint8_t data[], int count, bool increment);

};


#endif
