#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define BUFFER_SIZE 1024

// Função de inicialização: Configura o dispositivo de áudio ALSA
int inicializar(const char *device, snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **hw_params) {
    int err;

    // Abre o dispositivo de áudio ALSA
    if ((err = snd_pcm_open(pcm_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s: %s\n", device, snd_strerror(err));
        return -1;
    }

    // Configura os parâmetros de hardware do ALSA
    snd_pcm_hw_params_alloca(hw_params);
    snd_pcm_hw_params_any(*pcm_handle, *hw_params);
    snd_pcm_hw_params_set_access(*pcm_handle, *hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*pcm_handle, *hw_params, SND_PCM_FORMAT_S16_LE); // Formato PCM 16-bit
    snd_pcm_hw_params_set_channels(*pcm_handle, *hw_params, 2); // Estéreo (2 canais)
    snd_pcm_hw_params_set_rate(*pcm_handle, *hw_params, 44100, 0); // Taxa de amostragem de 44.1 kHz

    // Aplica a configuração
    if ((err = snd_pcm_hw_params(*pcm_handle, *hw_params)) < 0) {
        fprintf(stderr, "Erro ao configurar parâmetros de hardware: %s\n", snd_strerror(err));
        snd_pcm_close(*pcm_handle);
        return -1;
    }

    // Se a inicialização for bem-sucedida
    printf("Stream de áudio inicializado com sucesso no dispositivo: %s\n", device);

    return 0;
}


// Função de reprodução: Reproduz o áudio de um buffer processado
int reproduzir(snd_pcm_t *pcm_handle, short *media_buffer, size_t buffer_size) {
    int err;

    // Reproduz o conteúdo do media_buffer
    if ((err = snd_pcm_writei(pcm_handle, media_buffer, buffer_size/4)) < 0) { // Divide por 4 (2 canais * 2 bytes por amostra)
        fprintf(stderr, "Erro ao escrever no dispositivo: %s\n", snd_strerror(err));
        return -1;
    }

    return 1; // Áudio ainda está sendo reproduzido
}

// Função de finalização: Libera recursos e fecha dispositivos
void finalizar(snd_pcm_t *pcm_handle, short *buffer) {
    snd_pcm_drain(pcm_handle); // Drena o buffer de reprodução
    snd_pcm_close(pcm_handle); // Fecha o dispositivo de áudio
    free(buffer); // Libera o buffer
}
