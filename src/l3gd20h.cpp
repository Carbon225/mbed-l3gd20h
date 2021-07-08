#include <l3gd20h.h>


static const float MDPS_SCALE_FOR_FS[] = {
    8.75f,
    17.5f,
    70.f,
};

#define SPI_MODE (3)
#define SPI_HZ (10000000)

#define SPI_WRITE_BIT     (0b00000000)
#define SPI_READ_BIT      (0b10000000)
#define SPI_INCREMENT_BIT (0b01000000)

#define CTRL1_XEN_BIT (1u << 1)
#define CTRL1_YEN_BIT (1u << 0)
#define CTRL1_ZEN_BIT (1u << 2)
#define CTRL1_PD_BIT (1u << 3)
#define CTRL1_BW_OFFSET (4)
#define CTRL1_DR_OFFSET (6)

#define CTRL4_FS_OFFSET (4)

#define CTRL5_OUT_SEL_MASK (0b111)
#define CTRL5_OUT_SEL_OFFSET (0)
#define CTRL5_HP_EN_BIT (1u << 4)

#define LOW_ODR_I2C_DIS_BIT (1u << 3);
#define LOW_ODR_LOW_ODR_BIT (1u << 0);

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

enum ERROR_CODES
{
    ERROR_WHOAMI = 1,
};

L3GD20H::L3GD20H(PinName mosi, PinName miso, PinName sck, PinName cs)
    : _spi(mosi, miso, sck, cs, use_gpio_ssel)
    , _fs(L3G_FS_245DPS)
{

}

int L3GD20H::init(int dr, int bw, L3G_FS fs, bool low_odr)
{
    _spi.format(8, SPI_MODE);
    _spi.frequency(SPI_HZ);

    if (who_am_i() != L3GD20H_WHOAMI)
        return ERROR_WHOAMI;

    uint8_t val = 0;
    val |= LOW_ODR_I2C_DIS_BIT;
    if (low_odr)
        val |= LOW_ODR_LOW_ODR_BIT;
    write_reg(LOW_ODR, &val, 1, false);

    val = 0;
    val |= fs << CTRL4_FS_OFFSET;
    write_reg(CTRL4, &val, 1, false);
    _fs = fs;

    val = 0;
    val |= dr << CTRL1_DR_OFFSET;
    val |= bw << CTRL1_BW_OFFSET;
    write_reg(CTRL1, &val, 1, false);

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

int L3GD20H::set_enable(bool power, bool x, bool y, bool z)
{
    uint8_t val;
    read_reg(CTRL1, &val, 1, false);

    if (power)
        val |= CTRL1_PD_BIT;
    else
        val &= ~CTRL1_PD_BIT;

    if (x)
        val |= CTRL1_XEN_BIT;
    else
        val &= ~CTRL1_XEN_BIT;

    if (y)
        val |= CTRL1_YEN_BIT;
    else
        val &= ~CTRL1_YEN_BIT;

    if (z)
        val |= CTRL1_ZEN_BIT;
    else
        val &= ~CTRL1_YEN_BIT;

    write_reg(CTRL1, &val, 1, false);

    return 0;
}

int L3GD20H::read(float axes[3])
{
    uint8_t buf[3 * 2];
    read_reg(OUT_X_L, buf, sizeof buf, true);

    int16_t gx, gy, gz;

    gx = (int16_t) (buf[1] << 8 | buf[0]);
    gy = (int16_t) (buf[3] << 8 | buf[2]);
    gz = (int16_t) (buf[5] << 8 | buf[4]);

    axes[0] = MDPS_SCALE_FOR_FS[_fs] * (float) gx / 1000.f;
    axes[1] = MDPS_SCALE_FOR_FS[_fs] * (float) gy / 1000.f;
    axes[2] = MDPS_SCALE_FOR_FS[_fs] * (float) gz / 1000.f;

    return 0;
}

int L3GD20H::set_source(bool hp_en, int out_sel)
{
    uint8_t val;
    read_reg(CTRL5, &val, 1, false);

    val &= ~CTRL5_OUT_SEL_MASK;
    val |= out_sel << CTRL5_OUT_SEL_OFFSET;

    if (hp_en)
        val |= CTRL5_HP_EN_BIT;
    else
        val &= ~CTRL5_HP_EN_BIT;

    write_reg(CTRL5, &val, 1, false);

    return 0;
}
