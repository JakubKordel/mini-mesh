#ifndef DRIVER_NRF24L01_INTERFACE_H
#define DRIVER_NRF24L01_INTERFACE_H

#include "driver_nrf24l01.h"

#ifdef __cplusplus
extern "C"{
#endif

uint8_t nrf24l01_interface_spi_init(void);

uint8_t nrf24l01_interface_spi_deinit(void);

uint8_t nrf24l01_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len);

uint8_t nrf24l01_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len);

uint8_t nrf24l01_interface_gpio_init(void);

uint8_t nrf24l01_interface_gpio_deinit(void);

uint8_t nrf24l01_interface_gpio_write(uint8_t data);

void nrf24l01_interface_delay_ms(uint32_t ms);

void nrf24l01_interface_delay_us(uint32_t us);

void nrf24l01_interface_debug_print(const char *const fmt, ...);

void nrf24l01_interface_receive_callback(uint8_t type, uint8_t num, uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif
