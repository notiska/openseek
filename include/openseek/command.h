#ifndef _COMMAND_H
#define _COMMAND_H

/**
 * Seek shutter commands.
 * Found via trial and error.
 */
typedef enum {
    SEEK_SHUTTER_IMMEDIATE = 249,
    SEEK_SHUTTER_INVERTED  = 250,
    SEEK_SHUTTER_NORMAL    = 251,
    SEEK_SHUTTER_AUTO      = 252,
    SEEK_SHUTTER_NO_AUTO   = 253,
    // TODO: 254 and 255 return a weird error code - 0x8010300.
} seekshutter_command_t;

#endif // _COMMAND_H
