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
        fprintf(stderr, "Failed to initialise Seek Thermal device: %i (%s).\n", res, seek_error_name(res));
        libusb_exit(NULL);
        return 1;
    }
    device->set_platform(device, SEEK_ANDROID_TARGET);

    char *pretty_fw_version = device->pretty_fw_version(device);
    char *pretty_chip_id    = device->pretty_chip_id(device);
    char *pretty_opmode     = device->pretty_opmode(device);
    char *pretty_platform   = device->pretty_platform(device);

    printf("Firmware version: %s\n", pretty_fw_version);
    printf("Chip ID:          %s\n", pretty_chip_id);
    printf("Operation mode:   %s\n", pretty_opmode);
    printf("Platform:         %s\n", pretty_platform);

    free(pretty_fw_version);
    free(pretty_chip_id);
    free(pretty_opmode);
    free(pretty_platform);

    device->set_opmode(device, SEEK_RUNNING);
    // TODO: Stuff
    device->set_opmode(device, SEEK_SLEEPING);

    seek_deinit_device(device);
    libusb_exit(NULL);
}
