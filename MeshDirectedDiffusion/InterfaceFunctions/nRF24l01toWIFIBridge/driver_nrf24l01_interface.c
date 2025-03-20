#include "driver/gpio.h"
//#include "driver/spi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver_nrf24l01_interface.h"
#include <stdio.h>
#include <esp8266_spi_pro.h>
#include "esp_system.h"
#include "freertos/timers.h"
#include <rom/ets_sys.h>

#include <universal_semaphore.h>

binary_semaphore_t * spi_mutex = NULL;

uint8_t nrf24l01_interface_spi_init(void){

	spi_mutex = binary_semaphore_create(1);

	binary_semaphore_wait(spi_mutex);
	
	gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << GPIO_NUM_15),
        .mode = GPIO_MODE_OUTPUT,  
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  
        .intr_type = GPIO_INTR_DISABLE  
    };

    gpio_config(&gpio_cfg);
	gpio_set_level(GPIO_NUM_15, 1);
	
	nrf24l01_interface_gpio_init();
	
	spi_init(HSPI);
	
	spi_init_gpio(HSPI, SPI_CLK_USE_DIV);
	spi_clock(HSPI, 2, 5); //  8 MHz clock
	spi_tx_byte_order(HSPI, SPI_BYTE_ORDER_LOW_TO_HIGH);
	spi_rx_byte_order(HSPI, SPI_BYTE_ORDER_LOW_TO_HIGH);
	
	binary_semaphore_post(spi_mutex);

    return 0;
}

uint8_t nrf24l01_interface_spi_deinit(void){
	binary_semaphore_destroy(spi_mutex);
	spi_mutex = NULL;
    return 0;
}

uint8_t nrf24l01_interface_spi_read(uint8_t reg, uint8_t *buf, uint16_t len){
	//binary_semaphore_wait(spi_mutex);
	uint32_t in_bytes;
	int i = 0;
	gpio_set_level(GPIO_NUM_15, 0);
	
	spi_tx8(HSPI, reg);
		
	while (i < len){
		if (i + 4 <= len ){
			in_bytes = spi_rx32(HSPI);
			memcpy(buf + i, &in_bytes, 4);
			i += 4;
		} else {
			in_bytes = spi_transaction(HSPI, 0, 0, 0, 0, 0, 0, (len - i)*8, 0);
			if ( i < len){
				buf[i++] = ((uint8_t*)&in_bytes)[0];
			}
			if (i < len){
				buf[i++] = ((uint8_t*)&in_bytes)[1];
			}
			if (i < len){
				buf[i++] = ((uint8_t*)&in_bytes)[2];
			}
			if (i < len){
				buf[i++] = ((uint8_t*)&in_bytes)[3];
			}
		}
	}
	
	gpio_set_level(GPIO_NUM_15, 1);
	//binary_semaphore_post(spi_mutex);

    return 0;
}

uint8_t nrf24l01_interface_spi_write(uint8_t reg, uint8_t *buf, uint16_t len){
	//binary_semaphore_wait(spi_mutex);
	int i = 0;
	gpio_set_level(GPIO_NUM_15, 0);
	
	uint32_t dout = 0;
	
	spi_tx8(HSPI, reg);
		
	while (i < len){
		if (i + 4 <= len ){
			memcpy(&dout, buf + i, 4);
			spi_tx32(HSPI, dout);
		} else {
			memcpy(&dout, buf + i, len - i);
			spi_txd(HSPI, (len - i)*8, dout);
		}
		i += 4;
	}
	
	gpio_set_level(GPIO_NUM_15, 1);
	//binary_semaphore_post(spi_mutex);

    return 0;
}

uint8_t nrf24l01_interface_gpio_init(void){
	
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << GPIO_NUM_4),  
        .mode = GPIO_MODE_OUTPUT, 
        .pull_up_en = GPIO_PULLUP_DISABLE,  
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  
        .intr_type = GPIO_INTR_DISABLE 
    };

    gpio_config(&gpio_cfg);

    return 0; 
}

uint8_t nrf24l01_interface_gpio_deinit(void){

    return 0; 
}

uint8_t nrf24l01_interface_gpio_write(uint8_t data){
    gpio_set_level(GPIO_NUM_4, data);

    return 0;
}

void nrf24l01_interface_delay_ms(uint32_t ms){
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void nrf24l01_interface_delay_us(uint32_t us){
    ets_delay_us(us);
}

#include <stdarg.h>

void nrf24l01_interface_debug_print(const char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void nrf24l01_interface_receive_callback(uint8_t type, uint8_t num, uint8_t *buf, uint8_t len){
	//Not used because interupts are handled differently
}


