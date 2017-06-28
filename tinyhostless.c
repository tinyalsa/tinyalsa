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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>

static int close_h = 0;

void play_sample(unsigned int card, unsigned int p_device, unsigned int c_device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count, unsigned int play_cap_time);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close_h = 1;
}

int main(int argc, char **argv)
{
    unsigned int card = 0;
    unsigned int p_device = 255;
    unsigned int c_device = 255;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    unsigned int number_bits = 16;
    unsigned int num_channels = 2;
    unsigned int sample_rate = 48000;
    unsigned int play_cap_time = 0;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-D card] [-P Hostless Playback device] [-C Hostless Capture device] [-p period_size]"
                " [-n n_periods] [-c num_channels] [-r sample_rate] [-T playback/capture time]\n", argv[0]);
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
        if (*argv)
            argv++;
    }

    printf (" Hostless .. p_device =%d, c_device =%d, num_channel =%d, sample_rate =%d, number_bits =%d, play_cap_time =%d\n", p_device, c_device, num_channels, sample_rate, number_bits, play_cap_time);

    play_sample(card, p_device, c_device, num_channels, sample_rate, number_bits,
                period_size, period_count, play_cap_time);

    return 0;
}

void play_sample(unsigned int card, unsigned int p_device, unsigned int c_device, unsigned int channels,
                 unsigned int rate, unsigned int bits, unsigned int period_size,
                 unsigned int period_count, unsigned int play_cap_time)
{
    struct pcm_config config;
    struct pcm *pcm_play =NULL, *pcm_cap=NULL;

    unsigned int count =0;
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    if (bits == 32)
        config.format = PCM_FORMAT_S32_LE;
    else if (bits == 16)
        config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = INT_MAX;
    config.avail_min = 0;

    if(p_device < 255 )  {
        pcm_play = pcm_open(card, p_device, PCM_OUT, &config);
        if (!pcm_play || !pcm_is_ready(pcm_play)) {
            fprintf(stderr, "Unable to open PCM device %u (%s)\n",
            p_device, pcm_get_error(pcm_play));
            return;
        }
    }

    if (c_device < 255 ) {
        pcm_cap = pcm_open(card, c_device, PCM_IN, &config);
        if (!pcm_cap || !pcm_is_ready(pcm_cap)) {
            fprintf(stderr, "Unable to open PCM device %u (%s)\n",
                    c_device, pcm_get_error(pcm_cap));
            if (pcm_play != NULL ) pcm_close(pcm_play);
            return;
        }
    }

    printf("Hostless Playing device %u, Capture Device %u, sample: %u ch, %u hz, %u bit playback/capture duration in sec %u\n", p_device, c_device, channels, rate, bits, play_cap_time);

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    if (pcm_cap != NULL) pcm_start(pcm_cap);
    if (pcm_play != NULL) pcm_start(pcm_play);

    do  {
        count++;
        usleep(10000);
        if ( (play_cap_time > 0) && ((count/100) > play_cap_time)) break;
    } while(!close_h);

    if (pcm_play != NULL) pcm_close(pcm_play);
    if (pcm_cap != NULL) pcm_close(pcm_cap);
}

