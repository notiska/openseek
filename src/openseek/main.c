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

    printf("Firmware version: %i.%i.%i.%i\n", device->fw_version[0], device->fw_version[1], device->fw_version[2], device->fw_version[3]);
    printf(
        "Chip ID:          %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
        device->chip_id[0], device->chip_id[1], device->chip_id[2], device->chip_id[3],
        device->chip_id[4], device->chip_id[5], device->chip_id[6], device->chip_id[7],
        device->chip_id[8], device->chip_id[9], device->chip_id[10], device->chip_id[11]
    );

    printf("%p\n", device->_request_get);

    seek_deinit_device(device);
    libusb_exit(NULL);
}
