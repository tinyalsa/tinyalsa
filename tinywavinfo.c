/* tinywavinfo.c
**
** Copyright 2015, The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <malloc.h>

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

static int close = 0;

void analyse_sample(FILE *file, unsigned int channels, unsigned int bits,
                    unsigned int data_chunk_size);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close = 1;
}

int main(int argc, char **argv)
{
    FILE *file;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;
    char *filename;
    int more_chunks = 1;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav \n", argv[0]);
        return 1;
    }

    filename = argv[1];
    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        return 1;
    }

    fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
        fclose(file);
        return 1;
    }

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, file);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(chunk_fmt))
                fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = 0;
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(file, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    printf("Input File       : %s \n", filename);
    printf("Channels         : %u \n", chunk_fmt.num_channels);
    printf("Sample Rate      : %u \n", chunk_fmt.sample_rate);
    printf("Bits per sample  : %u \n\n", chunk_fmt.bits_per_sample);

    analyse_sample(file, chunk_fmt.num_channels, chunk_fmt.bits_per_sample,
                    chunk_header.sz);

    fclose(file);

    return 0;
}

void analyse_sample(FILE *file, unsigned int channels, unsigned int bits,
                    unsigned int data_chunk_size)
{
    void *buffer;
    int size;
    int num_read;
    int i;
    unsigned int ch;
    int frame_size = 1024;
    unsigned int byte_align = 0;
    float *power;
    int total_sample_per_channel;
    float normalization_factor;

    if (bits == 32)
        byte_align = 4;
    else if (bits == 16)
        byte_align = 2;

    normalization_factor = (float)pow(2.0, (bits-1));

    size = channels * byte_align * frame_size;
    buffer = memalign(byte_align, size);

    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        return;
    }

    power = (float *) calloc(channels, sizeof(float));

    total_sample_per_channel = data_chunk_size / (channels * byte_align);

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    do {
        num_read = fread(buffer, 1, size, file);
        if (num_read > 0) {
            if (2 == byte_align) {
                short *buffer_ptr = (short *)buffer;
                for (i = 0; i < num_read; i += channels) {
                    for (ch = 0; ch < channels; ch++) {
                        int temp = *buffer_ptr++;
                        /* Signal Normalization */
                        float f = (float) temp / normalization_factor;
                        *(power + ch) += (float) (f * f);
                    }
                }
            }
            if (4 == byte_align) {
                int *buffer_ptr = (int *)buffer;
                for (i = 0; i < num_read; i += channels) {
                    for (ch = 0; ch < channels; ch++) {
                        int temp = *buffer_ptr++;
                        /* Signal Normalization */
                        float f = (float) temp / normalization_factor;
                        *(power + ch) += (float) (f * f);
                    }
                }
            }
        }
    }while (!close && num_read > 0);

    for (ch = 0; ch < channels; ch++) {
        float average_power = 10 * log10((*(power + ch)) / total_sample_per_channel);
        if(isinf (average_power)) {
            printf("Channel [%2u] Average Power : NO signal or ZERO signal\n", ch);
        } else {
            printf("Channel [%2u] Average Power : %.2f dB\n", ch, average_power);
        }
    }

    free(buffer);
    free(power);

}

