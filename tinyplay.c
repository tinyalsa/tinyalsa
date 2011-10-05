/* tinyplay.c
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

#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

static int (*_pcm_write)(struct pcm *, void *, unsigned int );

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

static int close = 0;

void play_sample(FILE *file, unsigned int device, unsigned int mmap,
                 unsigned int noirq, struct pcm_config *config,
                 unsigned int bits);
void init_pcm_config(struct pcm_config *config, unsigned int channels,
                     unsigned int rate, unsigned int bits,
                     unsigned int period_size, unsigned int period_count,
                     unsigned int start_threshold, unsigned int stop_threshold,
                     unsigned int avail_min, unsigned int silence);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close = 1;
}

static void usage(char *name)
{
     fprintf(stderr, "Usage: %s file.wav [-d device] [-r rate] [-c channels]"
         " [-f format 16/24/32] [-raw] [-ps period-size] [-pc period-count]"
         " [-av avail-min] [-start] [-stop] [-silence] [-m] [-noirq]\n", name);
     exit(1);
}

static int get_int(int argc, char **argv, int *index)
{
    if (++(*index) >= argc)
        usage(argv[0]);

    return atoi(argv[*index]);
}

int main(int argc, char **argv)
{
    FILE *file;
    struct wav_header header;
    unsigned int device = 0;
    unsigned int mmap = 0;
    unsigned int raw = 0;
    unsigned int rate = 48000;
    unsigned int channels = 2;
    unsigned int bits = 16;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    unsigned int start_threshold = 0;
    unsigned int stop_threshold = 0;
    unsigned int avail_min = 1;
    unsigned int silence = 0;
    unsigned int noirq = 0;
    struct pcm_config config;
    int i;

    if (argc < 2)
        usage(argv[0]);

    file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", argv[1]);
        return 1;
    }

    /* parse command line arguments */
    for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            device = get_int(argc, argv, &i);
        if (strcmp(argv[i], "-m") == 0) {
            mmap = 1;
            continue;
        }
        if (strcmp(argv[i], "-raw") == 0)
            raw = 1;
            continue;
        }
        if (strcmp(argv[i], "-noirq") == 0) {
            noirq = 1;
            continue;
        }
        if (strcmp(argv[i], "-r") == 0) {
            rate = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-c") == 0) {
            channels = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-f") == 0) {
            bits = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-ps") == 0) {
            period_size = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-pc") == 0) {
            period_count = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-avail-min") == 0) {
            avail_min = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-start") == 0) {
            start_threshold = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-stop") == 0) {
            stop_threshold = get_int(argc, argv, &i);
            continue;
        }
        if (strcmp(argv[i], "-silence") == 0) {
            stop_threshold = get_int(argc, argv, &i);
            continue;
        }
    }

    if (!raw) {
        fread(&header, sizeof(struct wav_header), 1, file);

        if ((header.riff_id != ID_RIFF) ||
            (header.riff_fmt != ID_WAVE) ||
            (header.fmt_id != ID_FMT) ||
            (header.audio_format != FORMAT_PCM) ||
            (header.fmt_sz != 16)) {
            fprintf(stderr, "Error: '%s' is not a PCM riff/wave file\n", argv[1]);
            fclose(file);
            return 1;
        }
        channels = header.num_channels;
        rate = header.sample_rate;
        bits = header.bits_per_sample;
    }

    init_pcm_config(&config, channels, rate, bits, period_size, period_count,
        start_threshold, stop_threshold, avail_min, silence);
    play_sample(file, device, mmap, noirq, &config, bits);

    fclose(file);

    return 0;
}

void init_pcm_config(struct pcm_config *config, unsigned int channels,
                     unsigned int rate, unsigned int bits,
                     unsigned int period_size, unsigned int period_count,
                     unsigned int start_threshold, unsigned int stop_threshold,
                     unsigned int avail_min, unsigned int silence)
{
    config->channels = channels;
    config->rate = rate;
    config->period_size = period_size;
    config->period_count = period_count;
    switch (bits) {
    case 24:
        config->format = PCM_FORMAT_S24_LE;
        break;
    case 32:
        config->format = PCM_FORMAT_S32_LE;
        break;
    case 16:
    default:
        config->format = PCM_FORMAT_S16_LE;
        break;
    }
    config->start_threshold = start_threshold;
    config->stop_threshold = stop_threshold;
    config->avail_min = avail_min;
    config->silence_threshold = silence;
}

void play_sample(FILE *file, unsigned int device, unsigned int mmap,
                 unsigned int noirq, struct pcm_config *config,
                 unsigned int bits)
{
    struct pcm *pcm;
    char *buffer;
    int size;
    int num_read;
    int flags = PCM_OUT;

    if (mmap) {
        flags |= PCM_MMAP;
        _pcm_write = pcm_mmap_write;
        if (config->avail_min == 1)
            config->avail_min = config->period_size;
    } else
        _pcm_write = pcm_write;
    pcm = pcm_open(0, device, flags, config);
    if (noirq)
        flags |= PCM_NOIRQ;

    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device %u (%s)\n",
                device, pcm_get_error(pcm));
        return;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return;
    }

    printf("Playing samples: %u ch, %u hz, %u bit, %u periods of %u frames\n",
           config->channels, config->rate, bits, config->period_count,
           config->period_size);

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    do {
        int err;
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if ((err = _pcm_write(pcm, buffer, num_read)) != 0) {
                fprintf(stderr, "Error playing sample %d\n", err);
                break;
            }
        }
    } while (!close && num_read > 0);

    free(buffer);
    pcm_close(pcm);
}

