#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <string.h>
#include <lwip/sockets.h>

#include <meshLibraryProcesses.h>
#include <meshSystemConfig.h>
#include <packetEncryption.h>

#define GPIO_IRQ_PIN 4
#define MAX_PAYLOAD_SIZE 32

#define UDP_PORT 50120 
#define BROADCAST_IP "255.255.255.255" 
#define WIFI_SSID "YourSsid"      
#define WIFI_PASSWORD "YourPassword" 
#define WIFI_MAXIMUM_RETRY 5

static const char *TAG = "wifi station";
static int s_retry_num = 0;

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

binary_semaphore_t *nrf_mutex = NULL;
static MeshSystem *mSystem = NULL;
SemaphoreHandle_t irqSemaphore;
int sockfd;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retrying to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got IP: %s", ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta() {
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    if (strlen(WIFI_PASSWORD) > 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // Wait for connection or fail
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to AP SSID:%s password:%s", WIFI_SSID, WIFI_PASSWORD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASSWORD);
    } else {
        ESP_LOGE(TAG, "Unexpected event");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

void udp_receive_task(void *pvParameters) {
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    char buffer[500];

    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(UDP_PORT);
    recv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) < 0) {
        ESP_LOGE(TAG, "Socket bind failed");
        vTaskDelete(NULL);
    }

    struct timeval timeout = { .tv_sec = 10, .tv_usec = 0 }; // 10-second timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (1) {
        int length = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&recv_addr, &addr_len);
        if (length > 0) {
            ESP_LOGI(TAG, "Received %d bytes from broadcast: %s", length, buffer);
            packetReceivedCallback(mSystem, buffer, length);
        } else if (length == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            ESP_LOGI(TAG, "recvfrom timed out");
            // Optional: Do any periodic work or handle timeout cases here
        }
    }
    vTaskDelete(NULL);
}

int initTransmissionFunctionality(MeshSystem *meshSystem) {
    mSystem = meshSystem;

    nrf_mutex = binary_semaphore_create(1);
    irqSemaphore = xSemaphoreCreateBinary();

    ESP_ERROR_CHECK(nvs_flash_init());
    // Initialize Wi-Fi connection
    wifi_init_sta();
    
    initEncryption(&mSystem->encryptionHandler);

    // Create the UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ESP_LOGE(TAG, "Socket creation failed");
        return -1;
    }

    // Start the UDP receive task
    xTaskCreate(udp_receive_task, "udp_receive_task", 4096, NULL, configMAX_PRIORITIES - 1, NULL);

    return 0;
}

int transmit_data(MeshSystem *meshSystem, uint8_t *packetPtr, int length) {
    struct sockaddr_in broadcast_addr;
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(UDP_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);

    // Set socket option to enable broadcast
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        ESP_LOGE(TAG, "Failed to enable broadcast option");
        return -1;
    }

    int sent_bytes = sendto(sockfd, packetPtr, length, 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
    if (sent_bytes < 0) {
        ESP_LOGE(TAG, "Broadcast send failed");
        return -1;
    }

    ESP_LOGI(TAG, "Broadcasted %d bytes", sent_bytes);
    return 0;
}

int deinitTransmissionFunctionality(MeshSystem *meshSystem) {
    esp_wifi_disconnect();
    close(sockfd);
    return 0;
}

