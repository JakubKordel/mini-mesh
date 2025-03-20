#include "driver_nrf24l01_interface.h"
#include <stdio.h>
#include <stdarg.h>
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_tim.h"
#include "cmsis_os.h"

// Define CE, CSN, and IRQ pins
#define NRF_CE_PIN      GPIO_PIN_11
#define NRF_CE_PORT     GPIOD
#define NRF_CSN_PIN     GPIO_PIN_10
#define NRF_CSN_PORT    GPIOD

SPI_HandleTypeDef hspi1;

static void MX_SPI1_Init(void)
{
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 0x0;

    if (HAL_SPI_Init(&hspi1) != HAL_OK){
        printf("Error during SPI init\r\n");
    }
}

uint8_t nrf24l01_interface_spi_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = NRF_CSN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NRF_CSN_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(NRF_CSN_PORT, NRF_CSN_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = NRF_CE_PIN;
    HAL_GPIO_Init(NRF_CE_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, GPIO_PIN_RESET);  // Ensure CE is low initially

    MX_SPI1_Init();

    return 0;
}

uint8_t nrf24l01_interface_spi_deinit(void) {
    HAL_SPI_DeInit(&hspi1);
    return 0;
}

uint8_t SPI_Transmit(uint8_t data) {
    uint8_t receivedData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &data, &receivedData, 1, HAL_MAX_DELAY);
    return receivedData;
}

uint8_t nrf24l01_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len) {
    uint8_t tx_buf[1];
    int i = 0;

    HAL_GPIO_WritePin(NRF_CSN_PORT, NRF_CSN_PIN, GPIO_PIN_RESET);

    tx_buf[0] = reg;
    HAL_SPI_Transmit(&hspi1, tx_buf, 1, HAL_MAX_DELAY);

    for (i = 0; i < len; i++) {
        uint8_t byte_to_send = buf[i];
        HAL_SPI_Transmit(&hspi1, &byte_to_send, 1, HAL_MAX_DELAY);
    }

    HAL_GPIO_WritePin(NRF_CSN_PORT, NRF_CSN_PIN, GPIO_PIN_SET);

    return 0;
}

uint8_t nrf24l01_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len) {
    uint8_t tx_buf[1];
    int i = 0;

    HAL_GPIO_WritePin(NRF_CSN_PORT, NRF_CSN_PIN, GPIO_PIN_RESET);

    tx_buf[0] = reg;
    HAL_SPI_Transmit(&hspi1, tx_buf, 1, HAL_MAX_DELAY);

    for (i = 0; i < len; i++) {
        uint8_t rx_byte;
        HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY);
        buf[i] = rx_byte;
    }

    HAL_GPIO_WritePin(NRF_CSN_PORT, NRF_CSN_PIN, GPIO_PIN_SET);

    return 0;
}

uint8_t nrf24l01_interface_gpio_init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = NRF_CE_PIN;
    HAL_GPIO_Init(NRF_CE_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, GPIO_PIN_RESET);

    return 0;
}

uint8_t nrf24l01_interface_gpio_deinit(void) {
    return 0;
}

uint8_t nrf24l01_interface_gpio_write(uint8_t data) {
   if (data > 0){
          HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, GPIO_PIN_SET);
   } else {
      HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, GPIO_PIN_RESET);
   }

    return 0;
}

void nrf24l01_interface_delay_ms(uint32_t ms) {
    osDelay(ms);
}

void nrf24l01_interface_delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t delay_ticks = (HAL_RCC_GetHCLKFreq() / 1000000) * us;
    while ((DWT->CYCCNT - start) < delay_ticks);
}

void nrf24l01_interface_debug_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\r\n");
}

void nrf24l01_interface_receive_callback(uint8_t type, uint8_t num, uint8_t *buf, uint8_t len) {
    // Not used because interrupts are handled differently
}
