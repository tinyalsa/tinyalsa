/* snd_utils.c
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "snd_utils.h"

#define SND_DLSYM(h, p, s, err) \
do {                            \
    err = 0;                    \
    p = dlsym(h, s);            \
    if (!p)                        \
        err = -ENODEV;            \
} while(0)

int snd_utils_get_int(struct snd_node *node, const char *prop, int *val)
{
    if (!node || !node->card_node || !node->dev_node)
        return SND_NODE_TYPE_HW;

    return node->get_int(node->dev_node, prop, val);
}

int snd_utils_get_str(struct snd_node *node, const char *prop, char **val)
{
    if (!node || !node->card_node || !node->dev_node)
        return SND_NODE_TYPE_HW;

    return node->get_str(node->dev_node, prop, val);
}

void snd_utils_put_dev_node(struct snd_node *node)
{
    if (!node)
        return;

    if (node->card_node)
        node->put_card(node->card_node);

    if (node->dl_hdl)
        dlclose(node->dl_hdl);

    free(node);
}

enum snd_node_type snd_utils_get_node_type(struct snd_node *node)
{
    int val = SND_NODE_TYPE_HW;

    if (!node || !node->card_node || !node->dev_node)
        return SND_NODE_TYPE_HW;

    node->get_int(node->dev_node, "type", &val);

    return val;
};


static int snd_utils_resolve_symbols(struct snd_node *node)
{
    void *dl = node->dl_hdl;
    int err;

    SND_DLSYM(dl, node->get_card, "snd_card_def_get_card", err);
    if (err)
        goto done;
    SND_DLSYM(dl, node->put_card, "snd_card_def_put_card", err);
    if (err)
        goto done;
    SND_DLSYM(dl, node->get_node, "snd_card_def_get_node", err);
    if (err)
        goto done;
    SND_DLSYM(dl, node->get_int, "snd_card_def_get_int", err);
    if (err)
        goto done;
    SND_DLSYM(dl, node->get_str, "snd_card_def_get_str", err);

done:
    return err;
}

struct snd_node *snd_utils_get_dev_node(unsigned int card,
        unsigned int device, int dev_type)
{
    struct snd_node *node;
    int rc = 0;

    node = calloc(1, sizeof(*node));
    if (!node)
        return NULL;

    node->dl_hdl = dlopen("libsndcardparser.so", RTLD_NOW);
    if (!node->dl_hdl) {
        goto err_dl_open;
    }

    rc = snd_utils_resolve_symbols(node);
    if (rc < 0)
        goto err_resolve_symbols;

    node->card_node = node->get_card(card);
    if (!node->card_node)
        goto err_resolve_symbols;

    node->dev_node = node->get_node(node->card_node,
                                    device, dev_type);
    if (!node->dev_node)
        goto err_get_node;

    return node;

err_get_node:
    node->put_card(node->card_node);

err_resolve_symbols:
    dlclose(node->dl_hdl);

err_dl_open:
    free(node);
    return NULL;
}
