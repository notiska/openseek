#!/usr/bin/env python3

import os
import subprocess
import sys

BUILD_DIR   = "build"
INCLUDE_DIR = "include"
SRC_DIR     = "src"


if __name__ == "__main__":
    os.makedirs(BUILD_DIR, exist_ok=True)

    object_files = []

    for root, dirs, files in os.walk(SRC_DIR):
        for file in files:
            file, ext = os.path.splitext(os.path.join(root, file))
            if not ext in (".c",):
                continue

            object_file = os.path.join(BUILD_DIR, os.path.relpath(file, SRC_DIR) + ".o")

            os.makedirs(os.path.dirname(object_file), exist_ok=True)
            res = subprocess.call([
                "clang",
                "-I" + INCLUDE_DIR,
                "-o", object_file,
                "-c", file + ext,
            ])
            if res:
                print("Compilation failed for file %r." % (file + ext), file=sys.stderr)
                exit(1)

            object_files.append(object_file)

    res = subprocess.call([
        "clang",
        # "-static",
        "-lusb-1.0",
        *object_files,
        "-o", "test",
    ])
    if res:
        print("Linking failed.", file=sys.stderr)
        exit(1)
