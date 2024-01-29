#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openseek/device.h"

#define INTERFACE 0

#define INPUT  LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_IN
#define OUTPUT LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT

/* ---------------------------------------- Hidden ---------------------------------------- */

int _request_set(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len) {
    int res = libusb_control_transfer(device->handle, OUTPUT, command, 0, 0, data, data_len, device->timeout);
    if (res >= 0) return res;

    switch (res) {
        case LIBUSB_ERROR_TIMEOUT:   return -SEEK_ERROR_TIMEOUT;
        case LIBUSB_ERROR_PIPE:      return -SEEK_ERROR_PIPE;
        case LIBUSB_ERROR_NO_DEVICE: return -SEEK_ERROR_NO_DEVICE;
        case LIBUSB_ERROR_BUSY:      return -SEEK_ERROR_DEVICE_BUSY;
        default:                     return -SEEK_ERROR_UNKNOWN;
    }
}

int _request_get(seekdevice_t *device, _seekcommand_t command, unsigned char *data, int data_len) {
    int res = libusb_control_transfer(device->handle, INPUT, command, 0, 0, data, data_len, device->timeout);
    if (res >= 0) return res;

    switch (res) {
        case LIBUSB_ERROR_TIMEOUT:   return -SEEK_ERROR_TIMEOUT;
        case LIBUSB_ERROR_PIPE:      return -SEEK_ERROR_PIPE;
        case LIBUSB_ERROR_NO_DEVICE: return -SEEK_ERROR_NO_DEVICE;
        case LIBUSB_ERROR_BUSY:      return -SEEK_ERROR_DEVICE_BUSY;
        default:                     return -SEEK_ERROR_UNKNOWN;
    }
}

/* ---------------------------------------- Public ---------------------------------------- */

seekerror_t start_frame_transfer(seekdevice_t *device, int frame_size) {
    unsigned char data[4] = { (unsigned char)frame_size, (unsigned char)(frame_size >> 8) };
    return _request_get(device, _SEEK_START_GET_IMAGE_TRANSFER, data, 4);
}

seekerror_t get_firmware_info(seekdevice_t *device, seekfirmare_feature_t feature, unsigned char *data, int data_len) {
    unsigned char feature_data[2] = { (unsigned char)feature, (unsigned char)(feature >> 8) };

    int res = _request_set(device, _SEEK_SET_FIRMWARE_INFO_FEATURES, feature_data, 2);
    if (res < 0) return -res;

    res = _request_set(device, _SEEK_GET_FIRMWARE_INFO, data, data_len);
    if (res != data_len) {
        return res < 0 ? -res : SEEK_ERROR_UNKNOWN; // TODO: Underflow error.
    }
    return SEEK_ERROR_NONE;
}

seekerror_t get_factory_setting(seekdevice_t *device, seeksetting_factory_feature_t feature, unsigned char *data, int data_len) {
    int word_size = data_len / SEEK_WORD_SIZE;
    unsigned char feature_data[6] = { 
        (unsigned char)word_size, (unsigned char)(word_size >> 8),
        (unsigned char)feature, (unsigned char)(feature >> 8),
        0x00, 0x00
    };

    int res = _request_set(device, _SEEK_SET_FACTORY_SETTINGS_FEATURES, feature_data, 6);
    if (res < 0) return -res;

    res = _request_get(device, _SEEK_GET_FACTORY_SETTINGS, data, data_len);
    if (res != data_len) {
        return res < 0 ? -res : SEEK_ERROR_UNKNOWN; // TODO: Better error, such as "underflow"?
    }
    return SEEK_ERROR_NONE;
}

// seekerror_t reset(seekdevice_t *device) {
//     // _request_get(device, _SEEK_RESET_DEVICE,)
//     return SEEK_ERROR_NONE;
// }

/* ---------------------------------------- Setters / Getters ---------------------------------------- */

seekopmode_t opmode(seekdevice_t *device) {
    unsigned char data[2];
    int res = _request_get(device, _SEEK_GET_OPERATION_MODE, data, 2);
    if (res != 2) return SEEK_SLEEPING; // TODO: Defaults, perhaps some kind of "unknown"?
    return (seekopmode_t)(data[1] << 8 | data[0]);
}

seekerror_t set_opmode(seekdevice_t *device, seekopmode_t opmode) {
    unsigned char data[2] = { (unsigned char)(opmode & 0xff), (unsigned char)(opmode >> 8) };
    int res = _request_set(device, _SEEK_SET_OPERATION_MODE, data, 2);
    return res < 0 ? -res : SEEK_ERROR_NONE;
}

seekplatform_t platform(seekdevice_t *device) {
    return device->_platform;
}

seekerror_t set_platform(seekdevice_t *device, seekplatform_t platform) {
    if (platform == device->_platform) return SEEK_ERROR_NONE; // Nothing to do here.

    unsigned char data[1] = { (unsigned char)platform };
    int res = _request_set(device, _SEEK_TARGET_PLATFORM, data, 1);
    if (res < 0) return -res;

    device->_platform = platform;
    return SEEK_ERROR_NONE;
}

/* ---------------------------------------- Pretty ---------------------------------------- */

char *pretty_fw_version(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 16); // len("255.255.255.255") = 15
    sprintf(
        pretty, "%i.%i.%i.%i",
        device->fw_version[0], device->fw_version[1], device->fw_version[2], device->fw_version[3]
    );
    return pretty;
}

char *pretty_chip_id(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 25);
    sprintf(
        pretty, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
        device->chip_id[0], device->chip_id[1], device->chip_id[2], device->chip_id[3],
        device->chip_id[4], device->chip_id[5], device->chip_id[6], device->chip_id[7],
        device->chip_id[8], device->chip_id[9], device->chip_id[10], device->chip_id[11]
    );
    return pretty;
}

char *pretty_opmode(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 17); // len("Running (no AGC)") = 17
    switch (device->opmode(device)) {
        default:
        case SEEK_SLEEPING:       { strcpy(pretty, "Sleeping");         break; }
        case SEEK_RUNNING:        { strcpy(pretty, "Running");          break; }
        case SEEK_RUNNING_NO_AGC: { strcpy(pretty, "Running (no AGC)"); break; }
    }
    return pretty;
}

char *pretty_platform(seekdevice_t *device) {
    char *pretty = malloc(sizeof(char) * 8);
    switch (device->_platform) {
        default:
        case SEEK_UNKNOWN_TARGET: { strcpy(pretty, "Unknown"); break; }
        case SEEK_WINDOWS_TARGET: { strcpy(pretty, "Windows"); break; }
        case SEEK_ANDROID_TARGET: { strcpy(pretty, "Android"); break; }
        case SEEK_MACOS_TARGET:   { strcpy(pretty, "MacOS");   break; }
        case SEEK_IOS_TARGET:     { strcpy(pretty, "IOS");     break; }
    }
    return pretty;
}

/* ---------------------------------------- Init / De-init ---------------------------------------- */

seekerror_t seek_init_device(seekdevice_t **device, int vendor_id, int product_id, int options) {
    (*device) = (seekdevice_t*)calloc(1, sizeof(seekdevice_t));
    if (!(*device)) return SEEK_ERROR_NO_MEM;

    (*device)->handle = libusb_open_device_with_vid_pid(NULL, vendor_id, product_id);
    if (!(*device)->handle) return SEEK_ERROR_NO_DEVICE;

    (*device)->timeout = 1000;
    (*device)->_platform = SEEK_UNKNOWN_TARGET;

    // Beware: evil "method" trickery follows!!!

    (*device)->_request_set = &_request_set;
    (*device)->_request_get = &_request_get;

    (*device)->start_frame_transfer = &start_frame_transfer;
    (*device)->get_firmware_info    = &get_firmware_info;
    (*device)->get_factory_setting  = &get_factory_setting;
    // (*device)->reset = &reset;

    (*device)->opmode       = &opmode;
    (*device)->set_opmode   = &set_opmode;
    (*device)->platform     = &platform;
    (*device)->set_platform = &set_platform;

    (*device)->pretty_fw_version = &pretty_fw_version;
    (*device)->pretty_chip_id    = &pretty_chip_id;
    (*device)->pretty_opmode     = &pretty_opmode;
    (*device)->pretty_platform   = &pretty_platform;

#ifndef _WIN32
    {
        seekerror_t res;
        switch (libusb_attach_kernel_driver((*device)->handle, INTERFACE)) {
            case LIBUSB_SUCCESS:
            case LIBUSB_ERROR_NOT_SUPPORTED:
            // Not an issue, necessarily.
            case LIBUSB_ERROR_NOT_FOUND:     { res = SEEK_ERROR_NONE;        break; }

            default:
            case LIBUSB_ERROR_INVALID_PARAM: { res = SEEK_ERROR_UNKNOWN;     break; }                

            case LIBUSB_ERROR_NO_DEVICE:     { res = SEEK_ERROR_NO_DEVICE;   break; }
            case LIBUSB_ERROR_BUSY:          { res = SEEK_ERROR_DEVICE_BUSY; break; }
        }
        if (res) {
            libusb_close((*device)->handle);
            free(*device);
            return res;
        }
    }
#endif // _WIN32

    {
        seekerror_t res;
        switch (libusb_claim_interface((*device)->handle, INTERFACE)) {
            case LIBUSB_SUCCESS:         { res = SEEK_ERROR_NONE;        break; }
            default:
            case LIBUSB_ERROR_NOT_FOUND: { res = SEEK_ERROR_UNKNOWN;     break; }
            case LIBUSB_ERROR_BUSY:      { res = SEEK_ERROR_DEVICE_BUSY; break; }
            case LIBUSB_ERROR_NO_DEVICE: { res = SEEK_ERROR_NO_DEVICE;   break; }
        }
        if (res) {
#ifndef _WIN32
            libusb_attach_kernel_driver((*device)->handle, INTERFACE);
#endif // _WIN32
            libusb_close((*device)->handle);
            free(*device);
            return res;
        }
    }

    // set_platform(*device, SEEK_ANDROID_TARGET);

    // FIXME: Should we warn about not being able to read these?
    if (options & SEEK_READ_FW_VERSION) _request_get(*device, _SEEK_GET_FIRMWARE_INFO, (*device)->fw_version, 4);
    if (options & SEEK_READ_CHIP_ID)    _request_get(*device, _SEEK_READ_CHIP_ID,      (*device)->chip_id,    12);

    return SEEK_ERROR_NONE;
}

void seek_deinit_device(seekdevice_t *device) {
#ifndef _WIN32
    libusb_attach_kernel_driver(device->handle, INTERFACE);
#endif // _WIN32
    libusb_close(device->handle);
    free(device);
}
