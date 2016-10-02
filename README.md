TinyALSA
========

TinyALSA is a small library to interface with ALSA in the Linux kernel.

The aims are:

 - Provide a basic pcm and mixer API.
 - If it's not absolutely needed, don't add it to the API.
 - Avoid supporting complex and unnecessary operations, that could be
   dealt with at a higher level.
 - Provide comprehensive documentation.

### Building

TinyALSA uses Makefile as the primary build system.

To build and install with Make, run the commands:

```
make
sudo make install
sudo ldconfig
```

### Installing

TinyALSA is now available as a set of the following debian packages from [launchpad](https://launchpad.net/~taylorcholberton/+archive/ubuntu/tinyalsa):

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
man tinyalsa-pcm
man tinyalsa-mixer
```

