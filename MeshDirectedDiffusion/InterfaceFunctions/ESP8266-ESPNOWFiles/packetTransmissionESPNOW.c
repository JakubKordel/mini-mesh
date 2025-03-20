/**
 * @file packetTransmission.c
 * @brief Packet transmission functions implementation for ESP8266 FreeRTOS SDK using ESP-NOW communication with direct neighbors.
 *
 * This implementation provides the necessary functions for packet transmission and reception within a mesh network using ESP8266 running FreeRTOS SDK. It leverages ESP-NOW communication for direct neighbor exchange.
 */


#include "packetTransmission.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"

#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>
#include <packetEncryption.h>

static MeshSystem * mSystem = NULL;

void dataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // No usage so far
}

void receivedDataThroughEspNow(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    uint8_t copiedData[MAX_TRANSMIT_PCKT_SIZE_BYTES];
    int copySize = data_len;
    if (copySize > MAX_TRANSMIT_PCKT_SIZE_BYTES) {
        copySize = MAX_TRANSMIT_PCKT_SIZE_BYTES;
    }
    memcpy(copiedData, data, copySize);

    packetReceivedCallback(mSystem, (void *)copiedData, copySize);
}

int initTransmissionFunctionality(MeshSystem *meshSystem) {
    mSystem = meshSystem;
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_config_t wifi_config = {
        .sta = {
            .channel = WIFI_CHANNEL,
        },
    };

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, WIFI_BW_HT40);

    ESP_ERROR_CHECK(esp_wifi_start());

    initEncryption(&mSystem->encryptionHandler);

    esp_now_init();

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
    uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    memcpy(peerInfo.peer_addr, broadcastAddress, ESP_NOW_ETH_ALEN);
    peerInfo.channel = 0;
    peerInfo.ifidx = WIFI_IF_STA;

    esp_now_add_peer(&peerInfo);

    esp_now_register_recv_cb(receivedDataThroughEspNow);
    esp_now_register_send_cb(dataSentCallback);

    return 0;
}

int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length) {
    uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return esp_now_send(broadcastAddress, packetPtr, length);
}

int deinitTransmissionFunctionality(MeshSystem *meshSystem) {
    deinitEncryption(&meshSystem->encryptionHandler);
    esp_now_unregister_recv_cb();
    esp_now_unregister_send_cb();
    esp_now_deinit();

    esp_event_loop_delete_default();

    return 0;
}