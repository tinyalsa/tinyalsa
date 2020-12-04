TinyALSA
========

[![Build Status](https://travis-ci.org/tinyalsa/tinyalsa.svg?branch=master)](https://travis-ci.org/tinyalsa/tinyalsa)

TinyALSA is a small library to interface with ALSA in the Linux kernel.

The aims are:

 - Provide a basic pcm and mixer API.
 - If it's not absolutely needed, don't add it to the API.
 - Avoid supporting complex and unnecessary operations, that could be
   dealt with at a higher level.
 - Provide comprehensive documentation.

### Building

TinyALSA supports these build systems:

 - [CMake](https://en.wikipedia.org/wiki/CMake)
 - [Make](https://en.wikipedia.org/wiki/Make_(software))
 - [Meson](https://en.wikipedia.org/wiki/Meson_(software))
 - [Soong](https://android.googlesource.com/platform/build/soong/+/refs/heads/master/README.md) for Android

To build and install with Make, run the commands:

```
make
sudo make install
sudo ldconfig
```

### Installing

TinyALSA is now available as a set of the following [Debian](https://en.wikipedia.org/wiki/Debian)
packages from [launchpad](https://launchpad.net/~taylorcholberton/+archive/ubuntu/tinyalsa):

| Package Name:   | Description:                                        |
|-----------------|-----------------------------------------------------|
| tinyalsa        | Contains tinyplay, tinycap, tinymix and tinypcminfo |
| libtinyalsa     | Contains the shared library                         |
| libtinyalsa-dev | Contains the static library and header files        |

To install these packages, run the commands:

```
sudo apt-add-repository ppa:taylorcholberton/tinyalsa
sudo apt-get update
sudo apt-get install tinyalsa
sudo apt-get install libtinyalsa-dev
```

### Documentation

Once installed, the man pages are available via:

```
man tinyplay
man tinycap
man tinymix
man tinypcminfo
man libtinyalsa-pcm
man libtinyalsa-mixer
```

### Test

To test libtinyalsa, please follow the instructions,

#### Setup Bazel build environment

Visit [here](https://docs.bazel.build/versions/3.7.0/install.html) to get more info to setup Bazel environment.

#### Insert loopback devices

The test program does pcm_* operations on loopback devices. You have to insert loopback devices after your system boots up.

```
sudo modprobe snd-aloop
sudo chmod 777 /dev/snd/*
```

#### Run test program

```
bazel test //:tinyalsa_tests --test_output=all
```

The default playback device is hw:2,0 and the default capture device is hw:2,1. If your loopback devices are not hw:2,0 and hw:2,1, you can specify the loopback device.

```
bazel test //:tinyalsa_tests --test_output=all \
    --copt=-DTEST_LOOPBACK_CARD=[loopback card] \
    --copt=-DTEST_LOOPBACK_PLAYBACK_DEVICE=[loopback playback device] \
    --copt=-DTEST_LOOPBACK_CAPTURE_DEVICE=[loopback capture device]
```

#### Generate coverage report

```
bazel coverage //:tinyalsa_tests --combined_report=lcov --test_output=all
genhtml bazel-out/_coverage/_coverage_report.dat -o tinyalsa_tests_coverage
```
