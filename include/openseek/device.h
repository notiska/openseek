#ifndef _DEVICE_H
#define _DEVICE_H

// FIXME: libusb-1.0 ??? WTF arch!!!
#include <libusb-1.0/libusb.h>

/**
 * Seek custom URB "commands".
 * References:
 *  - `com.subi.usb.CustomRequests` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    GET_ERROR_CODE                  = 53,

    READ_CHIP_ID                    = 54,

    TOGGLE_SHUTTER                  = 55,
    SET_SHUTTER_POLARITY            = 56,
    GET_SHUTTER_POLARITY            = 57,

    SET_BIT_DATA_OFFSET             = 58,
    GET_BIT_DATA                    = 59,

    SET_OPERATION_MODE              = 60,
    GET_OPERATION_MODE              = 61,

    SET_IMAGE_PROCESSING_MODE       = 62,
    GET_IMAGE_PROCESSING_MODE       = 63,

    SET_DATA_PAGE                   = 64,
    GET_DATA_PAGE                   = 65,

    SET_CURRENT_COMMAND_ARRAY_SIZE  = 66,
    SET_CURRENT_COMMAND_ARRAY       = 67,
    GET_CURRENT_COMMAND_ARRAY       = 68,
    SET_DEFAULT_COMMAND_ARRAY_SIZE  = 69,
    SET_DEFAULT_COMMAND_ARRAY       = 70,
    GET_DEFAULT_COMMAND_ARRAY       = 71,

    SET_VDAC_ARRAY_OFFSET_AND_ITEMS = 72, // Great naming!
    SET_VDAC_ARRAY                  = 73,
    GET_VDAC_ARRAY                  = 74,
    SET_RDAC_ARRAY_OFFSET_AND_ITEMS = 75, // x2
    SET_RDAC_ARRAY                  = 76,
    GET_RDAC_ARRAY                  = 77,

    GET_FIRMWARE_INFO               = 78,
    UPLOAD_FIRMWARE_ROW_SIZE        = 79,

    BEGIN_MEMORY_WRITE              = 82,
    WRITE_MEMORY_DATA               = 80,
    COMPLETE_MEMORY_WRITE           = 81,

    START_GET_IMAGE_TRANSFER        = 83,

    TARGET_PLATFORM                 = 84,

    SET_FIRMWARE_INFO_FEATURES      = 85,
    FACTORY_SETTINGS_FEATURES       = 86,
    SET_FACTORY_SETTINGS            = 87,
    GET_FACTORY_SETTINGS            = 88,

    RESET_DEVICE                    = 89,
} seekcommand_t;

typedef struct _seekdevice seekdevice_t;

struct _seekdevice {
    libusb_device_handle *handle;
    int                   timeout; // libusb timeout to use.

    unsigned char         fw_version[4];
    unsigned char         chip_id[12];

    /**
     * Requests to send data to the device via the provided command.
     * 
     * @param device The device to send to.
     * @param command The URB command.
     * @param data The data buffer to transfer.
     * @param data_len The length of the data buffer.
     * @return The amount of data that was transferred.
     */
    int (*_request_set)(seekdevice_t *device, seekcommand_t command, unsigned char *data, int data_len);

    /**
     * Requests to get data from the device via the provided command.
     * 
     * @param device The device to request from.
     * @param command The URB command.
     * @param data The return data buffer.
     * @param data_len The amount of data to request from the device.
     * @return The amount of data that was transferred.
     */
    int (*_request_get)(seekdevice_t *device, seekcommand_t command, unsigned char *data, int data_len);
};

enum InitOptions {
    SEEK_READ_FW_VERSION = 0x01,
    SEEK_READ_CHIP_ID    = 0x02,
};

/**
 * Initialises a Seek device.
 * 
 * @param device The device to initialise.
 * @param vendor_id The USB vendor ID.
 * @param product_id The USB product ID.
 * @param options The initialisation options, see {@link InitOptions}.
 * @return 0 if successful, otherwise a libusb error.
 */
int seek_init_device(seekdevice_t **device, int vendor_id, int product_id, int options);

/**
 * Deinitialises a Seek device.
 * 
 * @param device The device to deinitialise.
 */
void seek_deinit_device(seekdevice_t *device);

#endif // _DEVICE_H
