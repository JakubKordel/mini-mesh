#include "nrf24l01meshDriver.h"

#include "cmsis_os.h"

static nrf24l01_handle_t gs_handle;

ReceiveCallbackFunction rcf;

uint8_t nrf24l01MeshInit(){
    uint8_t reg;

    DRIVER_NRF24L01_LINK_INIT(&gs_handle, nrf24l01_handle_t);
    DRIVER_NRF24L01_LINK_SPI_INIT(&gs_handle, nrf24l01_interface_spi_init);
    DRIVER_NRF24L01_LINK_SPI_DEINIT(&gs_handle, nrf24l01_interface_spi_deinit);
    DRIVER_NRF24L01_LINK_SPI_READ(&gs_handle, nrf24l01_interface_spi_read);
    DRIVER_NRF24L01_LINK_SPI_WRITE(&gs_handle, nrf24l01_interface_spi_write);
    DRIVER_NRF24L01_LINK_GPIO_INIT(&gs_handle, nrf24l01_interface_gpio_init);
    DRIVER_NRF24L01_LINK_GPIO_DEINIT(&gs_handle, nrf24l01_interface_gpio_deinit);
    DRIVER_NRF24L01_LINK_GPIO_WRITE(&gs_handle, nrf24l01_interface_gpio_write);
    DRIVER_NRF24L01_LINK_DELAY_MS(&gs_handle, nrf24l01_interface_delay_ms);
    DRIVER_NRF24L01_LINK_DEBUG_PRINT(&gs_handle, nrf24l01_interface_debug_print);
    DRIVER_NRF24L01_LINK_RECEIVE_CALLBACK(&gs_handle, nrf24l01_interface_receive_callback);

    nrf24l01_init(&gs_handle);
    osDelay(100);
    nrf24l01_set_active(&gs_handle, NRF24L01_BOOL_FALSE);
    osDelay(100);
    nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_PWR_UP, NRF24L01_BOOL_TRUE);
	  nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_CRCO, NRF24L01_MESH_DEFAULT_CRCO);
    nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_EN_CRC, NRF24L01_MESH_DEFAULT_ENABLE_CRC);
    nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_MASK_MAX_RT, NRF24L01_BOOL_FALSE);
    nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_MASK_TX_DS, NRF24L01_BOOL_FALSE);
    nrf24l01_set_config(&gs_handle, NRF24L01_CONFIG_MASK_RX_DR, NRF24L01_BOOL_FALSE);
    nrf24l01_set_auto_acknowledgment(&gs_handle, NRF24L01_PIPE_0, NRF24L01_MESH_DEFAULT_PIPE_0_AUTO_ACKNOWLEDGMENT);
    nrf24l01_set_rx_pipe(&gs_handle, NRF24L01_PIPE_0, NRF24L01_MESH_DEFAULT_RX_PIPE_0);
    nrf24l01_set_address_width(&gs_handle, NRF24L01_MESH_DEFAULT_ADDRESS_WIDTH);
    nrf24l01_auto_retransmit_delay_convert_to_register(&gs_handle, NRF24L01_MESH_DEFAULT_RETRANSMIT_DELAY, (uint8_t *)&reg);
    nrf24l01_set_auto_retransmit_delay(&gs_handle, reg);
    nrf24l01_set_auto_retransmit_count(&gs_handle, NRF24L01_MESH_DEFAULT_RETRANSMIT_COUNT);
    nrf24l01_set_channel_frequency(&gs_handle, NRF24L01_MESH_DEFAULT_CHANNEL_FREQUENCY);
	  nrf24l01_set_continuous_carrier_transmit(&gs_handle, NRF24L01_BOOL_FALSE);
    nrf24l01_set_force_pll_lock_signal(&gs_handle, NRF24L01_BOOL_FALSE);
    nrf24l01_set_data_rate(&gs_handle, NRF24L01_MESH_DEFAULT_DATA_RATE);
    nrf24l01_set_output_power(&gs_handle, NRF24L01_MESH_DEFAULT_OUTPUT_POWER);
    nrf24l01_clear_interrupt(&gs_handle, NRF24L01_INTERRUPT_RX_DR);
    nrf24l01_clear_interrupt(&gs_handle, NRF24L01_INTERRUPT_TX_DS);
    nrf24l01_clear_interrupt(&gs_handle, NRF24L01_INTERRUPT_MAX_RT);
    nrf24l01_clear_interrupt(&gs_handle, NRF24L01_INTERRUPT_TX_FULL);
    nrf24l01_set_pipe_0_payload_number(&gs_handle, NRF24L01_MESH_DEFAULT_PIPE_0_PAYLOAD);
	  nrf24l01_set_pipe_dynamic_payload(&gs_handle, NRF24L01_PIPE_0, NRF24L01_MESH_DEFAULT_PIPE_0_DYNAMIC_PAYLOAD);
	  nrf24l01_set_dynamic_payload(&gs_handle, NRF24L01_MESH_DEFAULT_DYNAMIC_PAYLOAD);
    nrf24l01_set_payload_with_ack(&gs_handle, NRF24L01_MESH_DEFAULT_PAYLOAD_WITH_ACK);
    osDelay(100);
    nrf24l01_set_tx_payload_with_no_ack(&gs_handle, NRF24L01_MESH_DEFAULT_TX_PAYLOAD_WITH_NO_ACK);

    nrf24l01_flush_tx(&gs_handle);
    nrf24l01_flush_rx(&gs_handle);

	  uint8_t addr[3] = NRF24L01_MESH_DEFAULT_RX_ADDR_0;

    nrf24l01_set_tx_address(&gs_handle, (uint8_t *)addr, NRF24L01_MESH_DEFAULT_ADDRESS_WIDTH + 2);

    nrf24l01_set_rx_pipe_0_address(&gs_handle, (uint8_t *)addr, NRF24L01_MESH_DEFAULT_ADDRESS_WIDTH + 2);

    nrf24l01_set_active(&gs_handle, NRF24L01_BOOL_TRUE);

    return 0;
}

uint8_t nrf24l01MeshDeinit(){
    if (nrf24l01_deinit(&gs_handle) != 0){
        return 1;
    }
    else{
        return 0;
    }
}

uint8_t nrf24l01MeshWritePayloadNoAck(uint8_t *buf, uint8_t len){

    nrf24l01_write_payload_with_no_ack(&gs_handle, buf, len);

    return 0;
}

uint8_t nrf24l01MeshSetActive(){
	nrf24l01_set_active(&gs_handle, NRF24L01_BOOL_TRUE);
	return 0;
}

uint8_t nrf24l01MeshSetInactive(){
	nrf24l01_set_active(&gs_handle, NRF24L01_BOOL_FALSE);
	return 0;
}

uint8_t nrf24l01MeshTypeSwitch(nrf24l01_type_t type){

	if (type == NRF24L01_TYPE_TX){
        nrf24l01_set_mode(&gs_handle, NRF24L01_MODE_TX);

    }
    else{
        nrf24l01_set_mode(&gs_handle, NRF24L01_MODE_RX);
    }
	return 0;
}

uint8_t nrf24l01MeshInteruptHandler(){

	uint8_t bufByte;

	uint8_t widths[17];
	uint8_t buffer[17][33];

	uint8_t msg[512];

	unsigned int counter = 0;

	int ptr = 0;

	int exitCounter = 0;

	while (exitCounter < 250){
		nrf24l01_interface_spi_read( 0x00 | 0x17, (uint8_t *)&bufByte, 1);

		if (bufByte % 2 == 0 ){
			nrf24l01_interface_spi_read(0x60, (uint8_t *)&widths[counter], 1);
			if (widths[counter] > 32) {
				nrf24l01_interface_spi_write(0xE2, NULL, 0);
				break;
			} else {
				nrf24l01_interface_spi_read(0x61, (uint8_t *)buffer[counter], widths[counter]);

				if ((unsigned int)buffer[counter][widths[counter]-1] == counter){
					if ((unsigned int)buffer[counter][widths[counter]-2] == 0) {
						for (int i = 0; i < counter; ++i) {
							for (int j = 0; j < widths[i]-2; j++){
								msg[ptr] = buffer[i][widths[i] - 3 - j];
								ptr++;
							}
						}

						rcf(msg, ptr);

						break;
					} else {
						counter++;
					}
				} else {
					break;
				}
			}
		} else {
			if (exitCounter == 0){
				break;
			}
			nrf24l01_interface_delay_us(1);
		}
		exitCounter++;
	}

	nrf24l01_interface_spi_read(0x00 | 0x07, (uint8_t *)&bufByte, 1);
	nrf24l01_interface_spi_write(0x20 | 0x07, (uint8_t *)&bufByte, 1);

	return 0;
}

uint8_t nrf24l01MeshSetReceiveCallback(ReceiveCallbackFunction callback){
	rcf = callback;
	return 0;
}
