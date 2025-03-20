/**
 * @file meshAPI.h
 * @brief Header file for the Mesh API library used with ESP8266 RTOS SDK.
 */

#ifndef MESHAPI_H_
#define MESHAPI_H_

#include <InterestId.h>
#include <stdint.h>
#include <interestCallbackTable.h>

/**
 * @brief Handle representing a mesh network.
 */
typedef struct mesh_network_handle mesh_network_handle_t;

/**
 * @brief Join a mesh network.
 *
 * This function joins a mesh network with the specified Mesh ID and encryption key.
 *
 * @param[out] mnh Pointer to the mesh network handle.
 * @param[in] meshId The Mesh ID of the network to join.
 * @param[in] encrKey Pointer to the encryption key for network security.
 * @return 0 if successful, otherwise an error code.
 */
int joinNetwork(mesh_network_handle_t **mnh, uint32_t meshId, uint8_t encrKey[16]);

/**
 * @brief Subscribe to an interest in the mesh network.
 *
 * This function subscribes to an interest in the mesh network with the specified Interest ID, hop distance, maximum distance, and callback function.
 *
 * @param[in] mnh Pointer to the mesh network handle.
 * @param[in] interestId Pointer to the Interest ID to subscribe to.
 * @param[in] maxDist The maximum distance for the subscription.
 * @param[in] callback Pointer to the callback function for handling received data.
 * @return 0 if successful, otherwise an error code.
 */
int subscribe(mesh_network_handle_t *mnh, InterestId *interestId, uint8_t maxDist, void (*callback)(void *, int));

/**
 * @brief Publish an interest in the mesh network.
 *
 * This function publishes an interest in the mesh network with the specified Interest ID and callback function.
 *
 * @param[in] mnh Pointer to the mesh network handle.
 * @param[in] interestId Pointer to the Interest ID to publish.
 * @param[in] callback Pointer to the callback function for handling published data.
 * @param[in] params Additional parameters for the callback.
 * @return 0 if successful, otherwise an error code.
 */
int publish(mesh_network_handle_t *mnh, InterestId *interestId, void (*callback)(void *), void *params);

/**
 * @brief Send data along the gradient in the mesh network.
 *
 * This function sends the specified data along the gradient of the mesh network for the specified Interest ID.
 *
 * @param[in] mnh Pointer to the mesh network handle.
 * @param[in] interestId Pointer to the Interest ID for data transmission.
 * @param[in] data Pointer to the data to send.
 * @param[in] dataSize The size of the data to send.
 * @return 0 if successful, otherwise an error code.
 */
int sendDataAlongGradient(mesh_network_handle_t *mnh, InterestId *interestId, void *data, uint16_t dataSize);

/**
 * @brief Leave the mesh network.
 *
 * This function allows the device to leave the current mesh network.
 *
 * @param[in] mnh Pointer to the mesh network handle.
 * @return 0 if successful, otherwise an error code.
 */
int leaveNetwork(mesh_network_handle_t **mnh);

#endif /* MESHAPI_H_ */
