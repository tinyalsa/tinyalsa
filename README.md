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

