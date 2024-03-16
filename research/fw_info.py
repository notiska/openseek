#!/usr/bin/env python3

import struct

import cv2
import numpy as np
from usb import core, util


_INPUT  = util.ENDPOINT_IN  | util.CTRL_TYPE_VENDOR | util.CTRL_RECIPIENT_INTERFACE
_OUTPUT = util.ENDPOINT_OUT | util.CTRL_TYPE_VENDOR | util.CTRL_RECIPIENT_INTERFACE


def request_set(device: core.Device, command: int, data: bytes) -> bytes:
    return device.ctrl_transfer(_OUTPUT, command, 0, 0, data)


def request_get(device: core.Device, command: int, data: int) -> bytes:
    return device.ctrl_transfer(_INPUT, command, 0, 0, data)


if __name__ == "__main__":
    device = None
    while device is None:
        device = core.find(idVendor=0x289d, idProduct=0x0010)
        conf = device.get_active_configuration()

    endpoint = util.find_descriptor(
        conf[0,0],
        custom_match=lambda ep: util.endpoint_direction(ep.bEndpointAddress) == util.ENDPOINT_OUT,
    )

    for index in range(65535):
        try:
            request_set(device, 0x55, struct.pack("<H", index))  # SET_FIRMWARE_INFO_FEATURES
            print(index, ":", bytes(request_get(device, 0x4e, 0x40)))        # GET_FIRMWARE_INFO
        except Exception:
            ...
