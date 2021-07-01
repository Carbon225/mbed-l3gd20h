#include <l3gd20h.h>


#define SPI_MODE (3)
#define SPI_HZ (10000000)

#define SPI_WRITE_BIT     (0b00000000)
#define SPI_READ_BIT      (0b10000000)
#define SPI_INCREMENT_BIT (0b01000000)

#define L3GD20H_WHOAMI (0b11010111)

enum REG
{
    WHO_AM_I       = 0x0F,

    CTRL1          = 0x20,
    CTRL2          = 0x21,
    CTRL3          = 0x22,
    CTRL4          = 0x23,
    CTRL5          = 0x24,
    REFERENCE      = 0x25,
    OUT_TEMP       = 0x26,
    STATUS         = 0x27,

    OUT_X_L        = 0x28,
    OUT_X_H        = 0x29,
    OUT_Y_L        = 0x2A,
    OUT_Y_H        = 0x2B,
    OUT_Z_L        = 0x2C,
    OUT_Z_H        = 0x2D,

    FIFO_CTRL      = 0x2E,
    FIFO_SRC       = 0x2F,

    IG_CFG         = 0x30,
    IG_SRC         = 0x31,
    IG_THS_XH      = 0x32,
    IG_THS_XL      = 0x33,
    IG_THS_YH      = 0x34,
    IG_THS_YL      = 0x35,
    IG_THS_ZH      = 0x36,
    IG_THS_ZL      = 0x37,
    IG_DURATION    = 0x38,

    LOW_ODR        = 0x39,
};

L3GD20H::L3GD20H(PinName mosi, PinName miso, PinName sck, PinName cs)
    : _spi(mosi, miso, sck, cs, use_gpio_ssel)
{

}

int L3GD20H::init()
{
    _spi.format(8, SPI_MODE);
    _spi.frequency(SPI_HZ);

    if (who_am_i() != L3GD20H_WHOAMI)
        return 1;

    return 0;
}

int L3GD20H::write_reg(uint8_t reg, const uint8_t *data, int count, bool increment)
{
    _spi.select();

    uint8_t first_byte = reg;
    first_byte |= SPI_WRITE_BIT;
    if (increment)
        first_byte |= SPI_INCREMENT_BIT;

    _spi.write(first_byte);
    _spi.write((const char*) data, count, nullptr, 0);

    _spi.deselect();

    return 0;
}

int L3GD20H::read_reg(uint8_t reg, uint8_t *data, int count, bool increment)
{
    _spi.select();

    uint8_t first_byte = reg;
    first_byte |= SPI_READ_BIT;
    if (increment)
        first_byte |= SPI_INCREMENT_BIT;

    _spi.write(first_byte);
    _spi.write(nullptr, 0, (char*) data, count);

    _spi.deselect();

    return 0;
}

uint8_t L3GD20H::who_am_i()
{
    uint8_t val;
    read_reg(WHO_AM_I, &val, 1, false);
    return val;
}
