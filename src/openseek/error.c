#include <stdlib.h>
#include <string.h>

#include "openseek/error.h"

char *seek_error_name(seekerror_t error) {
    char *pretty = malloc(sizeof(char) * 100); // TODO: Adjust size.
    switch (error) {
        case SEEK_ERROR_NONE:        { strcpy(pretty, "SEEK_ERROR_NONE");        break; }
        default:
        case SEEK_ERROR_UNKNOWN:     { strcpy(pretty, "SEEK_ERROR_UNKNOWN");     break; }
        case SEEK_ERROR_NO_DEVICE:   { strcpy(pretty, "SEEK_ERROR_NO_DEVICE");   break; }
        case SEEK_ERROR_DEVICE_BUSY: { strcpy(pretty, "SEEK_ERROR_DEVICE_BUSY"); break; }
        case SEEK_ERROR_TIMEOUT:     { strcpy(pretty, "SEEK_ERROR_TIMEOUT");     break; }
        case SEEK_ERROR_BROKEN_PIPE: { strcpy(pretty, "SEEK_ERROR_BROKEN_PIPE"); break; }
        case SEEK_ERROR_NO_MEM:      { strcpy(pretty, "SEEK_ERROR_NO_MEM");      break; }
    }
    return pretty;
}
