/* mixer_plugin.h
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


#ifndef __MIXER_PLUGIN_H__
#define __MIXER_PLUGIN_H__

/** Creates entry point function for the plugin */
#define MIXER_PLUGIN_OPEN_FN(name)                             \
    int name##_open(struct mixer_plugin **plugin,              \
                    unsigned int card)

/** Creates pointer to the entry point function of the plugin */
#define MIXER_PLUGIN_OPEN_FN_PTR()                              \
    int (*mixer_plugin_open_fn) (struct mixer_plugin **plugin,  \
                                 unsigned int card)
/** Forward declaration of the mixer plugin */
struct mixer_plugin;

typedef void (*event_callback)(struct mixer_plugin *);

struct mixer_plugin_ops {
    void (*close) (struct mixer_plugin **plugin);
    int (*subscribe_events) (struct mixer_plugin *plugin,
                             event_callback event_cb);
    ssize_t (*read_event) (struct mixer_plugin *plugin,
                           struct snd_ctl_event *ev, size_t size);
};

struct snd_control {
    snd_ctl_elem_iface_t iface;
    unsigned int access;
    const char *name;
    snd_ctl_elem_type_t type;
    void *value;
    int (*get) (struct mixer_plugin *plugin,
                struct snd_control *control,
                struct snd_ctl_elem_value *ev);
    int (*put) (struct mixer_plugin *plugin,
                struct snd_control *control,
                struct snd_ctl_elem_value *ev);
    uint32_t private_value;
    void *private_data;
};

struct mixer_plugin {
    unsigned int card;
    struct mixer_plugin_ops *ops;
    void *priv;

    int eventfd;
    int subscribed;
    int event_cnt;

    struct snd_control *controls;
    unsigned int num_controls;
};

struct snd_value_enum {
    unsigned int items;
    char **texts;
};

struct snd_value_bytes {
    unsigned int size;
};

struct snd_value_tlv_bytes {
    unsigned int size;
    int (*get) (struct mixer_plugin *plugin,
                struct snd_control *control,
                struct snd_ctl_tlv *tlv);
    int (*put) (struct mixer_plugin *plugin,
                struct snd_control *control,
                struct snd_ctl_tlv *tlv);
};

struct snd_value_int {
    unsigned int count;
    int min;
    int max;
    int step;
};

/* static initializers */

#define SND_VALUE_ENUM(etexts, eitems)    \
    {.texts = etexts, .items = eitems}

#define SND_VALUE_BYTES(csize)    \
    {.size = csize }

#define SND_VALUE_INTEGER(icount, imin, imax, istep) \
    {.count = icount, .min = imin, .max = imax, .step = istep }

#define SND_VALUE_TLV_BYTES(csize, cget, cput)       \
    {.size = csize, .get = cget, .put = cput }

#define SND_CONTROL_ENUM(cname, cget, cput, cenum, priv_val, priv_data)   \
    {    .iface = SNDRV_CTL_ELEM_IFACE_MIXER,                             \
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,                        \
        .type = SNDRV_CTL_ELEM_TYPE_ENUMERATED,                           \
        .name = cname, .value = &cenum, .get = cget, .put = cput,         \
        .private_value = priv_val, .private_data = priv_data,             \
    }

#define SND_CONTROL_BYTES(cname, cget, cput, cbytes, priv_val, priv_data) \
    {                                                                     \
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,                              \
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,                        \
        .type = SNDRV_CTL_ELEM_TYPE_BYTES,                                \
        .name = cname, .value = &cbytes, .get = cget, .put = cput,        \
        .private_value = priv_val, .private_data = priv_data,             \
    }

#define SND_CONTROL_INTEGER(cname, cget, cput, cint, priv_val, priv_data) \
    {                                                                        \
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,                                 \
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,                           \
        .type = SNDRV_CTL_ELEM_TYPE_INTEGER,                                 \
        .name = cname, .value = &cint, .get = cget, .put = cput,             \
        .private_value = priv_val, .private_data = priv_data,                \
    }

#define SND_CONTROL_TLV_BYTES(cname, cbytes, priv_val, priv_data)  \
    {                                                                        \
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,                                 \
        .access = SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE,                       \
        .type = SNDRV_CTL_ELEM_TYPE_BYTES,                                   \
        .name = cname, .value = &cbytes,                                     \
        .private_value = priv_val, .private_data = priv_data,                \
    }

/* pointer based initializers */
#define INIT_SND_CONTROL_INTEGER(c, cname, cget, cput, cint, pval, pdata)   \
    {                                                                       \
        c->iface = SNDRV_CTL_ELEM_IFACE_MIXER;                              \
        c->access = SNDRV_CTL_ELEM_ACCESS_READWRITE;                        \
        c->type = SNDRV_CTL_ELEM_TYPE_INTEGER;                              \
        c->name = cname; c->value = &cint; c->get = cget; c->put = cput;    \
        c->private_value = pval; c->private_data = pdata;                   \
    }

#define INIT_SND_CONTROL_BYTES(c, cname, cget, cput, cint, pval, pdata)     \
    {                                                                       \
        c->iface = SNDRV_CTL_ELEM_IFACE_MIXER;                              \
        c->access = SNDRV_CTL_ELEM_ACCESS_READWRITE;                        \
        c->type = SNDRV_CTL_ELEM_TYPE_BYTES;                                \
        c->name = cname; c->value = &cint; c->get = cget; c->put = cput;    \
        c->private_value = pval; c->private_data = pdata;                   \
    }

#define INIT_SND_CONTROL_ENUM(c, cname, cget, cput, cenum, pval, pdata)     \
    {                                                                       \
        c->iface = SNDRV_CTL_ELEM_IFACE_MIXER;                              \
        c->access = SNDRV_CTL_ELEM_ACCESS_READWRITE;                        \
        c->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;                           \
        c->name = cname; c->value = cenum; c->get = cget; c->put = cput;    \
        c->private_value = pval; c->private_data = pdata;                   \
    }
#define INIT_SND_CONTROL_TLV_BYTES(c, cname, cbytes, priv_val, priv_data)  \
    {                                                                      \
        c->iface = SNDRV_CTL_ELEM_IFACE_MIXER;                             \
        c->access = SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE;                   \
        c->type = SNDRV_CTL_ELEM_TYPE_BYTES;                               \
        c->name = cname; c->value = &cbytes;                               \
        c->private_value = priv_val; c->private_data = priv_data;          \
    }
#endif /* end of __MIXER_PLUGIN_H__ */
