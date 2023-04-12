// Copyright © 2018 Vouch.io LLC

#define DT_DRV_COMPAT st_st25r

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "platform.h"

#include "st25r.h"

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)

LOG_MODULE_DECLARE(ST25R);

static const struct device *s_spi_dev;
static const struct device *s_cs_dev;
static gpio_pin_t s_cs_pin;

static void usurp_cs_control(const struct device *dev)
{
    const struct st25r_device_config *config = dev->config;
    s_cs_dev = config->spi.config.cs->gpio.port;
    *((struct gpio_port **)&config->spi.config.cs->gpio.port) = NULL;
    s_cs_pin = config->spi.config.cs->gpio.pin;
}

int st25r_spi_init(const struct device *dev)
{
    const struct st25r_device_config *config = dev->config;

    if (!spi_is_ready(&config->spi)) {
        LOG_ERR("Bus device is not ready");
        return -ENODEV;
    }

    usurp_cs_control(dev);

    s_spi_dev = dev;

    return 0;
}

enum cs_dir {
    CS_DEASSERT = 0,
    CS_ASSERT = 1
};

static void cs_set(enum cs_dir dir)
{
    if (s_cs_dev && device_is_ready(s_cs_dev)) {
        gpio_pin_set(s_cs_dev, s_cs_pin, dir);
    } else {
        LOG_ERR("Unable to access CS");
    }
}

void platform_st25r_spi_select()
{
    LOG_DBG("SPI select");
    cs_set(CS_ASSERT);
}

void platform_st25r_spi_deselect()
{
    LOG_DBG("SPI deselect");
    cs_set(CS_DEASSERT);
}

void platform_st25r_spi_transceive(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len)
{
    LOG_DBG("platform_st25r_spi_transceive %d bytes, txBuf: %p rxBuf: %p", len, txBuf, rxBuf);

    const struct st25r_device_config *config = s_spi_dev->config;
    const struct spi_buf tx_buf[1] = {
            {
                    .buf = (uint8_t *) txBuf,
                    .len = len,
            },
    };
    const struct spi_buf_set tx = {
            .buffers = tx_buf,
            .count = ARRAY_SIZE(tx_buf),
    };
    const struct spi_buf rx_buf[1] = {
            {
                    .buf = rxBuf,
                    .len = len,
            },
    };
    const struct spi_buf_set rx = {
            .buffers = rx_buf,
            .count = ARRAY_SIZE(rx_buf),
    };

    if (spi_transceive_dt(&config->spi, &tx, &rx)) {
        LOG_ERR("error with spi transceive");
    }

}

#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
