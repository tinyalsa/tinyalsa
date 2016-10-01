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
```

### Documentation

Once installed, the man pages are available via:

```
man tinyalsa-pcm
man tinyalsa-mixer
```

