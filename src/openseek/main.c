#include <stdio.h>
#include <stdlib.h>

#include "openseek/camera.h"

#define VENDOR_ID  0x289d
#define PRODUCT_ID 0x0011

int main(void) {
    if (libusb_init(NULL)) {
        fprintf(stderr, "Failed to initialise libusb.\n");
        return 1;
    }

    seekcamera_t *camera;

    seekerror_t res = seek_init_camera(&camera, NULL);
    if (res) {
        fprintf(stderr, "Failed to initialise Seek Thermal device: %i (%s).\n", res, seek_error_name(res));
        libusb_exit(NULL);
        return 1;
    }

    seekdevice_t *device = camera->device;

    device->set_platform(device, SEEK_TARGET_ANDROID);

    char *pretty_fw_version = device->pretty_fw_version(device);
    char *pretty_chip_id    = device->pretty_chip_id(device);
    char *pretty_opmode     = device->pretty_opmode(device);
    char *pretty_platform   = device->pretty_platform(device);

    char *pretty_type       = camera->pretty_type(camera);
    char *pretty_serial_no  = camera->pretty_serial_no(camera);
    char *pretty_lens_fov   = camera->pretty_lens_fov(camera);
    char *pretty_lens_focus = camera->pretty_lens_focus(camera);

    printf("Firmware version: %s\n", pretty_fw_version);
    printf("Chip ID:          %s\n", pretty_chip_id);
    printf("Operation mode:   %s\n", pretty_opmode);
    printf("Platform:         %s\n", pretty_platform);

    printf("\n");

    printf("Type:             %s\n", pretty_type);
    printf("Hardware:         v%i\n", camera->hw_version);
    printf("Serial no.:       %s\n", pretty_serial_no);
    printf("Lens FOV:         %s\n", pretty_lens_fov);
    printf("Lens focus:       %s\n", pretty_lens_focus);

    free(pretty_fw_version);
    free(pretty_chip_id);
    free(pretty_opmode);
    free(pretty_platform);

    free(pretty_type);
    free(pretty_serial_no);
    free(pretty_lens_fov);
    free(pretty_lens_focus);

    device->set_opmode(device, SEEK_RUNNING);
    // TODO: Stuff
    device->set_opmode(device, SEEK_SLEEPING);

    seek_deinit_camera(camera);
    libusb_exit(NULL);
}
