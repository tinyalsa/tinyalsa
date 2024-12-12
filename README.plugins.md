# Plugin support

## Disclaimer

This description was not written by original authors of the plugin API. The
description may require some corrections.

## Overview

Tinyalsa provides support for client-side plugins to extend the functionality of
both PCM and mixer devices (or provide virtual ones). The plugins are loaded
from shared libraries, and execute within the process calling into tinyalsa.

### Code structure

- `src/`

  - `snd_card_plugin.c` - accessors for the main plugin .so file

  - `mixer_plugin.c` - implementations of mixer device plugin operations

  - `pcm_plugin.c` - implementations of the PCM device plugin operations

### Integration with tinyalsa

When compiled with plugin support, some tinyalsa APIs attempt to load a shared
library with a fixed `libsndcardparser.so` name, using APIs defined in
`snd_card_plugin.c`. The library must export an
`struct snd_node_ops snd_card_ops` global variable containing a set of
function pointers (see `include/tinyalsa/plugin.h`).

Those functions are used to expose a hierarchy of properties:

"cards" identified by numeric IDs, and for each card, optionally one of each
types of "nodes":

- PCM

- mixer

- "compress" (with somewhat unclear meaning as there is no example of that one).

PCM/mixer/"compress" nodes contain integer and string properties with string keys,
that can be queried with `snd_utils_get_{int,str}`.

`libsndcardparser.so` can be best understood as an interface to some sort of a
config file, and does not need to contain any actual audio logic. That is a job
of other shared libraries, referred to by `libsndcardparser` by the `so-name`
string property of a PCM/mixer/"compress" node.

The `snd_card_ops->open_card` operation is used to return an opaque `void*`
handle to an object representing a sound card. When not needed anymore, it's
released by a `snd_card_ops->close_card` call.

Depending on the particular tinyalsa operation being called, other function
pointers get invoked.

For an example `libsndcardparser.so`, see
`example/sndcardparser/sample_sndcardparser.c`.

## PCM plugins

Card properties exposed by `snd_utils_get_{int,str}` interpreted by tinyalsa
for PCM plugins:

- strings:

  - `so-name` - the name of a .so file that is the driver for a virtual PCM or
    mixer,

  - `name` - virtual PCM device name (unused for mixer devices),

- integers:

  - `type` - `enum snd_node_type` value, indicating if the sound card is a
    hardware or plugin PCM device. Hardware device entries are ignored by the
    plugin mechanism.

  - `playback` - must be nonzero if the virtual PCM device supports playback,

  - `capture` - must be nonzero if the virtual PCM device supports capture.

For a stub example PCM plugin .so, see `example/plugins/sample_pcm_plugin.c`.

### PCM plugin .so interface

PCM plugin shared libraries export an `pcm_plugin_ops` global symbol, a vtable
of type `struct pcm_plugin_ops`. The virtual PCM objects are initialized with
`pcm_plugin_ops->open` (and later released with `pcm_plugin_ops->close`), and
provide a set of functions used to bridge the virtual PCM to tinyalsa APIs.

## Mixer plugins

Card properties exposed by `snd_utils_get_{int,str}` that matter for PCM plugins:

- strings:

  - `so-name` - the name of a .so file that is the driver for a virtual PCM or
    mixer.

### Mixer plugin .so interface

Mixer plugin shared libraries export an `mixer_plugin_ops` global symbol - a
vtable of type `struct mixer_plugin_ops`, that are used to initialize/teardown
the virtual mixer (with `mixer_plugin_ops->{open,close}`), and also subscribing
to notifications about mixer events, and reading their value.

On initialization (`mixer_plugin_ops->open`), the mixer plugin exposes a fixed
set of available controls, represented by an array of `struct snd_control`. Each
entry contains somecontrol metadata, an opaque pointer to the control's value,
and a getter/setter pair of function pointers invoked whenever tinyalsa adjusts
the mixer control value.

For a stub example of a mixer plugin .so, see
`example/plugins/sample_mixer_plugin.c`.
