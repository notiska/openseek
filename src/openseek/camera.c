#include <stdio.h>
#include <stdlib.h>

#include "openseek/camera.h"

#define SEEK_VENDOR_ID      0x289d
#define SEEK_PRODUCT_ID     0x0010
#define SEEK_PRO_PRODUCT_ID 0x0011

/* ---------------------------------------- Pretty ---------------------------------------- */

char *pretty_type(seekcamera_t *camera) {
    char *pretty = malloc(sizeof(char) * 11); // len("CompactPRO") = 10
    switch (camera->type) {
        default:
        case SEEK_COMPACT:     { sprintf(pretty, "Compact");    break; }
        case SEEK_COMPACT_PRO: { sprintf(pretty, "CompactPRO"); break; }
    }
    return pretty;
}

char *pretty_serial_no(seekcamera_t *camera) {
    char *pretty = malloc(sizeof(char) * 25);
    sprintf(
        pretty, "%c%c%c%c%c%c%c%c%c%c%c%c",
        camera->serial_no[0], camera->serial_no[1], camera->serial_no[2], camera->serial_no[3],
        camera->serial_no[4], camera->serial_no[5], camera->serial_no[6], camera->serial_no[7],
        camera->serial_no[8], camera->serial_no[9], camera->serial_no[10], camera->serial_no[11]
    );
    return pretty;
}

char *pretty_lens_fov(seekcamera_t *camera) {
    char *pretty = malloc(sizeof(char) * 7); // len("Narrow") = 6
    switch (camera->lens_fov) {
        default:
        case SEEK_FOV_WIDE:   { sprintf(pretty, "Wide");   break; }
        case SEEK_FOV_NARROW: { sprintf(pretty, "Narrow"); break; }
        case SEEK_FOV_NONE:   { sprintf(pretty, "None");   break; }
    }
    return pretty;
}

char *pretty_lens_focus(seekcamera_t *camera) {
    char *pretty = malloc(sizeof(char) * 7); // len("Manual") = 7
    switch (camera->lens_focus) {
        default:
        case SEEK_FOCUS_FIXED:  { sprintf(pretty, "Fixed");  break; }
        case SEEK_FOCUS_MANUAL: { sprintf(pretty, "Manual"); break; }
        case SEEK_FOCUS_NONE:   { sprintf(pretty, "None");   break; }
    }
    return pretty;
}

seekerror_t seek_init_camera(seekcamera_t **camera, seekdevice_t *device) {
    *camera = (seekcamera_t*)calloc(1, sizeof(seekcamera_t));
    if (!(*camera)) return SEEK_ERROR_NO_MEM;

    (*camera)->device = device;

    (*camera)->pretty_type       = &pretty_type;
    (*camera)->pretty_serial_no  = &pretty_serial_no;
    (*camera)->pretty_lens_fov   = &pretty_lens_fov;
    (*camera)->pretty_lens_focus = &pretty_lens_focus;

    if (!device) {
        seekerror_t res = seek_init_device(
            &(*camera)->device, SEEK_VENDOR_ID, SEEK_PRODUCT_ID,
            /* options */ SEEK_READ_FW_VERSION | SEEK_READ_CHIP_ID
        );
        device = (*camera)->device;
        if (!res) goto read_info; // gotos?!?! The devil!!

        (*camera)->type = SEEK_COMPACT_PRO;

        // Trying again, but with the CompactPRO product ID.
        res = seek_init_device(
            &(*camera)->device, SEEK_VENDOR_ID, SEEK_PRO_PRODUCT_ID,
            /* options */ SEEK_READ_FW_VERSION | SEEK_READ_CHIP_ID
        );
        device = (*camera)->device;
        if (res) return res;
    } else {
        struct libusb_device_descriptor descriptor;
        int res = libusb_get_device_descriptor(libusb_get_device(device->handle), &descriptor);
        if (res < 0) return SEEK_ERROR_UNKNOWN;

        if (descriptor.idProduct == SEEK_PRODUCT_ID) {
            (*camera)->type = SEEK_COMPACT;
        } else {
            (*camera)->type = SEEK_COMPACT_PRO;
        }
    }

read_info:;
    unsigned char data[2];
    seekerror_t res;

    // Interestingly enough, the Seek CompactPRO seems to be able to read up to 20 bytes, though discards the last
    // 18. The Compact only reads 2 bytes, though.
    res = device->get_firmware_info(device, SEEK_HARDWARE_VERSION, data, 2);
    if (res) return res;
    (*camera)->hw_version = data[1] << 8 | data[0];

    res = device->get_factory_setting(device, SEEK_SERIAL_NO, (*camera)->serial_no, 12);
    if (res) return res;

    (*camera)->lens_fov = SEEK_FOV_NONE;
    res = device->get_factory_setting(device, SEEK_LENS_FOV, data, 2);
    if (!res) {
        switch (data[1] << 8 | data[0]) {
            case 0:     { (*camera)->lens_fov = SEEK_FOV_WIDE;   break; }
            case 1:     { (*camera)->lens_fov = SEEK_FOV_NARROW; break; }
            case 65535: { (*camera)->lens_fov = SEEK_FOV_WIDE;   break; }
        }
    }

    (*camera)->lens_focus = SEEK_FOCUS_NONE;
    res = device->get_factory_setting(device, SEEK_LENS_FOCUS, data, 2);
    if (!res) {
        switch (data[1] << 8 | data[0]) {
            case 0:     { (*camera)->lens_focus = SEEK_FOCUS_FIXED;  break; }
            case 1:     { (*camera)->lens_focus = SEEK_FOCUS_MANUAL; break; }
            case 65535: { (*camera)->lens_focus = SEEK_FOCUS_FIXED;  break; }
        }
    }

    return SEEK_ERROR_NONE;
}

void seek_deinit_camera(seekcamera_t *camera) {
    camera->device->set_opmode(camera->device, SEEK_SLEEPING); // Idle before we free the device.
    seek_deinit_device(camera->device);
    free(camera);
}
