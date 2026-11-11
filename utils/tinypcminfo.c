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

#define OPTPARSE_IMPLEMENTATION
#include "optparse.h"

#define BUFFER_SIZE 4096


int main(int argc, char **argv)
{
    unsigned int device = 0;
    unsigned int card = 0;
    int i;
    struct optparse opts;
    struct optparse_long long_options[] = {
        { "help",   'h', OPTPARSE_NONE     },
        { "card",   'D', OPTPARSE_REQUIRED },
        { "device", 'd', OPTPARSE_REQUIRED },
        { 0, 0, 0 }
    };
    char pcm_params_str[BUFFER_SIZE];

    (void)argc; /* silence -Wunused-parameter */
    /* parse command line arguments */
    optparse_init(&opts, argv);
    while ((i = optparse_long(&opts, long_options, NULL)) != -1) {
        switch (i) {
        case 'D':
            card = atoi(opts.optarg);
            break;
        case 'd':
            device = atoi(opts.optarg);
            break;
        case 'h':
            fprintf(stderr, "Usage: %s -D card -d device\n", argv[0]);
            return 0;
        case '?':
            fprintf(stderr, "%s\n", opts.errmsg);
            return EXIT_FAILURE;
        }
    }

    printf("Info for card %u, device %u:\n", card, device);

    for (i = 0; i < 2; i++) {
        struct pcm_params *params;

        printf("\nPCM %s:\n", i == 0 ? "out" : "in");

        params = pcm_params_get(card, device, i == 0 ? PCM_OUT : PCM_IN);
        if (params == NULL) {
            printf("Device does not exist.\n");
            continue;
        }

        pcm_params_str[0] = '\0';
        pcm_params_str[BUFFER_SIZE - 1] = '\0';

        if (pcm_params_to_string(params, pcm_params_str, BUFFER_SIZE - 1) >= BUFFER_SIZE) {
            fprintf(stderr, "Warning, output of pcm_params_to_string function is truncated.\n");
        }

        printf("%s\n", pcm_params_str);

        pcm_params_free(params);
    }

    return 0;
}
