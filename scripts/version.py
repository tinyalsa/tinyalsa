#!/usr/bin/env python3
#
# tinyalsa version.py: Extracts versions from TINYALSA_VERSION_STRING in
# include/tinyalsa/version.h header file for Meson build.
import os
import sys

if __name__ == '__main__':
    try:
        srcroot = os.environ['MESON_SOURCE_ROOT']
    except:
        srcroot = os.getcwd()
        print('Warning: MESON_SOURCE_ROOT env var not set, assuming source code is in', srcroot, file=sys.stderr)

    # API version
    api_version = None
    f = open(os.path.join(srcroot, 'include', 'tinyalsa', 'version.h'), 'r')
    for line in f:
        if line.startswith('#define TINYALSA_VERSION_STRING '):
            api_version = line[32:].strip().replace('"', '')
            print(api_version)
            sys.exit(0)

    print('Warning: Could not extract API version from TINYALSA_VERSION_STRING in include/tinyalsa/version.h in', srcroot, file=sys.stderr)
    sys.exit(-1)
