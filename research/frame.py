#!/usr/bin/env python3

"""
Barebones OpenCV based implementation for getting images off the Seek.
"""

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
    compact_pro = True

    if not compact_pro:
        x_res = 208
        y_res = 156
    else:
        x_res = 342
        y_res = 260
    device = None
    while device is None:
        device = core.find(idVendor=0x289d, idProduct=0x0011 if compact_pro else 0x0010)
        conf = device.get_active_configuration()

    endpoint = util.find_descriptor(
        conf[0,0],
        custom_match=lambda ep: util.endpoint_direction(ep.bEndpointAddress) == util.ENDPOINT_OUT,
    )

    # request_set(device, 0x54, b"\x01")      # TARGET_PLATFORM, ANDROID
    # request_set(device, 0x3c, b"\x00\x00")  # SET_OPERATION_MODE, SLEEPING

    firmware_ver = request_get(device, 0x4e, 0x04)  # GET_FIRMWARE_INFO
    chip_id = request_get(device, 0x36, 0x12)       # READ_CHIP_ID

    # com.tyriansystems.Seekware.SeekwarePhysicalDevice.convertByteToFirmwareString()
    print("Firmware version: %s" % ".".join(map(str, firmware_ver)))
    # com.tyriansystems.Seekware.SeekwarePhysicalDevice.readChipIDFromDev()
    print("Chip ID: 0x%s" % "".join("%02x" % (value) for value in struct.unpack(">6H", bytes(chip_id))))

    # SET_FIRMWARE_INFO_FEATURES, 16
    # GET_FIRMWARE_INFO (Compact - 2, CompactPRO - 20)

    request_set(device, 0x55, b"\x10\x00")  # SET_FIRMWARE_INFO_FEATURES, 16
    hw_version, = struct.unpack("<H", request_get(device, 0x4e, 0x02))  # GET_FIRMWARE_INFO

    print("HW: v%i" % hw_version)
    # HW: v0, TH: v3, TLID: -1

    # request_set(device, 0x56, b"\x06\x00\x08\x00\x00\x00")  # SET_FACTORY_SETTINGS_FEATURES
    # request_get(device, 0x58, 0x12)                         # GET_FACTORY_SETTINGS
    # request_set(device, 0x55, b"\x17\x00")                  # SET_FIRMWARE_INFO_FEATURES
    # request_get(device, 0x4e, 0x40)                         # GET_FIRMWARE_INFO

    request_set(device, 0x56, b"\x06\x00\x08\x00\x00\x00")           # SET_FACTORY_SETTINGS_FEATURES, 6, 8, 0
    serial = bytes(request_get(device, 0x58, 0x0c)).decode("ascii")  # GET_FACTORY_SETTINGS
    print("Serial no.: %r" % serial)

    request_set(device, 0x56, b"\x01\x00\x00\x06\x00\x00")       # SET_FACTORY_SETTINGS_FEATURES, 1, 1536, 0
    fov, = struct.unpack("<H", request_get(device, 0x58, 0x02))  # GET_FACTORY_SETTINGS
    request_set(device, 0x56, b"\x01\x00\x01\x06\x00\x00")         # SET_FACTORY_SETTINGS_FEATURES, 1, 1537, 0
    focus, = struct.unpack("<H", request_get(device, 0x58, 0x04))  # GET_FACTORY_SETTINGS

    # com.tyriansystems.Seekware.SeekwarePhysicalDevice.lensFromBytes()
    if fov in (0, 65535):
        print("FOV: wide")
    elif fov == 1:
        print("FOV: narrow")
    else:
        print("FOV: no lens")

    # com.tyriansystems.Seekware.SeekwarePhysicalDevice.focusFromBytes()
    if focus in (0, 65535):
        print("Focus: fixed")
    elif focus == 1:
        print("Focus: manual")
    else:
        print("Focus: no lens")

    # request_set(device, 0x56, b"\x08\x00\x02\x06\x00\x00")  # SET_FACTORY_SETTINGS_FEATURES
    print(request_get(device, 0x36, 0x0c))                  # SET_SHUTTER_POLARITY
    request_set(device, 0x56, b"\x00\x0a\x00\x00\x00\x00")  # SET_FACTORY_SETTINGS_FEATURES

    try:
        request_set(device, 0x52, b"\x2e\x00\x53\x16\x10\x31\x80\xdd\x00\xb7\x4a\xf9\xe4\x17\xc5\x94\xbe\xd4")  # BEGIN_MEMORY_WRITE
    except Exception:
        error, = struct.unpack(">I", request_get(device, 0x35, 0x04))  # GET_ERROR_CODE
        print(hex(error))

    # for index in range(0, 2560, 32):
    #     # TODO: if 5120 - index < 64 ???
    #     # SET_FACTORY_SETTINGS_FEATURES
    #     request_set(device, 0x56, b"\x20\x00%b\x00\x00" % struct.pack("<H", index))

    # print(request_get(device, 0x41, 3))  # GET_DATA_PAGE

    # request_set(device, 0x3e, b"\x08\x00")  # SET_IMAGE_PROCESSING_MODE, COLOUR_SOFT_KNEE
    # request_get(device, 0x4e, 0x40)         # GET_FIRMWARE_INFO

    # for index in range(65535):
    #     try:
    #         request_set(device, 0x55, index.to_bytes(2, "little"))  # SET_FIRMWARE_INFO_FEATURES
    #         print("0x%04x: %s" % (index, "".join(map("%02x".__mod__, request_get(device, 0x4e, 0x40)))))
    #     except core.USBTimeoutError:
    #         print("0x%04x: <invalid>" % index)
    #     except Exception:
    #         ...

    # print(*struct.unpack(">I", request_get(device, 0x35, 0x04)))  # GET_ERROR_CODE
    # request_set(device, 0x55, b"\x0d\x00")  # SET_FIRMWARE_INFO_FEATURES, DBG_LOG_LENGTH

    request_set(device, 0x3c, b"\x01\x00")  # SET_OPERATION_MODE, RUNNING

    cal_frame = None  # np.zeros((260, 342), dtype=np.uint16)
    save = False

    try:
        while True:
            request_set(device, 0x53, struct.pack("<I", x_res * y_res))  # START_GET_IMAGE_TRANSFER

            data = b""
            # for index in range(342 * 260 * 2 // 13680):
            while len(data) < x_res * y_res * 2:
                data += device.read(0x81, 13680 if compact_pro else 16224, 1000)

            frame_raw = np.frombuffer(data, dtype=np.uint16)
            # print(frame_raw[:20])

            if not compact_pro:
                type_   = frame_raw[10]
                counter = frame_raw[40]
                if frame_raw[0] == 30792 and frame_raw[1] == 37371:
                    type_ = 14
                # frame_raw[25] << 16 | frame_raw[40]
            else:
                type_   = frame_raw[2]
                counter = frame_raw[1]
                # frame_raw[3] << 16 | frame_raw[4]

            print(type_)

            if type_ == 1:
                cal_frame = frame_raw
            elif type_ == 3:
                frame = frame_raw + (0x4000 - cal_frame)
                # print(
                #     np.sum(frame_raw) / frame_raw.shape[0],
                #     np.sum(cal_frame) / cal_frame.shape[0],
                #     np.sum(frame) / frame.shape[0],
                # )
                frame = frame.reshape((y_res, x_res))

                if not compact_pro:
                    frame = frame[1:-1, 1:]
                else:
                    frame = frame[4:-2, :-18]

                # frame.byteswap(inplace=True)

                sort = np.sort(frame.ravel())
                # lower = sort[int(sort.shape[0] * 0.25)]
                # upper = sort[int(sort.shape[0] * 0.75)]

                lower = np.sum(sort[:sort.shape[0] // 2]) / (sort.shape[0] // 2)
                upper = np.sum(sort[sort.shape[0] // 2:]) / (sort.shape[0] // 2)

                # print(lower, upper, upper - lower)

                frame = (frame - lower) / (upper - lower) * 255
                frame[frame < 0] = 0
                frame[frame > 255] = 255

                if save:
                    cal_frame_bytes = cal_frame.tobytes()
                    frame_raw_bytes = frame_raw.tobytes()
                    with open("%i.bin" % counter, "wb") as stream:
                        stream.write(struct.pack(">II", len(cal_frame_bytes), len(frame_raw_bytes)))
                        stream.write(cal_frame_bytes)
                        stream.write(frame_raw_bytes)
                    save = False

                cv2.imshow("corrected", frame.astype(np.uint8))

            # cv2.imwrite("%i.png" % counter, frame.astype(np.uint8))
            # print(type_)
            cv2.imshow("raw", (frame_raw // 256).reshape((y_res, x_res)).astype(np.uint8))

            # save = cv2.waitKey(1) == ord(" ")
            key = cv2.waitKey(1)

            if key == ord("1"):
                request_set(device, 0x37, b"\xff\x00\xf9\x00")  # TOGGLE_SHUTTER, SHUTTER_IMMEDIATE
            elif key == ord("2"):
                request_set(device, 0x37, b"\xff\x00\xfa\x00")  # TOGGLE_SHUTTER, SHUTTER_INVERT
            elif key == ord("3"):
                request_set(device, 0x37, b"\xff\x00\xfb\x00")  # TOGGLE_SHUTTER, SHUTTER_NORMAL
            elif key == ord("4"):
                request_set(device, 0x37, b"\xff\x00\xfc\x00")  # TOGGLE_SHUTTER, SHUTTER_AUTO
            elif key == ord("5"):
                request_set(device, 0x37, b"\xff\x00\xfd\x00")  # TOGGLE_SHUTTER, SHUTTER_NOAUTO

            if key == ord(" "):
                # request_set(device, 0x37, b"\xff\x00\xf9\x00")  # TOGGLE_SHUTTER
                # b"\x00\x00\x00\x00" - error 0x10200 (all invalid first arg)
                # b"\xff\x00\x00\x00" - error 0x20200 (all invalid second arg)

                # b"\xff\x00\xf9\x00" - toggles shutter immediately
                # b"\xff\x00\xfa\x00" - inverts the shutter behaviour ?
                # b"\xff\x00\xfb\x00" - reverts b"\xff\x00\xfa\x00"
                # b"\xff\x00\xfc\x00" - enables the auto shutter
                # b"\xff\x00\xfd\x00" - stops the auto shutter
                # b"\xff\x00\xfe\x00" - error 0x8010300
                # b"\xff\x00\xff\x00" - error 0x8010300

                # b"\xfc\x00\x00\x00", b"\xfc\x00\x01\x00" - nothing ?

                # for index in range(65535):
                #     try:
                #         request_set(device, 0x37, b"\xff\x00" + index.to_bytes(2, "little"))  # TOGGLE_SHUTTER
                #         print(index)
                #     except Exception:
                #         error, = struct.unpack(">I", request_get(device, 0x35, 0x04))  # GET_ERROR_CODE
                #         if error != 0x20200:
                #             print(index, hex(error))
                #         ...
                #         # print(index, "%08x" % struct.unpack(">I", request_get(device, 0x35, 0x04)))  # GET_ERROR_CODE

                ...

    except KeyboardInterrupt:
        ...

    cv2.destroyAllWindows()

    request_set(device, 0x3c, b"\x00\x00")  # SET_OPERATION_MODE
