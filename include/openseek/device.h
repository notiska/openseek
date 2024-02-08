#ifndef _DEVICE_H
#define _DEVICE_H

// FIXME: libusb-1.0 ??? WTF arch!!! Also appears to be a thing on Ubuntu, lol.
#include <libusb-1.0/libusb.h>

#include "openseek/command.h"
#include "openseek/error.h"
#include "openseek/feature.h"

#define SEEK_WORD_SIZE 2

/**
 * Seek custom URB "commands".
 * References:
 *  - `com.subi.usb.CustomRequests` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    _SEEK_GET_ERROR_CODE                  = 53,

    _SEEK_READ_CHIP_ID                    = 54,

    _SEEK_TOGGLE_SHUTTER                  = 55,
    _SEEK_SET_SHUTTER_POLARITY            = 56,
    _SEEK_GET_SHUTTER_POLARITY            = 57,

    _SEEK_SET_BIT_DATA_OFFSET             = 58,
    _SEEK_GET_BIT_DATA                    = 59,

    _SEEK_SET_OPERATION_MODE              = 60,
    _SEEK_GET_OPERATION_MODE              = 61,

    _SEEK_SET_IMAGE_PROCESSING_MODE       = 62,
    _SEEK_GET_IMAGE_PROCESSING_MODE       = 63,

    _SEEK_SET_DATA_PAGE                   = 64,
    _SEEK_GET_DATA_PAGE                   = 65,

    _SEEK_SET_CURRENT_COMMAND_ARRAY_SIZE  = 66,
    _SEEK_SET_CURRENT_COMMAND_ARRAY       = 67,
    _SEEK_GET_CURRENT_COMMAND_ARRAY       = 68,
    _SEEK_SET_DEFAULT_COMMAND_ARRAY_SIZE  = 69,
    _SEEK_SET_DEFAULT_COMMAND_ARRAY       = 70,
    _SEEK_GET_DEFAULT_COMMAND_ARRAY       = 71,

    _SEEK_SET_VDAC_ARRAY_OFFSET_AND_ITEMS = 72, // Great naming!
    _SEEK_SET_VDAC_ARRAY                  = 73,
    _SEEK_GET_VDAC_ARRAY                  = 74,
    _SEEK_SET_RDAC_ARRAY_OFFSET_AND_ITEMS = 75, // x2
    _SEEK_SET_RDAC_ARRAY                  = 76,
    _SEEK_GET_RDAC_ARRAY                  = 77,

    _SEEK_GET_FIRMWARE_INFO               = 78,
    _SEEK_UPLOAD_FIRMWARE_ROW_SIZE        = 79,

    _SEEK_BEGIN_MEMORY_WRITE              = 82,
    _SEEK_WRITE_MEMORY_DATA               = 80,
    _SEEK_COMPLETE_MEMORY_WRITE           = 81,

    _SEEK_START_GET_IMAGE_TRANSFER        = 83,

    _SEEK_TARGET_PLATFORM                 = 84,

    _SEEK_SET_FIRMWARE_INFO_FEATURES      = 85,

    _SEEK_SET_FACTORY_SETTINGS_FEATURES   = 86,
    _SEEK_SET_FACTORY_SETTINGS            = 87,
    _SEEK_GET_FACTORY_SETTINGS            = 88,

    _SEEK_RESET_DEVICE                    = 89,

    // TODO: Appears to be an unknown command 93 in later APKs.

} _seekcommand_t;

/**
 * Current Seek device operation mode. Not to be confused with what is labelled as the "status" in the APK.
 * References:
 *  - `com.tyriansystems.Seekware.SeekwareDevice` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    SEEK_SLEEPING       = 0,
    SEEK_RUNNING        = 1,
    SEEK_RUNNING_NO_AGC = 2, // TODO: Can't actually get this to work? More experimentation needed.
} seekopmode_t;

/**
 * The platform that the Seek device is talking to.
 * References:
 *  - `com.subi.usb.CustomRequests$TargetPlatform` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    SEEK_TARGET_UNKNOWN = -1, // Note: not actually recognised by the device.

    SEEK_TARGET_WINDOWS = 0,
    SEEK_TARGET_ANDROID = 1,
    SEEK_TARGET_MACOS   = 2,
    SEEK_TARGET_IOS     = 3,
} seekplatform_t;

typedef struct _SeekDevice seekdevice_t;
struct _SeekDevice {
    libusb_device_handle *handle;
    int                   timeout; // libusb timeout to use.

    // https://github.com/seekthermal/seekcamera-python/blob/main/seekcamera/camera.py#L78
    // Supposedly product.variant.major.minor. Interestingly enough, you can read more than 4 bytes though.
    unsigned char         fw_version[4];
    unsigned char         chip_id[12];

    seekplatform_t        _platform;

    /* ------------------------------ Hidden ------------------------------ */

    int (*_request_set)(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len);
    int (*_request_get)(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len);

    /* ------------------------------ Public ------------------------------ */

    seekerror_t      (*shutter_command)(seekdevice_t *device, seekshutter_command_t command);
    seekerror_t (*start_frame_transfer)(seekdevice_t *device, int frame_size);
    seekerror_t    (*get_firmware_info)(
        seekdevice_t *device, seekfirmare_feature_t feature, unsigned char *data, int data_len
    );
    seekerror_t  (*get_factory_setting)(
        seekdevice_t *device, seekfactory_feature_t feature, unsigned char *data, int data_len
    );
    // seekerror_t (*reset)(seekdevice_t *device);

    /* ------------------------------ Setters / Getters ------------------------------ */

    // TODO: Documentation!!!!

    seekopmode_t      (*opmode)(seekdevice_t *device);
    seekerror_t   (*set_opmode)(seekdevice_t *device, seekopmode_t opmode);

    seekplatform_t  (*platform)(seekdevice_t *device);
    seekerror_t (*set_platform)(seekdevice_t *device, seekplatform_t platform);

    /* ------------------------------ Pretty ------------------------------ */

    char* (*pretty_fw_version)(seekdevice_t *device);
    char*    (*pretty_chip_id)(seekdevice_t *device);
    char*     (*pretty_opmode)(seekdevice_t *device);
    char*   (*pretty_platform)(seekdevice_t *device);
};

enum SeekInitOptions {
    SEEK_READ_FW_VERSION = 0x01,
    SEEK_READ_CHIP_ID    = 0x02,
};

/**
 * Initialises a Seek device.
 * 
 * @param device The device to initialise.
 * @param vendor_id The USB vendor ID.
 * @param product_id The USB product ID.
 * @param options The initialisation options, see {@link SeekInitOptions}.
 * @return 0 if successful, otherwise an error code.
 */
// TODO: Custom errors.
seekerror_t seek_init_device(seekdevice_t **device, int vendor_id, int product_id, int options);

/**
 * Deinitialises a Seek device.
 * 
 * @param device The device to deinitialise.
 */
void seek_deinit_device(seekdevice_t *device);

#endif // _DEVICE_H
