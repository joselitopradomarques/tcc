#ifndef AUDIO_H
#define AUDIO_H

#include <alsa/asoundlib.h>

// Definição do tamanho do buffer

// Função de inicialização: Configura o dispositivo de áudio ALSA
int inicializar(const char *device, snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **hw_params);

// Função de reprodução: Reproduz o áudio de um buffer processado
int reproduzir(snd_pcm_t *pcm_handle, short *media_buffer, size_t buffer_size);

// Função de finalização: Libera recursos e fecha dispositivos
void finalizar(snd_pcm_t *pcm_handle, short *buffer);

#endif // AUDIO_H
