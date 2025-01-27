#ifndef PLAY_H
#define PLAY_H

#include <alsa/asoundlib.h>
#include "capture.h"  // Para utilizar os buffers circulares e outros parâmetros

// Função para inicializar a reprodução
int init_playback(snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **params);

// Função para reproduzir áudio a partir dos buffers
void *play_audio_thread(void *args);

// Função para liberar recursos da reprodução
void close_playback(snd_pcm_t *pcm_handle);

#endif
