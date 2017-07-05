/* tinyhostless.c
**
** Copyright 2011, The Android Open Source Project
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of The Android Open Source Project nor the names of
**       its contributors may be used to endorse or promote products derived
**       from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY The Android Open Source Project ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL The Android Open Source Project BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
** DAMAGE.
*/

/* Playback data to a PCM device recorded from a capture PCM device. */

#include <tinyalsa/asoundlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

/* Used when that particular device isn't opened. */
#define TINYHOSTLESS_DEVICE_UNDEFINED 255

static int close_h = 0;

int play_sample(unsigned int card, unsigned int p_device,
                unsigned int c_device, unsigned int channels,
                unsigned int rate, unsigned int bits, unsigned int period_size,
                unsigned int period_count, unsigned int play_cap_time,
                int do_loopback);

static void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close_h = 1;
}

int main(int argc, char **argv)
{
    unsigned int card = 0;
    unsigned int p_device = TINYHOSTLESS_DEVICE_UNDEFINED;
    unsigned int c_device = TINYHOSTLESS_DEVICE_UNDEFINED;
    unsigned int period_size = 192;
    unsigned int period_count = 4;
    unsigned int number_bits = 16;
    unsigned int num_channels = 2;
    unsigned int sample_rate = 48000;
    unsigned int play_cap_time = 0;
    unsigned int do_loopback = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-D card] [-P playback device]"
            " [-C capture device] [-p period_size] [-n n_periods]"
            " [-c num_channels] [-r sample_rate] [-l]"
            " [-T playback/capture time]\n\n"
            "Used to enable 'hostless' mode for audio devices with a DSP back-end.\n"
            "Alternatively, specify '-l' for loopback mode: this program will read\n"
            "from the capture device and write to the playback device.\n",
            argv[0]);
        return 1;
    }

   /* parse command line arguments */
    argv += 1;
    while (*argv) {
        if (strcmp(*argv, "-P") == 0) {
            argv++;
            if (*argv)
                p_device = atoi(*argv);
        }
        if (strcmp(*argv, "-C") == 0) {
            argv++;
            if (*argv)
                c_device = atoi(*argv);
        }
        if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (*argv)
                period_size = atoi(*argv);
        }
        if (strcmp(*argv, "-n") == 0) {
            argv++;
            if (*argv)
                period_count = atoi(*argv);
        }
        if (strcmp(*argv, "-c") == 0) {
            argv++;
            if (*argv)
                num_channels = atoi(*argv);
        }
        if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (*argv)
                sample_rate = atoi(*argv);
        }
        if (strcmp(*argv, "-T") == 0) {
            argv++;
            if (*argv)
                play_cap_time = atoi(*argv);
        }
        if (strcmp(*argv, "-D") == 0) {
            argv++;
            if (*argv)
                card = atoi(*argv);
        }
        if (strcmp(*argv, "-l") == 0) {
            do_loopback = 1;
        }
        if (*argv)
            argv++;
    }

    if (p_device == TINYHOSTLESS_DEVICE_UNDEFINED &&
        c_device == TINYHOSTLESS_DEVICE_UNDEFINED) {
        fprintf(stderr, "Specify at least one of -C (capture device) or -P (playback device).\n");
        return EINVAL;
    }

    if (do_loopback && (p_device == TINYHOSTLESS_DEVICE_UNDEFINED ||
                        c_device == TINYHOSTLESS_DEVICE_UNDEFINED)) {
        fprintf(stderr, "Loopback requires both playback and capture devices.\n");
        return EINVAL;
    }

    return play_sample(card, p_device, c_device, num_channels, sample_rate,
                       number_bits, period_size, period_count, play_cap_time,
                       do_loopback);
}

static int check_param(struct pcm_params *params, unsigned int param,
                       unsigned int value, char *param_name, char *param_unit)
{
    unsigned int min;
    unsigned int max;
    int is_within_bounds = 1;

    min = pcm_params_get_min(params, param);
    if (value < min) {
        fprintf(stderr, "%s is %u%s, device only supports >= %u%s\n", param_name, value,
                param_unit, min, param_unit);
        is_within_bounds = 0;
    }

    max = pcm_params_get_max(params, param);
    if (value > max) {
        fprintf(stderr, "%s is %u%s, device only supports <= %u%s\n", param_name, value,
                param_unit, max, param_unit);
        is_within_bounds = 0;
    }

    return is_within_bounds;
}

static int check_params(unsigned int card, unsigned int device, unsigned int direction,
                        const struct pcm_config *config)
{
    struct pcm_params *params;
    int can_play;
    int bits;

    params = pcm_params_get(card, device, direction);
    if (params == NULL) {
        fprintf(stderr, "Unable to open PCM %s device %u.\n",
                direction == PCM_OUT ? "playback" : "capture", device);
        return 0;
    }

    switch (config->format) {
    case PCM_FORMAT_S32_LE:
        bits = 32;
        break;
    case PCM_FORMAT_S24_3LE:
        bits = 24;
        break;
    case PCM_FORMAT_S16_LE:
        bits = 16;
        break;
    default:
        fprintf(stderr, "Invalid format: %u", config->format);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, config->rate, "Sample rate", "Hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, config->channels, "Sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, bits, "Bitrate", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, config->period_size, "Period size", " frames");
    can_play &= check_param(params, PCM_PARAM_PERIODS, config->period_count, "Period count", " periods");

    pcm_params_free(params);

    return can_play;
}

int play_sample(unsigned int card, unsigned int p_device,
                unsigned int c_device, unsigned int channels,
                unsigned int rate, unsigned int bits, unsigned int period_size,
                unsigned int period_count, unsigned int play_cap_time,
                int do_loopback)
{
    struct pcm_config config;
    struct pcm *pcm_play =NULL, *pcm_cap=NULL;
    unsigned int count =0;
    char *buffer = NULL;
    int size = 0;
    int rc = 0;
    struct timespec end;
    struct timespec now;

    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 24)
        config.format = PCM_FORMAT_S24_3LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.avail_min = 0;

    if(p_device < TINYHOSTLESS_DEVICE_UNDEFINED )  {
        if (!check_params(card, p_device, PCM_OUT, &config))
            return EINVAL;
        pcm_play = pcm_open(card, p_device, PCM_OUT, &config);
        if (!pcm_play || !pcm_is_ready(pcm_play)) {
            fprintf(stderr, "Unable to open PCM playback device %u (%s)\n",
                    p_device, pcm_get_error(pcm_play));
            return errno;
        }
    }

    if (c_device < TINYHOSTLESS_DEVICE_UNDEFINED ) {
        if (!check_params(card, c_device, PCM_IN, &config))
            return EINVAL;
        pcm_cap = pcm_open(card, c_device, PCM_IN, &config);
        if (!pcm_cap || !pcm_is_ready(pcm_cap)) {
            fprintf(stderr, "Unable to open PCM capture device %u (%s)\n",
                    c_device, pcm_get_error(pcm_cap));
            if (pcm_play != NULL ) pcm_close(pcm_play);
            return errno;
        }
    }

    printf("%s: Playing device %u, Capture Device %u\n",
           do_loopback ? "Loopback" : "Hostless", p_device, c_device);
    printf("Sample: %u ch, %u hz, %u bit\n", channels, rate, bits);
    if (play_cap_time)
        printf("Duration in sec: %u\n", play_cap_time);
    else
        printf("Duration in sec: forever\n");

    if (do_loopback) {
        size = pcm_frames_to_bytes(pcm_cap, pcm_get_buffer_size(pcm_cap));
        buffer = malloc(size);
        if (!buffer) {
            fprintf(stderr, "Unable to allocate %d bytes\n", size);
            pcm_close(pcm_play);
            pcm_close(pcm_cap);
            return ENOMEM;
        }
    }

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);
    signal(SIGHUP, stream_close);
    signal(SIGTERM, stream_close);

    if (pcm_cap != NULL) pcm_start(pcm_cap);
    if (pcm_play != NULL) pcm_start(pcm_play);

    clock_gettime(CLOCK_MONOTONIC, &now);
    end.tv_sec = now.tv_sec + play_cap_time;
    end.tv_nsec = now.tv_nsec;

    do  {
        if (do_loopback) {
            if (pcm_read(pcm_cap, buffer, size)) {
                fprintf(stderr, "Unable to read from PCM capture device %u (%s)\n",
                        c_device, pcm_get_error(pcm_cap));
                rc = errno;
                break;
            }
            if (pcm_write(pcm_play, buffer, size)) {
                fprintf(stderr, "Unable to write to PCM playback device %u (%s)\n",
                        p_device, pcm_get_error(pcm_play));
                break;
            }
        } else {
            usleep(100000);
        }
        if (play_cap_time) {
            clock_gettime(CLOCK_MONOTONIC, &now);
            if (now.tv_sec > end.tv_sec ||
                (now.tv_sec == end.tv_sec && now.tv_nsec >= end.tv_nsec))
                break;
        }
    } while(!close_h);

    if (buffer)
        free(buffer);
    if (pcm_play != NULL)
        pcm_close(pcm_play);
    if (pcm_cap != NULL)
        pcm_close(pcm_cap);
    return rc;
}
