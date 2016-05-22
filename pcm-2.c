#include <salsa/pcm.h>

#include <unistd.h>

void salsa_pcm_init(salsa_pcm * pcm){
	salsa_pcm_set_file_descriptor(pcm, -1);
}

void salsa_pcm_close(salsa_pcm * pcm){
	int fd = salsa_pcm_get_file_descriptor(pcm);
	if (fd >= 0){
		close(fd);
	}
}

int salsa_pcm_get_file_descriptor(const salsa_pcm * pcm){
	return pcm->fd;
}

void salsa_pcm_set_file_descriptor(salsa_pcm * pcm, int fd){
	pcm->fd = fd;
}

