#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "play.h"
#include "capture.h"

// Função para inicializar a reprodução
int init_playback(snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **params) {
    int err;

    // Configura a captura de áudio para reprodução
    if ((err = snd_pcm_open(pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo de reprodução: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_malloc(params)) < 0) {
        fprintf(stderr, "Erro ao alocar memória para parâmetros de hardware: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_any(*pcm_handle, *params)) < 0) {
        fprintf(stderr, "Erro ao inicializar parâmetros de hardware: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_format(*pcm_handle, *params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf(stderr, "Erro ao configurar formato de amostra: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_channels(*pcm_handle, *params, 2)) < 0) {
        fprintf(stderr, "Erro ao configurar número de canais: %s\n", snd_strerror(err));
        return -1;
    }

    unsigned int rate = 44100;  // Taxa de amostragem padrão
    if ((err = snd_pcm_hw_params_set_rate_near(*pcm_handle, *params, &rate, 0)) < 0) {
        fprintf(stderr, "Erro ao configurar taxa de amostragem: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params(*pcm_handle, *params)) < 0) {
        fprintf(stderr, "Erro ao configurar parâmetros de hardware: %s\n", snd_strerror(err));
        return -1;
    }

    return 0;
}

// Função para reproduzir áudio a partir dos buffers
void *play_audio_thread(void *args) {
    CaptureThreadArgs *capture_args = (CaptureThreadArgs *)args;
    snd_pcm_t *pcm_handle = capture_args->pcm_handle;
    CircularBuffer *write_buffer = capture_args->write_buffer;
    CircularBuffer *read_buffer = capture_args->read_buffer;
    size_t buffer_size = UPDATE_BUFFER_SIZE * 1024;  // Tamanho do buffer de áudio

    short *audio_data = (short *)malloc(buffer_size);

    if (!audio_data) {
        fprintf(stderr, "Erro ao alocar memória para os dados de áudio\n");
        return NULL;
    }

    while (1) {
        // Verificar se há dados no buffer de leitura para reprodução
        size_t bytes_to_read = buffer_size;
        if (read_circular_buffer(read_buffer, (unsigned char *)audio_data, &bytes_to_read) < 0) {
            fprintf(stderr, "Erro ao ler do buffer circular\n");
            free(audio_data);
            return NULL;
        }

        // Enviar os dados para a reprodução
        if (snd_pcm_writei(pcm_handle, audio_data, bytes_to_read / 2) < 0) {
            fprintf(stderr, "Erro ao escrever no dispositivo de reprodução: %s\n", snd_strerror(snd_pcm_writei(pcm_handle, audio_data, bytes_to_read / 2)));
            free(audio_data);
            return NULL;
        }
    }

    free(audio_data);
    return NULL;
}

// Função para liberar recursos da reprodução
void close_playback(snd_pcm_t *pcm_handle) {
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}
