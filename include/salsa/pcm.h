#ifndef SALSA_PCM_H
#define SALSA_PCM_H

typedef enum salsa_pcm_stream {
	SALSA_PCM_STREAM_PLAYBACK = 0,
	SALSA_PCM_STREAM_CAPTURE,
} salsa_pcm_stream;

typedef struct salsa_pcm {
	int fd;
} salsa_pcm;

void salsa_pcm_init(salsa_pcm * pcm);

void salsa_pcm_close(salsa_pcm * pcm);

int salsa_pcm_open(salsa_pcm * pcm, unsigned card, unsigned device, salsa_pcm_stream);

int salsa_pcm_get_file_descriptor(const salsa_pcm * pcm);

void salsa_pcm_set_file_descriptor(salsa_pcm * pcm, int fd);

#endif /* SALSA_PCM_H */

