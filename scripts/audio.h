#ifndef AUDIO_H
#define AUDIO_H

#include <alsa/asoundlib.h>

// Definição do tamanho do buffer
#define BUFFER_SIZE 1024

// Função de inicialização: Configura o dispositivo de áudio ALSA
int inicializar(const char *device, snd_pcm_t **pcm_handle, char **buffer, snd_pcm_hw_params_t **hw_params);

// Função de reprodução: Reproduz o áudio de um buffer processado
int reproduzir(snd_pcm_t *pcm_handle, char *media_buffer, size_t buffer_size);

// Função de finalização: Libera recursos e fecha dispositivos
void finalizar(snd_pcm_t *pcm_handle, char *buffer);

#endif // AUDIO_H
