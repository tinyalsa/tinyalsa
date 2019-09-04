/* pcm_hw.c
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
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sound/asound.h>
#include <tinyalsa/asoundlib.h>

#include "pcm_io.h"

struct pcm_hw_data {
    /** Card number of the pcm device */
    unsigned int card;
    /** Device number for the pcm device */
    unsigned int device;
    /** File descriptor to the pcm device file node */
    unsigned int fd;
    /** Pointer to the pcm node from snd card definiton */
    void *node;
};

static void pcm_hw_close(void *data)
{
    struct pcm_hw_data *hw_data = data;

    if (hw_data->fd > 0)
        close(hw_data->fd);

    free(hw_data);
}

static int pcm_hw_ioctl(void *data, unsigned int cmd, ...)
{
    struct pcm_hw_data *hw_data = data;
    va_list ap;
    void *arg;

    va_start(ap, cmd);
    arg = va_arg(ap, void *);
    va_end(ap);

    return ioctl(hw_data->fd, cmd, arg);
}

static int pcm_hw_open(unsigned int card, unsigned int device,
                unsigned int flags, void **data, void *node)
{
    struct pcm_hw_data *hw_data;
    char fn[256];
    int fd;

    hw_data = calloc(1, sizeof(*hw_data));
    if (!hw_data) {
        return -ENOMEM;
    }

    snprintf(fn, sizeof(fn), "/dev/snd/pcmC%uD%u%c", card, device,
             flags & PCM_IN ? 'c' : 'p');
    if (flags & PCM_NONBLOCK)
        fd = open(fn, O_RDWR|O_NONBLOCK);
    else
        fd = open(fn, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "%s: cannot open device '%s'",
                __func__, fn);
        return fd;
    }

    hw_data->card = card;
    hw_data->device = device;
    hw_data->fd = fd;
    hw_data->node = node;

    *data = hw_data;

    return fd;
}

struct pcm_ops hw_ops = {
    .open = pcm_hw_open,
    .close = pcm_hw_close,
    .ioctl = pcm_hw_ioctl,
};

