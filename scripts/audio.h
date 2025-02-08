#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <alsa/asoundlib.h>

// Função de inicialização: Abre arquivo, aloca buffer e configura o dispositivo de áudio ALSA
int inicializar(const char *audio_file, const char *device, snd_pcm_t **pcm_handle, FILE **file, char **buffer, snd_pcm_hw_params_t **hw_params);

// Função de reprodução: Reproduz o áudio em buffers de forma iterativa
int reproduzir(snd_pcm_t *pcm_handle, FILE *file, char *buffer);

// Função de finalização: Libera recursos e fecha arquivos/dispositivos
void finalizar(snd_pcm_t *pcm_handle, FILE *file, char *buffer);

#endif
