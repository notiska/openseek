#ifndef _ERROR_H
#define _ERROR_H

/**
 * Various errors raised in the library.
 * Some are partly based off real Seek errors.
 * References:
 *  - `https://github.com/seekthermal/seekcamera-python/blob/main/seekcamera/error.py`
 */
typedef enum {
    SEEK_ERROR_NONE        = 0,
    SEEK_ERROR_UNKNOWN     = 1,

    SEEK_ERROR_NO_DEVICE   = 2,
    SEEK_ERROR_DEVICE_BUSY = 3,
    SEEK_ERROR_TIMEOUT     = 4,
    SEEK_ERROR_BROKEN_PIPE = 5,

    SEEK_ERROR_NO_MEM      = 6,
} seekerror_t;

/**
 * Translates the given error code into a printable name.
 * 
 * @param error The error code.
 * @return The name of the error. It is your responsibility to free this.
 */
char *seek_error_name(seekerror_t error);

#endif // _ERROR_H
