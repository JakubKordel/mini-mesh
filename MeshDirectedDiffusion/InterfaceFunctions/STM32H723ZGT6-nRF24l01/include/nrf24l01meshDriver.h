#ifndef NRF24L01_MESH_DRIVER_H
#define NRF24L01_MESH_DRIVER_H_H

#include <driver_nrf24l01.h>
#include "driver_nrf24l01_interface.h"

#define NRF24L01_MESH_DEFAULT_CRCO                              NRF24L01_BOOL_TRUE                 
#define NRF24L01_MESH_DEFAULT_ENABLE_CRC                        NRF24L01_BOOL_TRUE                    
#define NRF24L01_MESH_DEFAULT_PIPE_0_AUTO_ACKNOWLEDGMENT        NRF24L01_BOOL_FALSE                    
#define NRF24L01_MESH_DEFAULT_RX_PIPE_0                         NRF24L01_BOOL_TRUE                    
#define NRF24L01_MESH_DEFAULT_ADDRESS_WIDTH                     NRF24L01_ADDRESS_WIDTH_3_BYTES        
#define NRF24L01_MESH_DEFAULT_RETRANSMIT_DELAY                  0                                   
#define NRF24L01_MESH_DEFAULT_RETRANSMIT_COUNT                  0                                    
#define NRF24L01_MESH_DEFAULT_CHANNEL_FREQUENCY                 20                                    
#define NRF24L01_MESH_DEFAULT_DATA_RATE                         NRF24L01_DATA_RATE_2M                 
#define NRF24L01_MESH_DEFAULT_OUTPUT_POWER                      NRF24L01_OUTPUT_POWER_0_DBM           
#define NRF24L01_MESH_DEFAULT_PIPE_0_PAYLOAD                    32                                    

#define NRF24L01_MESH_DEFAULT_PIPE_0_DYNAMIC_PAYLOAD            NRF24L01_BOOL_TRUE                    

#define NRF24L01_MESH_DEFAULT_DYNAMIC_PAYLOAD                   NRF24L01_BOOL_TRUE                   
#define NRF24L01_MESH_DEFAULT_PAYLOAD_WITH_ACK                  NRF24L01_BOOL_FALSE                  
#define NRF24L01_MESH_DEFAULT_TX_PAYLOAD_WITH_NO_ACK            NRF24L01_BOOL_TRUE                  
#define NRF24L01_MESH_DEFAULT_RX_ADDR_0                         {0x1A, 0x01, 0x00}       				 

typedef void (*ReceiveCallbackFunction)(uint8_t *data, int length);

typedef enum{
    NRF24L01_TYPE_TX = 0x00,        
    NRF24L01_TYPE_RX = 0x01,       
} nrf24l01_type_t;

uint8_t nrf24l01MeshInit();

uint8_t nrf24l01MeshDeinit();

uint8_t nrf24l01MeshWritePayloadNoAck(uint8_t *buf, uint8_t len);

uint8_t nrf24l01MeshSetActive();

uint8_t nrf24l01MeshSetInactive();

uint8_t nrf24l01MeshTypeSwitch(nrf24l01_type_t type);

uint8_t nrf24l01MeshInteruptHandler();

uint8_t nrf24l01MeshSetReceiveCallback(ReceiveCallbackFunction callback);

#endif
