/* sample_mixer_plugin.c
**
** Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include <errno.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sound/asound.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <tinyalsa/plugin.h>
#include <tinyalsa/asoundlib.h>

/* 2 words of uint32_t = 64 bits of mask */
#define PCM_MASK_SIZE (2)
#define PCM_FORMAT_BIT(x) (1ULL << x)

struct sample_pcm_priv {
    FILE *fptr;
    int session_id;
    int channels;
    int bitwidth;
    int sample_rate;
    unsigned int period_size;
    snd_pcm_uframes_t total_size_frames;
};

struct pcm_plugin_hw_constraints sample_pcm_constrs = {
    .access = 0,
    .format = 0,
    .bit_width = {
        .min = 16,
        .max = 32,
    },
    .channels = {
        .min = 1,
        .max = 8,
    },
    .rate = {
        .min = 8000,
        .max = 384000,
    },
    .periods = {
        .min = 1,
        .max = 8,
    },
    .period_bytes = {
        .min = 96,
        .max = 122880,
    },
};

static inline struct snd_interval *param_to_interval(struct snd_pcm_hw_params *p,
                                                  int n)
{
    return &(p->intervals[n - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL]);
}

static inline int param_is_interval(int p)
{
    return (p >= SNDRV_PCM_HW_PARAM_FIRST_INTERVAL) &&
        (p <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL);
}

static unsigned int param_get_int(struct snd_pcm_hw_params *p, int n)
{
    if (param_is_interval(n)) {
        struct snd_interval *i = param_to_interval(p, n);
        if (i->integer)
            return i->max;
    }
    return 0;
}

static inline struct snd_mask *param_to_mask(struct snd_pcm_hw_params *p, int n)
{
    return &(p->masks[n - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
}

static inline int param_is_mask(int p)
{
    return (p >= SNDRV_PCM_HW_PARAM_FIRST_MASK) &&
        (p <= SNDRV_PCM_HW_PARAM_LAST_MASK);
}

static inline int snd_mask_val(const struct snd_mask *mask)
{
    int i;
    for (i = 0; i < PCM_MASK_SIZE; i++) {
        if (mask->bits[i])
            return ffs(mask->bits[i]) + (i << 5) - 1;
    }
    return 0;
}

static int alsaformat_to_bitwidth(int format)
{
    switch (format) {
    case SNDRV_PCM_FORMAT_S32_LE:
    case SNDRV_PCM_FORMAT_S24_LE:
        return 32;
    case SNDRV_PCM_FORMAT_S8:
        return 8;
    case SNDRV_PCM_FORMAT_S24_3LE:
        return 24;
    default:
    case SNDRV_PCM_FORMAT_S16_LE:
        return 16;
    };
}

static int param_get_mask_val(struct snd_pcm_hw_params *p,
                                        int n)
{
    if (param_is_mask(n)) {
        struct snd_mask *m = param_to_mask(p, n);
        int val = snd_mask_val(m);

        return alsaformat_to_bitwidth(val);
    }
    return 0;
}

static int sample_session_open(int sess_id, unsigned int mode, struct sample_pcm_priv *priv)
{
     char fname[128];

     snprintf(fname, 128, "sample_pcm_data_%d.raw", sess_id);
     priv->fptr = fopen(fname,"rwb+");
     if (priv->fptr == NULL) {
         return -EIO;
     }
     rewind(priv->fptr);
     return 0;
}


static int sample_session_write(struct sample_pcm_priv *priv, void *buff, size_t count)
{
    uint8_t *data = (uint8_t *)buff;
    size_t len;

    len  = fwrite(buff, 1, count, priv->fptr);

    if (len != count)
        return -EIO;

    return 0;
}

static int sample_pcm_hw_params(struct pcm_plugin *plugin,
                             struct snd_pcm_hw_params *params)
{
    struct sample_pcm_priv *priv = plugin->priv;

    priv->sample_rate = param_get_int(params, SNDRV_PCM_HW_PARAM_RATE);
    priv->channels = param_get_int(params, SNDRV_PCM_HW_PARAM_CHANNELS);
    priv->bitwidth = param_get_mask_val(params, SNDRV_PCM_HW_PARAM_FORMAT);

    return 0;
}

static int sample_pcm_sw_params(struct pcm_plugin *plugin,
                             struct snd_pcm_sw_params *sparams)
{
    return 0;
}

static int sample_pcm_sync_ptr(struct pcm_plugin *plugin,
                            struct snd_pcm_sync_ptr *sync_ptr)
{
    return 0;
}

static int sample_pcm_writei_frames(struct pcm_plugin *plugin, struct snd_xferi *x)
{
    struct sample_pcm_priv *priv = plugin->priv;
    void *buff;
    size_t count;

    buff = x->buf;
    count = x->frames * (priv->channels * (priv->bitwidth) / 8);

    return sample_session_write(priv, buff, count);
}

static int sample_pcm_readi_frames(struct pcm_plugin *plugin, struct snd_xferi *x)
{
    return 0;
}

static int sample_pcm_ttstamp(struct pcm_plugin *plugin, int *tstamp)
{
    return 0;
}

static int sample_pcm_prepare(struct pcm_plugin *plugin)
{
    return 0;
}

static int sample_pcm_start(struct pcm_plugin *plugin)
{
    return 0;
}

static int sample_pcm_drop(struct pcm_plugin *plugin)
{
    return 0;
}

static int sample_pcm_close(struct pcm_plugin *plugin)
{
    struct sample_pcm_priv *priv = plugin->priv;
    int ret = 0;

    fclose(priv->fptr);
    free(plugin->priv);
    free(plugin);

    return ret;
}

static int sample_pcm_poll(struct pcm_plugin *plugin, struct pollfd *pfd,
        nfds_t nfds, int timeout)
{
    return 0;
}

static void* sample_pcm_mmap(struct pcm_plugin *plugin, void *addr, size_t length, int prot,
                               int flags, off_t offset)
{
    return MAP_FAILED;
}

static int sample_pcm_munmap(struct pcm_plugin *plugin, void *addr, size_t length)
{
    return 0;
}

int sample_pcm_open(struct pcm_plugin **plugin, unsigned int card,
                    unsigned int device, unsigned int mode)
{
    struct pcm_plugin *sample_pcm_plugin;
    struct sample_pcm_priv *priv;
    int ret = 0, session_id = device;

    sample_pcm_plugin = calloc(1, sizeof(struct pcm_plugin));
    if (!sample_pcm_plugin)
        return -ENOMEM;

    priv = calloc(1, sizeof(struct sample_pcm_priv));
    if (!priv) {
        ret = -ENOMEM;
        goto err_plugin_free;
    }

    sample_pcm_constrs.access = (PCM_FORMAT_BIT(SNDRV_PCM_ACCESS_RW_INTERLEAVED) |
                              PCM_FORMAT_BIT(SNDRV_PCM_ACCESS_RW_NONINTERLEAVED));
    sample_pcm_constrs.format = (PCM_FORMAT_BIT(SNDRV_PCM_FORMAT_S16_LE) |
                              PCM_FORMAT_BIT(SNDRV_PCM_FORMAT_S24_LE) |
                              PCM_FORMAT_BIT(SNDRV_PCM_FORMAT_S24_3LE) |
                              PCM_FORMAT_BIT(SNDRV_PCM_FORMAT_S32_LE));

    sample_pcm_plugin->card = card;
    sample_pcm_plugin->mode = mode;
    sample_pcm_plugin->constraints = &sample_pcm_constrs;
    sample_pcm_plugin->priv = priv;

    priv->session_id = session_id;

    ret = sample_session_open(session_id, mode, priv);
    if (ret) {
        errno = -ret;
        goto err_priv_free;
    }
    *plugin = sample_pcm_plugin;
    return 0;

err_priv_free:
    free(priv);
err_plugin_free:
    free(sample_pcm_plugin);
    return ret;
}

struct pcm_plugin_ops pcm_plugin_ops = {
    .open = sample_pcm_open,
    .close = sample_pcm_close,
    .hw_params = sample_pcm_hw_params,
    .sw_params = sample_pcm_sw_params,
    .sync_ptr = sample_pcm_sync_ptr,
    .writei_frames = sample_pcm_writei_frames,
    .readi_frames = sample_pcm_readi_frames,
    .ttstamp = sample_pcm_ttstamp,
    .prepare = sample_pcm_prepare,
    .start = sample_pcm_start,
    .drop = sample_pcm_drop,
    .mmap = sample_pcm_mmap,
    .munmap = sample_pcm_munmap,
    .poll = sample_pcm_poll,
};
