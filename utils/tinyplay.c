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
#include <string.h>
#include <signal.h>

struct cmd {
    const char *filename;
    const char *filetype;
    unsigned int card;
    unsigned int device;
    int flags;
    struct pcm_config config;
    unsigned int bits;
};

void cmd_init(struct cmd *cmd)
{
    cmd->filename = NULL;
    cmd->filetype = NULL;
    cmd->card = 0;
    cmd->device = 0;
    cmd->flags = PCM_OUT;
    cmd->config.period_size = 1024;
    cmd->config.period_count = 2;
    cmd->config.channels = 2;
    cmd->config.rate = 48000;
    cmd->config.format = PCM_FORMAT_S16_LE;
    cmd->config.silence_threshold = 1024 * 2;
    cmd->config.stop_threshold = 1024 * 2;
    cmd->config.start_threshold = 1024;
    cmd->bits = 16;
}

int cmd_parse_arg(struct cmd *cmd, int argc, const char **argv)
{
    if (argc < 1) {
        return 0;
    }

    if ((strcmp(argv[0], "-M") == 0) || (strcmp(argv[0], "--mmap") == 0)) {
        cmd->flags |= PCM_MMAP;
        return 1;
    }

    if ((strcmp(argv[0], "-B") == 0) || (strcmp(argv[0], "--dmabuf") == 0)) {
        cmd->flags |= PCM_DMABUF;
        return 1;
    }

    if (argv[0][0] != '-' || (strcmp(argv[0],"-") == 0)) {
        cmd->filename = argv[0];
        return 1;
    }

    if (argc < 2) {
        fprintf(stderr, "option '%s' is missing argument\n", argv[0]);
        return -1;
    }

    if ((strcmp(argv[0], "-D") == 0) || (strcmp(argv[0], "--card") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->card) != 1) {
            fprintf(stderr, "failed parsing card number '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-d") == 0) || (strcmp(argv[0], "--device") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->device) != 1) {
            fprintf(stderr, "failed parsing device number '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-p") == 0) || (strcmp(argv[0], "--period-size") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->config.period_size) != 1) {
            fprintf(stderr, "failed parsing period size '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-n") == 0) || (strcmp(argv[0], "--period-count") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->config.period_count) != 1) {
            fprintf(stderr, "failed parsing period count '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-c") == 0) || (strcmp(argv[0], "--channels") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->config.channels) != 1) {
            fprintf(stderr, "failed parsing channel count '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-r") == 0) || (strcmp(argv[0], "--rate") == 0)) {
        if (sscanf(argv[1], "%u", &cmd->config.rate) != 1) {
            fprintf(stderr, "failed parsing rate '%s'\n", argv[1]);
            return -1;
        }
    } else if ((strcmp(argv[0], "-i") == 0) || (strcmp(argv[0], "--file-type") == 0)) {
        cmd->filetype = argv[1];
    } else {
        fprintf(stderr, "unknown option '%s'\n", argv[0]);
        return -1;
    }
    return 2;
}

int cmd_parse_args(struct cmd *cmd, int argc, const char **argv)
{
    int i = 0;
    while (i < argc) {
        int j = cmd_parse_arg(cmd, argc - i, &argv[i]);
        if (j < 0){
            break;
        }
        i += j;
    }

    if ((cmd->filename != NULL)
     && (cmd->filetype == NULL)) {
        cmd->filetype = strrchr(cmd->filename, '.');
        if (cmd->filetype != NULL) {
            cmd->filetype++;
        }
    }

    return i;
}

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

struct ctx {
    struct pcm *pcm;

    struct riff_wave_header wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt chunk_fmt;

    FILE *file;
};

int ctx_init(struct ctx* ctx, const struct cmd *cmd)
{
    unsigned int bits = cmd->bits;
    struct pcm_config config = cmd->config;

    if (cmd->filename == NULL) {
        fprintf(stderr, "filename not specified\n");
        return -1;
    }
    if (strcmp(cmd->filename, "-") == 0) {
	    ctx->file = stdin;
    } else {
	    ctx->file = fopen(cmd->filename, "rb");
	}
    if (ctx->file == NULL) {
        fprintf(stderr, "failed to open '%s'\n", cmd->filename);
        return -1;
    }

    if ((cmd->filetype != NULL) && (strcmp(cmd->filetype, "wav") == 0)) {
        if (fread(&ctx->wave_header, sizeof(ctx->wave_header), 1, ctx->file) != 1){
            fprintf(stderr, "error: '%s' does not contain a riff/wave header\n", cmd->filename);
            fclose(ctx->file);
            return -1;
        }
        if ((ctx->wave_header.riff_id != ID_RIFF) ||
            (ctx->wave_header.wave_id != ID_WAVE)) {
            fprintf(stderr, "error: '%s' is not a riff/wave file\n", cmd->filename);
            fclose(ctx->file);
            return -1;
        }
	unsigned int more_chunks = 1;
        do {
            if (fread(&ctx->chunk_header, sizeof(ctx->chunk_header), 1, ctx->file) != 1){
                fprintf(stderr, "error: '%s' does not contain a data chunk\n", cmd->filename);
                fclose(ctx->file);
                return -1;
            }
            switch (ctx->chunk_header.id) {
            case ID_FMT:
                if (fread(&ctx->chunk_fmt, sizeof(ctx->chunk_fmt), 1, ctx->file) != 1){
                    fprintf(stderr, "error: '%s' has incomplete format chunk\n", cmd->filename);
                    fclose(ctx->file);
                    return -1;
                }
                /* If the format header is larger, skip the rest */
                if (ctx->chunk_header.sz > sizeof(ctx->chunk_fmt))
                    fseek(ctx->file, ctx->chunk_header.sz - sizeof(ctx->chunk_fmt), SEEK_CUR);
                break;
            case ID_DATA:
                /* Stop looking for chunks */
                more_chunks = 0;
                break;
            default:
                /* Unknown chunk, skip bytes */
                fseek(ctx->file, ctx->chunk_header.sz, SEEK_CUR);
            }
        } while (more_chunks);
        config.channels = ctx->chunk_fmt.num_channels;
        config.rate = ctx->chunk_fmt.sample_rate;
        bits = ctx->chunk_fmt.bits_per_sample;
    }

    if (bits == 8) {
        config.format = PCM_FORMAT_S8;
    } else if (bits == 16) {
        config.format = PCM_FORMAT_S16_LE;
    } else if (bits == 24) {
        config.format = PCM_FORMAT_S24_3LE;
    } else if (bits == 32) {
        config.format = PCM_FORMAT_S32_LE;
    } else {
        fprintf(stderr, "bit count '%u' not supported\n", bits);
        fclose(ctx->file);
        return -1;
    }

    ctx->pcm = pcm_open(cmd->card,
                        cmd->device,
                        cmd->flags,
                        &config);
    if (ctx->pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for pcm\n");
        fclose(ctx->file);
        return -1;
    } else if (!pcm_is_ready(ctx->pcm)) {
        fprintf(stderr, "failed to open for pcm %u,%u\n", cmd->card, cmd->device);
        fclose(ctx->file);
        pcm_close(ctx->pcm);
        return -1;
    }

    return 0;
}

void ctx_free(struct ctx *ctx)
{
    if (ctx == NULL) {
        return;
    }
    if (ctx->pcm != NULL) {
        pcm_close(ctx->pcm);
    }
    if (ctx->file != NULL) {
        fclose(ctx->file);
    }
}

static int close = 0;

int play_sample(struct ctx *ctx, const struct cmd *cmd);

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
    close = 1;
}

void print_usage(const char *argv0)
{
    fprintf(stderr, "usage: %s file.wav [options]\n", argv0);
    fprintf(stderr, "options:\n");
    fprintf(stderr, "-D | --card   <card number>    The device to receive the audio\n");
    fprintf(stderr, "-d | --device <device number>  The card to receive the audio\n");
    fprintf(stderr, "-p | --period-size <size>      The size of the PCM's period\n");
    fprintf(stderr, "-n | --period-count <count>    The number of PCM periods\n");
    fprintf(stderr, "-i | --file-type <file-type >  The type of file to read (raw or wav)\n");
    fprintf(stderr, "-c | --channels <count>        The amount of channels per frame\n");
    fprintf(stderr, "-r | --rate <rate>             The amount of frames per second\n");
    fprintf(stderr, "-b | --bits <bit-count>        The number of bits in one sample\n");
    fprintf(stderr, "-M | --mmap                    Use memory mapped IO to play audio\n");
}

int main(int argc, const char **argv)
{
    struct cmd cmd;
    struct ctx ctx;

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    cmd_init(&cmd);
    if (cmd_parse_args(&cmd, argc - 1, &argv[1]) < 0) {
        return EXIT_FAILURE;
    }

    if (ctx_init(&ctx, &cmd) < 0) {
        return EXIT_FAILURE;
    }

    /* TODO get parameters from context */
    printf("playing '%s': %u ch, %u hz, %u bit\n",
           cmd.filename,
           cmd.config.channels,
           cmd.config.rate,
           cmd.bits);

    if (play_sample(&ctx, &cmd) < 0) {
        ctx_free(&ctx);
        return EXIT_FAILURE;
    }

    ctx_free(&ctx);
    return EXIT_SUCCESS;
}

int check_param(struct pcm_params *params, unsigned int param, unsigned int value,
                 char *param_name, char *param_unit)
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

int sample_is_playable(const struct cmd *cmd)
{
    struct pcm_params *params;
    int can_play;

    params = pcm_params_get(cmd->card, cmd->device, PCM_OUT);
    if (params == NULL) {
        fprintf(stderr, "unable to open PCM %u,%u\n", cmd->card, cmd->device);
        return 0;
    }

    can_play = check_param(params, PCM_PARAM_RATE, cmd->config.rate, "sample rate", "hz");
    can_play &= check_param(params, PCM_PARAM_CHANNELS, cmd->config.channels, "sample", " channels");
    can_play &= check_param(params, PCM_PARAM_SAMPLE_BITS, cmd->bits, "bits", " bits");
    can_play &= check_param(params, PCM_PARAM_PERIOD_SIZE, cmd->config.period_size, "period size", "");
    can_play &= check_param(params, PCM_PARAM_PERIODS, cmd->config.period_count, "period count", "");

    pcm_params_free(params);

    return can_play;
}

int play_sample(struct ctx *ctx, const struct cmd *cmd)
{
    char *buffer;
    int size;
    int num_read;
    int ret;

    size = pcm_frames_to_bytes(ctx->pcm, pcm_get_buffer_size(ctx->pcm));
    buffer = malloc(size);
    if (!buffer) {
        fprintf(stderr, "unable to allocate %d bytes\n", size);
        return -1;
    }

    /* catch ctrl-c to shutdown cleanly */
    signal(SIGINT, stream_close);

    do {
        num_read = fread(buffer, 1, size, ctx->file);
        if (num_read > 0) {
            if (cmd->flags & (PCM_MMAP | PCM_DMABUF))
                ret = pcm_mmap_write(ctx->pcm, buffer, num_read);
            else
                ret = pcm_writei(ctx->pcm, buffer,
                                 pcm_bytes_to_frames(ctx->pcm, num_read));
            if (ret < 0) {
                fprintf(stderr, "error playing sample\n");
                break;
            }
        }
    } while (!close && num_read > 0);

    free(buffer);
    return 0;
}

