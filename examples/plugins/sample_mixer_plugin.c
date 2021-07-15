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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sound/asound.h>
#include <tinyalsa/plugin.h>
#include <tinyalsa/asoundlib.h>

#define SAMPLE_MIXER_PRIV_GET_CTL_PTR(p, idx) (p->ctls + idx)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const char *const sample_enum_text[] = {"One", "Two", "Three"};

struct sample_mixer_priv {
    struct snd_control *ctls;
    int ctl_count;

    struct snd_value_enum sample_enum;

    mixer_event_callback event_cb;
};

static int sample_mixer_int_ctl_get(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
    return 0;
}

static int sample_mixer_int_ctl_put(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
/*
 *   Integer values can be retrieved using:
 *   uint32_t val1 = (uint32_t)ev->value.integer.value[0];
 *   uint32_t val2 = (uint32_t)ev->value.integer.value[1];
 *   uint32_t val3 = (uint32_t)ev->value.integer.value[2];
 */
    return 0;
}

static int sample_mixer_byte_array_ctl_get(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
    return 0;
}

static int sample_mixer_byte_array_ctl_put(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
/*
 *   Byte array payload can be retrieved using:
 *   void *payload = ev->value.bytes.data;
 */

    return 0;
}

static int sample_mixer_tlv_ctl_get(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_tlv *ev)
{
    return 0;
}

static int sample_mixer_tlv_ctl_put(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_tlv *tlv)
{
/*
 *   TLV payload and len can be retrieved using:
 *   void *payload = &tlv->tlv[0];
 *   size_t tlv_size = tlv->length;
 */

    return 0;
}

static int sample_mixer_enum_ctl_get(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
    return 0;
}

static int sample_mixer_enum_ctl_put(struct mixer_plugin *plugin,
                struct snd_control *ctl, struct snd_ctl_elem_value *ev)
{
/*
 *    Enum value can be retrieved using:
 *    unsigned int val = ev->value.enumerated.item[0];
 */
    return 0;
}

static struct snd_value_int sample_mixer_ctl_value_int =
    SND_VALUE_INTEGER(3, 0, 1000, 100);

/* 512 max bytes for non-tlv byte controls */
static struct snd_value_bytes byte_array_ctl_bytes =
    SND_VALUE_BYTES(512);

static struct snd_value_tlv_bytes sample_mixer_tlv_ctl_bytes =
    SND_VALUE_TLV_BYTES(1024, sample_mixer_tlv_ctl_get, sample_mixer_tlv_ctl_put);

static void create_integer_ctl(struct sample_mixer_priv *priv,
                int ctl_idx, int pval, void *pdata)
{
    struct snd_control *ctl = SAMPLE_MIXER_PRIV_GET_CTL_PTR(priv, ctl_idx);
    char *ctl_name = strdup("Sample integer control");

    /* pval and pdata can be retrieved using snd_control during get()/put() */
    INIT_SND_CONTROL_INTEGER(ctl, ctl_name, sample_mixer_int_ctl_get,
                    sample_mixer_int_ctl_put, sample_mixer_ctl_value_int, pval, pdata);
}

static void create_byte_array_ctl(struct sample_mixer_priv *priv,
    int ctl_idx, int pval, void *pdata)
{
    struct snd_control *ctl = SAMPLE_MIXER_PRIV_GET_CTL_PTR(priv, ctl_idx);
    char *ctl_name = strdup("Sample byte array control");

    INIT_SND_CONTROL_BYTES(ctl, ctl_name, sample_mixer_byte_array_ctl_get,
            sample_mixer_byte_array_ctl_put, byte_array_ctl_bytes,
            pval, pdata);
}

static void create_tlv_ctl(struct sample_mixer_priv *priv,
                int ctl_idx, int pval, void *pdata)
{
    struct snd_control *ctl = SAMPLE_MIXER_PRIV_GET_CTL_PTR(priv, ctl_idx);
    char *ctl_name = strdup("Sample tlv control");

    INIT_SND_CONTROL_TLV_BYTES(ctl, ctl_name, sample_mixer_tlv_ctl_bytes,
                    pval, pdata);
}

static void create_enum_ctl(struct sample_mixer_priv *priv,
            int ctl_idx, struct snd_value_enum *e,
            int pval, void *pdata)
{
    struct snd_control *ctl = SAMPLE_MIXER_PRIV_GET_CTL_PTR(priv, ctl_idx);
    char *ctl_name = strdup("Sample enum control");

    INIT_SND_CONTROL_ENUM(ctl, ctl_name, sample_mixer_enum_ctl_get,
                    sample_mixer_enum_ctl_put, e, pval, pdata);
}

static int sample_mixer_form_ctls(struct sample_mixer_priv *priv, int ctl_idx)
{
    create_integer_ctl(priv, ctl_idx, 0, NULL);
    ctl_idx++;
    create_byte_array_ctl(priv, ctl_idx, 0, NULL);
    ctl_idx++;
    create_tlv_ctl(priv, ctl_idx, 0, NULL);
    ctl_idx++;
    create_enum_ctl(priv, ctl_idx, &priv->sample_enum, 0, NULL);
    ctl_idx++;

    return 0;
}

static ssize_t sample_mixer_read_event(struct mixer_plugin *plugin,
                              struct snd_ctl_event *ev, size_t size)
{
    /* Fill snd_ctl_event *ev before sending.
     * Return : sizeof(struct snd_ctl_event),
     *          0 in case no event present.
     */

    return 0;
}

static int sample_mixer_subscribe_events(struct mixer_plugin *plugin,
                                  mixer_event_callback event_cb)
{
    struct sample_mixer_priv *priv = plugin->priv;

    priv->event_cb = event_cb;
   /* event_cb is the callback function which needs to be called
    * when an event occurs. This will unblock poll() on mixer fd
    * which is called from mixer_wait_event().
    * Once poll is unblocked, clients can call mixer_read_event()
    * During unsubscribe(), event_cb is NULL.
    */
    return 0;
}

static int sample_mixer_alloc_ctls(struct sample_mixer_priv *priv)
{
    int ret = 0, i;

    priv->ctls = calloc(priv->ctl_count, sizeof(*priv->ctls));
    if (!priv->ctls) {
        return -ENOMEM;
    }

    priv->sample_enum.items = ARRAY_SIZE(sample_enum_text);
    priv->sample_enum.texts = calloc(priv->sample_enum.items, sizeof(*priv->sample_enum.texts));

    for (i = 0; i < ARRAY_SIZE(sample_enum_text); i++)
        priv->sample_enum.texts[i] = strdup(sample_enum_text[i]);

    return sample_mixer_form_ctls(priv, 0);
}

static void sample_mixer_free_ctls(struct sample_mixer_priv *priv)
{
    int num_enums, i;
    struct snd_control *ctl = NULL;

    for (i = 0; i < priv->ctl_count; i++) {
        ctl = SAMPLE_MIXER_PRIV_GET_CTL_PTR(priv, i);
        if (ctl->name)
            free((void *)ctl->name);
    }

    num_enums = priv->sample_enum.items;

    for (i = 0; i < num_enums; i++)
        free(priv->sample_enum.texts[i]);

    free(priv->sample_enum.texts);
    priv->ctl_count = 0;

    if (priv->ctls) {
        free(priv->ctls);
        priv->ctls = NULL;
    }
}

static void sample_mixer_close(struct mixer_plugin **plugin)
{
    struct mixer_plugin *mp = *plugin;
    struct sample_mixer_priv *priv = mp->priv;

    /* unblock mixer event during close */
    if (priv->event_cb)
        priv->event_cb(mp);
    sample_mixer_subscribe_events(mp, NULL);
    sample_mixer_free_ctls(priv);
    free(priv);
    free(*plugin);
    *plugin = NULL;
}

int sample_mixer_open(struct mixer_plugin **plugin, unsigned int card)
{
    struct mixer_plugin *mp;
    struct sample_mixer_priv *priv;
    int i, ret = 0;
    int ctl_cnt = 4;

    mp = calloc(1, sizeof(*mp));
    if (!mp) {
        return -ENOMEM;
    }

    priv = calloc(1, sizeof(*priv));
    if (!priv) {
        ret = -ENOMEM;
        goto err_priv_alloc;
    }

    priv->ctl_count = ctl_cnt;
    ret = sample_mixer_alloc_ctls(priv);
    if (ret)
        goto err_ctls_alloc;

    /* Register the controls */
    mp->controls = priv->ctls;
    mp->num_controls = priv->ctl_count;
    mp->priv = priv;
    *plugin = mp;

    return 0;

err_ctls_alloc:
    sample_mixer_free_ctls(priv);
    free(priv);

err_priv_alloc:
    free(mp);
    return ret;
}

struct mixer_plugin_ops mixer_plugin_ops = {
    .open = sample_mixer_open,
    .close = sample_mixer_close,
    .subscribe_events = sample_mixer_subscribe_events,
    .read_event = sample_mixer_read_event,
};
