#include <stdio.h>
#include <stdlib.h>

#include <tinyalsa/pcm.h>

static long int file_size(FILE * file)
{
    if (fseek(file, 0, SEEK_END) < 0) {
        return -1;
    }
    long int file_size = ftell(file);
    if (fseek(file, 0, SEEK_SET) < 0) {
        return -1;
    }
    return file_size;
}

static size_t read_file(void ** frames){

    FILE * input_file = fopen("audio.raw", "rb");
    if (input_file == NULL) {
        perror("failed to open 'audio.raw' for writing");
        return 0;
    }

    long int size = file_size(input_file);
    if (size < 0) {
        perror("failed to get file size of 'audio.raw'");
        fclose(input_file);
        return 0;
    }

    *frames = malloc(size);
    if (*frames == NULL) {
        fprintf(stderr, "failed to allocate frames\n");
        fclose(input_file);
        return 0;
    }

    size = fread(*frames, 1, size, input_file);

    fclose(input_file);

    return size;
}

static int write_frames(const void * frames, size_t byte_count){

    unsigned int card = 0;
    unsigned int device = 0;
    int flags = PCM_OUT;

    const struct pcm_config config = {
        .channels = 2,
        .rate = 48000,
        .format = PCM_FORMAT_S32_LE,
        .period_size = 1024,
        .period_count = 2,
        .start_threshold = 1024,
        .silence_threshold = 1024 * 2,
        .stop_threshold = 1024 * 2
    };

    struct pcm * pcm = pcm_open(card, device, flags, &config);
    if (pcm == NULL) {
        fprintf(stderr, "failed to allocate memory for PCM\n");
        return -1;
    } else if (!pcm_is_ready(pcm)){
        pcm_close(pcm);
        fprintf(stderr, "failed to open PCM\n");
        return -1;
    }

    unsigned int frame_count = pcm_bytes_to_frames(pcm, byte_count);

    int err = pcm_writei(pcm, frames, frame_count);
    if (err < 0) {
      printf("error: %s\n", pcm_get_error(pcm));
    }

    pcm_close(pcm);

    return 0;
}

int main(void)
{
    void *frames;
    size_t size;

    size = read_file(&frames);
    if (size == 0) {
        return EXIT_FAILURE;
    }

    if (write_frames(frames, size) < 0) {
        return EXIT_FAILURE;
    }

    free(frames);
    return EXIT_SUCCESS;
}

