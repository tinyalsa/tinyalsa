/* pcm_plugin.h
** Copyright (c) 2019, The Linux Foundation.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above
**     copyright notice, this list of conditions and the following
**     disclaimer in the documentation and/or other materials provided
**     with the distribution.
**   * Neither the name of The Linux Foundation nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
** ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
** BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
** BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
** OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
** IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef __PCM_PLUGIN_H__
#define __PCM_PLUGIN_H__

/** Macro to create entry point function for the plugin.
 * @ingroup libtinyalsa-pcm
 */
#define PCM_PLUGIN_OPEN_FN(name)                    \
    int name##_open(struct pcm_plugin **plugin,     \
                    unsigned int card,              \
                    unsigned int device,            \
                    int mode)
/** Macro to create function pointer to the plugin's
 * entry point function.
 * @ingroup libtinyalsa-pcm
 */
#define PCM_PLUGIN_OPEN_FN_PTR()                        \
    int (*plugin_open_fn) (struct pcm_plugin **plugin,  \
                           unsigned int card,           \
                           unsigned int device,         \
                           int mode);
/** Forward declaration of the structure to hold pcm
 * plugin related data/information.
 */
struct pcm_plugin;

/** Operations that are required to be registered by the plugin.
 * @ingroup libtinyalsa-pcm
 */
struct pcm_plugin_ops {
    /** close the pcm plugin */
    int (*close) (struct pcm_plugin *plugin);
    /** Set the PCM hardware parameters to the plugin */
    int (*hw_params) (struct pcm_plugin *plugin,
                      struct snd_pcm_hw_params *params);
    /** Set the PCM software parameters to the plugin */
    int (*sw_params) (struct pcm_plugin *plugin,
                      struct snd_pcm_sw_params *params);
    /** Synchronize the pointer */
    int (*sync_ptr) (struct pcm_plugin *plugin,
                     struct snd_pcm_sync_ptr *sync_ptr);
    /** Write frames to plugin to be rendered to output */
    int (*writei_frames) (struct pcm_plugin *plugin,
                          struct snd_xferi *x);
    /** Read frames from plugin captured from input */
    int (*readi_frames) (struct pcm_plugin *plugin,
                         struct snd_xferi *x);
    /** Obtain the timestamp for the PCM */
    int (*ttstamp) (struct pcm_plugin *plugin,
                    int *tstamp);
    /** Prepare the plugin for data transfer */
    int (*prepare) (struct pcm_plugin *plugin);
    /** Start data transfer from/to the plugin */
    int (*start) (struct pcm_plugin *plugin);
    /** Drop pcm frames */
    int (*drop) (struct pcm_plugin *plugin);
    /** Any custom or alsa specific ioctl implementation */
    int (*ioctl) (struct pcm_plugin *plugin,
                  int cmd, void *arg);
};

/** Minimum and maximum values for hardware parameter constraints.
 * @ingroup libtinyalsa-pcm
 */
struct pcm_plugin_min_max {
    /** Minimum value for the hardware parameter */
    unsigned int min;
    /** Maximum value for the hardware parameter */
    unsigned int max;
};

/** Encapsulate the hardware parameter constraints
 * @ingroup libtinyalsa-pcm
 */
struct pcm_plugin_hw_constraints {
    /** Value for SNDRV_PCM_HW_PARAM_ACCESS param */
    uint64_t access;
    /** Value for SNDRV_PCM_HW_PARAM_FORMAT param.
     * As of this implementation ALSA supports 52 formats */
    uint64_t format;
    /** Value for SNDRV_PCM_HW_PARAM_SAMPLE_BITS param */
    struct pcm_plugin_min_max bit_width;
    /** Value for SNDRV_PCM_HW_PARAM_CHANNELS param */
    struct pcm_plugin_min_max channels;
    /** Value for SNDRV_PCM_HW_PARAM_RATE param */
    struct pcm_plugin_min_max rate;
    /** Value for SNDRV_PCM_HW_PARAM_PERIODS param */
    struct pcm_plugin_min_max periods;
    /** Value for SNDRV_PCM_HW_PARAM_PERIOD_BYTES param */
    struct pcm_plugin_min_max period_bytes;
};

struct pcm_plugin {
    /** Card number for the pcm device */
    unsigned int card;
    /** pointer to plugin operation */
    struct pcm_plugin_ops *ops;
    /** pointer to the contraints registered by the plugin */
    struct pcm_plugin_hw_constraints *constraints;
    /** pointer to pcm node under snd card definition */
    void *node;
    /** Indicates read/write mode, etc.. */
    int mode;
    /* Pointer to hold the plugin's private data */
    void *priv;
    /* Tracks the plugin state */
    unsigned int state;
};

#endif /* end of __PCM_PLUGIN_H__ */
