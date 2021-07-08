#ifndef _LIB_L3GD20H_H_
#define _LIB_L3GD20H_H_


#include <mbed.h>

enum L3G_FS
{
    L3G_FS_245DPS = 0,
    L3G_FS_500DPS = 1,
    L3G_FS_2000DPS = 2,
};

class L3GD20H
{
public:
    L3GD20H(PinName mosi, PinName miso, PinName sck, PinName cs);

    int init(int dr, int bw, L3G_FS fs, bool low_odr);

    int set_enable(bool power, bool x, bool y, bool z);

    int set_source(bool hp_en, int out_sel);

    int read(float axes[3]);

private:
    SPI _spi;
    L3G_FS _fs;

    uint8_t who_am_i();

    int read_reg(uint8_t reg, uint8_t data[], int count, bool increment);
    int write_reg(uint8_t reg, const uint8_t data[], int count, bool increment);

};


#endif
