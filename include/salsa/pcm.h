#ifndef SALSA_PCM_H
#define SALSA_PCM_H

typedef struct salsa_pcm {
	int fd;
} salsa_pcm;

void salsa_pcm_init(salsa_pcm * pcm);

void salsa_pcm_free(salsa_pcm * pcm);

int salsa_pcm_get_file_descriptor(const salsa_pcm * pcm);

void salsa_pcm_set_file_descriptor(salsa_pcm * pcm, int fd);

#endif /* SALSA_PCM_H */

