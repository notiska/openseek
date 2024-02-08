#ifndef _FEATURE_H
#define _FEATURE_H

/**
 * Various firmware info features.
 * References:
 *  - `com.subi.usb.CustomRequests$FirmwareInfoFeatureIndex` (Seek Thermal APK 1.9.1)
 *  - `com.tyriansystems.Seekware.o$i.a()` (Seek Thermal APK 2.3)
 */
typedef enum {
    SEEK_FIRMWARE_VERSION = 0,

    // FIXME: Can't actually get the debug log to work as of right now.
    SEEK_DEBUG_LOG_LENGTH = 13,
    SEEK_DEBUG_LOG        = 14,

    SEEK_HARDWARE_VERSION = 16,

    // TODO: 21 and 23 are also referenced in the APK, both of which being 64 bytes. Unfortunately, at least in the 2.3
    //       APK, these aren't actually used - so I can't determine what they are.
} seekfirmare_feature_t;

/**
 * Various factory device features.
 * References:
 *  - `com.tyriansystems.Seekware.SeekwarePhysicalDevice.readLensInfo()` (Seek Thermal APK 1.9.1)
 *  - `com.tyriansystems.Seekware.SeekwarePhysicalDevice.readSerialFromDevice()` (Seek Thermal APK 1.9.1)
 */
typedef enum {
    SEEK_SERIAL_NO  = 8,
    SEEK_LENS_FOV   = 1536,
    SEEK_LENS_FOCUS = 1537,
} seekfactory_feature_t;

#endif // _FEATURE_H
