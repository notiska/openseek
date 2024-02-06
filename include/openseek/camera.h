#ifndef _CAMERA_H
#define _CAMERA_H

#include "openseek/device.h"
#include "openseek/error.h"

/**
 * The field of view of the device / the lens on the device. (It's really per-device though.)
 * References:
 *  - `com.tyriansystems.Seekware.SeekwareDevice$LensType` (Seek Thermal APK 1.9.1)
 *  - `com.tyriansystems.Seekware.SeekwarePhysicalDevice.lensFromBytes()` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    SEEK_FOV_WIDE   = 0,
    SEEK_FOV_NARROW = 1,
    SEEK_FOV_NONE   = 2, // AKA, no lens
} seeklens_fov_t;

/**
 * The focus type of the device / the lens on the device.
 * References:
 *  - `com.tyriansystems.Seekware.SeekwareDevice$FocusMode` (Seek Thermal APK 1.9.1)
 *  - `com.tyriansystems.Seekware.SeekwarePhysicalDevice.focusFromBytes()` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    SEEK_FOCUS_FIXED  = 0,
    SEEK_FOCUS_MANUAL = 1,
    SEEK_FOCUS_NONE   = 2,
} seeklens_focus_t;

/**
 * The type of Seek device.
 * References:
 *  - `com.tyriansystems.Seekware.c$b` (Seek Thermal APK 2.3)
 */
typedef enum {
    SEEK_COMPACT           = 0,
    // SEEK_COMPACT_XR        = 1,
    // SEEK_COMPACT_FF        = 2,
    // SEEK_COMPACT_XR_FF     = 3,
    SEEK_COMPACT_PRO       = 4,
    // SEEK_COMPACT_PRO_XR    = 5,
    // SEEK_COMPACT_PRO_FF    = 6,
    // SEEK_COMPACT_PRO_XR_FF = 7,
} seekcameratype_t;

typedef struct _SeekCamera seekcamera_t;
struct _SeekCamera {
    seekdevice_t     *device;
    seekcameratype_t  type;

    int               hw_version;
    unsigned char     serial_no[12];

    seeklens_fov_t    lens_fov;
    seeklens_focus_t  lens_focus;
};

/**
 * Initialises a Seek camera.
 * 
 * @param camera The camera to initialise.
 * @param device The device to initialise the camera with, if {@code NULL}, it will attempt to automatically find one.
 * @return 0 if successful, otherwise an error code.
 */
// TODO: Custom errors.
seekerror_t seek_init_camera(seekcamera_t **camera, seekdevice_t *device);

void seek_deinit_camera(seekcamera_t *camera);

#endif // _CAMERA_H
