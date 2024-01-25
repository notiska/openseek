#include <stdlib.h>

#include "openseek/device.h"

#define INTERFACE 0

#define INPUT  LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_IN
#define OUTPUT LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT

int _request_set(seekdevice_t *device, seekcommand_t command, unsigned char *data, int data_len) {
    return libusb_control_transfer(device->handle, OUTPUT, command, 0, 0, data, data_len, device->timeout);
}

int _request_get(seekdevice_t *device, seekcommand_t command, unsigned char *data, int data_len) {
    return libusb_control_transfer(device->handle, INPUT, command, 0, 0, data, data_len, device->timeout);
}

/* ---------------------------------------- Init / De-init ---------------------------------------- */

int seek_init_device(seekdevice_t **device, int vendor_id, int product_id, int options) {
    (*device) = (seekdevice_t*)calloc(1, sizeof(seekdevice_t));
    if (!(*device)) return LIBUSB_ERROR_NO_MEM;

    (*device)->handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
    if (!(*device)->handle) return LIBUSB_ERROR_NO_DEVICE; // Should this be LIBUSB_ERROR_NOT_FOUND?

    (*device)->timeout = 1000;

    // Evil "method" trickery follows!!!
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

    // FIXME: Should we warn about not being able to read these?
    if (options & SEEK_READ_FW_VERSION) _request_get(*device, GET_FIRMWARE_INFO, (*device)->fw_version, 4);
    if (options & SEEK_READ_CHIP_ID)    _request_get(*device, READ_CHIP_ID,      (*device)->chip_id,    12);

    return 0;
}

void seek_deinit_device(seekdevice_t *device) {
#ifndef _WIN32
    libusb_attach_kernel_driver(device->handle, INTERFACE);
#endif // _WIN32
    libusb_close(device->handle);
    free(device);
}
