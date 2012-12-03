/* tinypcminfo.c
**
** Copyright 2012, The Android Open Source Project
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
#include <string.h>

int main(int argc, char **argv)
{
    unsigned int device = 0;
    unsigned int card = 0;
    int i;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s -D card -d device\n", argv[0]);
        return 1;
    }

    /* parse command line arguments */
    argv += 1;
    while (*argv) {
        if (strcmp(*argv, "-D") == 0) {
            argv++;
            if (*argv)
                card = atoi(*argv);
        }
        if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (*argv)
                device = atoi(*argv);
        }
        if (*argv)
            argv++;
    }

    printf("Info for card %d, device %d:\n", card, device);

    for (i = 0; i < 2; i++) {
        struct pcm_params *params;
        unsigned int min;
        unsigned int max;

        printf("\nPCM %s:\n", i == 0 ? "out" : "in");

        params = pcm_params_get(card, device, i == 0 ? PCM_OUT : PCM_IN);
        if (params == NULL) {
            printf("Device does not exist.\n");
            continue;
        }

        min = pcm_params_get_min(params, PCM_PARAM_RATE);
        max = pcm_params_get_max(params, PCM_PARAM_RATE);
        printf("        Rate:\tmin=%uHz\tmax=%uHz\n", min, max);
        min = pcm_params_get_min(params, PCM_PARAM_CHANNELS);
        max = pcm_params_get_max(params, PCM_PARAM_CHANNELS);
        printf("    Channels:\tmin=%u\t\tmax=%u\n", min, max);
        min = pcm_params_get_min(params, PCM_PARAM_SAMPLE_BITS);
        max = pcm_params_get_max(params, PCM_PARAM_SAMPLE_BITS);
        printf(" Sample bits:\tmin=%u\t\tmax=%u\n", min, max);
        min = pcm_params_get_min(params, PCM_PARAM_PERIOD_SIZE);
        max = pcm_params_get_max(params, PCM_PARAM_PERIOD_SIZE);
        printf(" Period size:\tmin=%u\t\tmax=%u\n", min, max);
        min = pcm_params_get_min(params, PCM_PARAM_PERIODS);
        max = pcm_params_get_max(params, PCM_PARAM_PERIODS);
        printf("Period count:\tmin=%u\t\tmax=%u\n", min, max);

        pcm_params_free(params);
    }

    return 0;
}
