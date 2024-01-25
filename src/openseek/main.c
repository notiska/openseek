#include <stdio.h>
#include <stdlib.h>

#include "openseek/device.h"

#define VENDOR_ID  0x289d
#define PRODUCT_ID 0x0011

int main(void) {
    if (libusb_init(NULL)) {
        fprintf(stderr, "Failed to initialise libusb.\n");
        return 1;
    }

    seekdevice_t *device;

    int res = seek_init_device(&device, VENDOR_ID, PRODUCT_ID, SEEK_READ_FW_VERSION | SEEK_READ_CHIP_ID);
    if (res) {
        fprintf(stderr, "Failed to initialise Seek Thermal device: %i (%s).\n", res, libusb_error_name(res));
        libusb_exit(NULL);
        return 1;
    }
    device->set_platform(device, SEEK_ANDROID_TARGET);

    printf("Firmware version: %s\n", device->pretty_fw_version(device));
    printf("Chip ID:          %s\n", device->pretty_chip_id(device));
    printf("Platform:         %s\n", device->pretty_platform(device));

    seek_deinit_device(device);
    libusb_exit(NULL);
}
