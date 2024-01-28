#include <stdlib.h>

#include "openseek/camera.h"

#define SEEK_VENDOR_ID      0x289d
#define SEEK_PRODUCT_ID     0x0010
#define SEEK_PRO_PRODUCT_ID 0x0011

seekerror_t seek_init_camera(seekcamera_t **camera, seekdevice_t *device) {
    *camera = (seekcamera_t*)calloc(1, sizeof(seekcamera_t));
    if (!(*camera)) return SEEK_ERROR_NO_MEM;

    (*camera)->device = device;
    (*camera)->type = SEEK_COMPACT; // Default assumption.
    // TODO: Can we get the vendor and product IDs from the libusb handle? If so - we can determine a candidate for the
    //       device type that way.

    if (!device) {
        seekerror_t res = seek_init_device(
            &(*camera)->device, SEEK_VENDOR_ID, SEEK_PRODUCT_ID,
            /* options */ SEEK_READ_FW_VERSION | SEEK_READ_CHIP_ID
        );
        if (!res) goto read_info; // gotos?!?! The devil!!

        (*camera)->type = SEEK_COMPACT_PRO;

        // Trying again, but with the CompactPRO product ID.
        res = seek_init_device(
            &(*camera)->device, SEEK_VENDOR_ID, SEEK_PRO_PRODUCT_ID,
            /* options */ SEEK_READ_FW_VERSION | SEEK_READ_CHIP_ID
        );
        if (!res) return res;
    }

read_info:
    {
        seekerror_t res = device->get_factory_setting(device, SEEK_SERIAL_NO, (*camera)->serial_no, 12);
        if (res) return res;
    }

    (*camera)->lens_fov   = SEEK_FOV_NONE;
    (*camera)->lens_focus = SEEK_FOCUS_NONE;

    { // Reading lens FOV.
        unsigned char data[2];
        seekerror_t res = device->get_factory_setting(device, SEEK_LENS_FOV, data, 2);
        if (!res) {
            switch (data[0] | data[1] << 8) {
                case 0:     { (*camera)->lens_fov = SEEK_FOV_WIDE;   break; }
                case 1:     { (*camera)->lens_fov = SEEK_FOV_NARROW; break; }
                case 65535: { (*camera)->lens_fov = SEEK_FOV_WIDE;   break; }
            }
        }
    }
    { // Reading lens focus type.
        unsigned char data[2];
        seekerror_t res = device->get_factory_setting(device, SEEK_LENS_FOCUS, data, 2);
        if (!res) {
            switch (data[0] | data[1] << 8) {
                case 0:     { (*camera)->lens_focus = SEEK_FOCUS_FIXED;  break; }
                case 1:     { (*camera)->lens_focus = SEEK_FOCUS_MANUAL; break; }
                case 65535: { (*camera)->lens_focus = SEEK_FOCUS_FIXED;  break; }
            }
        }
    }

    return SEEK_ERROR_NONE;
}
