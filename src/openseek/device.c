#include <stdio.h>
#include <stdlib.h>

#include "openseek/device.h"

#define INTERFACE 0

#define INPUT  LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_IN
#define OUTPUT LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT

/* ---------------------------------------- Hidden "Methods" ---------------------------------------- */

int _request_set(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len) {
    return libusb_control_transfer(device->handle, OUTPUT, command, 0, 0, data, data_len, device->timeout);
}

int _request_get(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len) {
    return libusb_control_transfer(device->handle, INPUT, command, 0, 0, data, data_len, device->timeout);
}

/* ---------------------------------------- Public "Methods" ---------------------------------------- */

char *pretty_fw_version(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 15); // len("255.255.255.255") = 15
    sprintf(
        pretty, "%i.%i.%i.%i",
        device->fw_version[0], device->fw_version[1], device->fw_version[2], device->fw_version[3]
    );
    return pretty;
}

char *pretty_chip_id(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 24);
    sprintf(
        pretty, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        device->chip_id[0], device->chip_id[1], device->chip_id[2], device->chip_id[3],
        device->chip_id[4], device->chip_id[5], device->chip_id[6], device->chip_id[7],
        device->chip_id[8], device->chip_id[9], device->chip_id[10], device->chip_id[11]
    );
    return pretty;
}

char *pretty_platform(seekdevice_t *device) {
    switch (device->_platform) {
        case SEEK_WINDOWS_TARGET: return "Windows";
        case SEEK_ANDROID_TARGET: return "Android";
        case SEEK_MACOS_TARGET:   return "MacOS";
        case SEEK_IOS_TARGET:     return "IOS";
        default:
        case SEEK_UNKNOWN_TARGET: return "Unknown";
    }
}

seekplatform_t platform(seekdevice_t *device) {
    return device->_platform;
}

int set_platform(seekdevice_t *device, seekplatform_t platform) {
    if (platform == device->_platform) return 0; // Nothing to do here.

    unsigned char data[1] = { (unsigned char)platform };
    int res = _request_set(device, _SEEK_TARGET_PLATFORM, data, 1);
    if (res < 0) return res;

    device->_platform = platform;
    return 0;
}

/* ---------------------------------------- Init / De-init ---------------------------------------- */

int seek_init_device(seekdevice_t **device, int vendor_id, int product_id, int options) {
    (*device) = (seekdevice_t*)calloc(1, sizeof(seekdevice_t));
    if (!(*device)) return LIBUSB_ERROR_NO_MEM;

    (*device)->handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
    if (!(*device)->handle) return LIBUSB_ERROR_NO_DEVICE; // Should this be LIBUSB_ERROR_NOT_FOUND?

    (*device)->timeout = 1000;
    (*device)->_platform = SEEK_UNKNOWN_TARGET;

    // Beware: evil "method" trickery follows!!!

    (*device)->pretty_fw_version = &pretty_fw_version;
    (*device)->pretty_chip_id    = &pretty_chip_id;
    (*device)->pretty_platform   = &pretty_platform;
    (*device)->platform     = &platform;
    (*device)->set_platform = &set_platform;

    (*device)->_request_set = &_request_set;
    (*device)->_request_get = &_request_get;

    int res;

#ifndef _WIN32
    res = libusb_attach_kernel_driver((*device)->handle, INTERFACE);
    if (res == LIBUSB_ERROR_NOT_FOUND) {
        ; // Not an issue, necessarily.
    } else if (res < 0) {
        libusb_close((*device)->handle);
        free(*device);
        return res;
    }
#endif // _WIN32

    res = libusb_claim_interface((*device)->handle, INTERFACE);
    if (res < 0) {
#ifndef _WIN32
        libusb_attach_kernel_driver((*device)->handle, INTERFACE);
#endif // _WIN32
        libusb_close((*device)->handle);
        free(*device);
        return res;
    }

    // set_platform(*device, SEEK_ANDROID_TARGET);

    // FIXME: Should we warn about not being able to read these?
    if (options & SEEK_READ_FW_VERSION) _request_get(*device, _SEEK_GET_FIRMWARE_INFO, (*device)->fw_version, 4);
    if (options & SEEK_READ_CHIP_ID)    _request_get(*device, _SEEK_READ_CHIP_ID,      (*device)->chip_id,    12);

    return 0;
}

void seek_deinit_device(seekdevice_t *device) {
#ifndef _WIN32
    libusb_attach_kernel_driver(device->handle, INTERFACE);
#endif // _WIN32
    libusb_close(device->handle);
    free(device);
}
