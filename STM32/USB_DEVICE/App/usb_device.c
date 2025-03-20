/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

/* USER CODE BEGIN Includes */

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */


#include "cmsis_os.h"

// Declare the mutex as a static variable so it persists across calls
static osMutexId usbWriteMutex = NULL; // Mutex ID
osMutexDef(usbWriteMutex); // Define the mutex globally

int _write(int file, char *ptr, int len) {
    // Initialize the mutex if it hasn't been initialized yet
    if (usbWriteMutex == NULL) {
        usbWriteMutex = osMutexCreate(osMutex(usbWriteMutex)); // Create the mutex if it doesn't exist
        if (usbWriteMutex == NULL) {
            // Mutex creation failed, handle error as needed
            return -1;  // You may want to return an error code
        }
    }

    // Wait for the mutex before writing to the USB (critical section)
    osMutexWait(usbWriteMutex, osWaitForever);

    // Perform the transmission
    CDC_Transmit_HS((uint8_t*) ptr, len);

    // Optionally add a small delay to avoid flooding USB transmission
    osDelay(10);  // Adjust delay as needed (10ms here)

    // Release the mutex after transmission is complete
    osMutexRelease(usbWriteMutex);

    // Return the number of bytes written
    return len;
}
/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */
  HAL_PWREx_EnableUSBVoltageDetector();

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

