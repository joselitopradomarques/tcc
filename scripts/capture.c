#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "capture.h"

const size_t UPDATE_BUFFER_SIZE = 1024;  // Tamanho do buffer de captura

// Função para configurar a captura de áudio
int configure_capture(snd_pcm_t **pcm_handle, snd_pcm_hw_params_t **params) {
    int rc;
    unsigned int rate = SAMPLE_RATE;
    snd_pcm_uframes_t buffer_size = UPDATE_BUFFER_SIZE;
    int periods = 2;

    printf("Iniciando configuração ALSA...\n");

    rc = snd_pcm_open(pcm_handle, DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao abrir o dispositivo %s: %s\n", DEVICE, snd_strerror(rc));
        return -1;
    }
    printf("Dispositivo PCM aberto com sucesso: %s\n", DEVICE);

    snd_pcm_hw_params_malloc(params);
    snd_pcm_hw_params_any(*pcm_handle, *params);

    rc = snd_pcm_hw_params_set_access(*pcm_handle, *params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar acesso intercalado: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_format(*pcm_handle, *params, FORMAT);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar formato: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_channels(*pcm_handle, *params, CHANNELS);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar canais: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_rate_near(*pcm_handle, *params, &rate, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar taxa de amostragem: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_period_size_near(*pcm_handle, *params, &buffer_size, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar tamanho do frame: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params_set_periods_near(*pcm_handle, *params, &periods, 0);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar número de períodos: %s\n", snd_strerror(rc));
        return -1;
    }

    rc = snd_pcm_hw_params(*pcm_handle, *params);
    if (rc < 0) {
        fprintf(stderr, "Erro ao configurar os parâmetros de hardware: %s\n", snd_strerror(rc));
        return -1;
    }

    snd_pcm_hw_params_free(*params);
    return 0;
}

// Função para capturar áudio indefinidamente com buffer circular
void capture_audio_loop(snd_pcm_t *pcm_handle, CircularBuffer *write_buffer, CircularBuffer *read_buffer) {
    int rc;
    snd_pcm_uframes_t buffer_size = UPDATE_BUFFER_SIZE;

    short *temp_buffer = malloc(buffer_size * sizeof(short) * CHANNELS);
    if (temp_buffer == NULL) {
        perror("Erro ao alocar memória para buffer temporário");
        return;
    }

    printf("Iniciando captura de áudio indefinidamente. Pressione Ctrl + C para interromper...\n");

    while (1) {
        int frames = snd_pcm_readi(pcm_handle, temp_buffer, buffer_size);
        if (frames < 0) {
            fprintf(stderr, "Erro de captura: %s\n", snd_strerror(frames));
            break;
        }

        // Escreve os dados lidos no buffer circular de escrita
        if (write_circular_buffer(write_buffer, temp_buffer, frames * CHANNELS) < 0) {
            fprintf(stderr, "Erro ao escrever no buffer circular de escrita\n");
            break;
        }

        // Lê os dados do buffer circular de leitura para processamento
        short *process_buffer = malloc(frames * CHANNELS * sizeof(short));
        if (read_circular_buffer(read_buffer, process_buffer, frames * CHANNELS) == 0) {
            // Aqui você pode aplicar o filtro ou outro processamento
            // Exemplo: apply_filter(process_buffer, frames);
            printf("Processando %d frames\n", frames);
        }

        free(process_buffer);
    }

    free(temp_buffer);
}

// Função para fechar a captura de áudio
void close_capture(snd_pcm_t *pcm_handle, CircularBuffer *write_buffer, CircularBuffer *read_buffer) {
    snd_pcm_close(pcm_handle);
    free_circular_buffer(write_buffer);
    free_circular_buffer(read_buffer);
    printf("Captura de áudio finalizada.\n");
}

// Função para inicializar o buffer circular
int init_circular_buffer(CircularBuffer *circular_buffer, size_t size) {
    circular_buffer->buffer = malloc(size * sizeof(short));
    if (circular_buffer->buffer == NULL) {
        return -1;
    }
    circular_buffer->size = size;
    circular_buffer->start = 0;
    circular_buffer->end = 0;
    circular_buffer->count = 0;
    return 0;
}

// Função para liberar o buffer circular
void free_circular_buffer(CircularBuffer *circular_buffer) {
    free(circular_buffer->buffer);
}

// Função para escrever no buffer circular
int write_circular_buffer(CircularBuffer *circular_buffer, short *data, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        circular_buffer->buffer[circular_buffer->end] = data[i];
        circular_buffer->end = (circular_buffer->end + 1) % circular_buffer->size;

        if (circular_buffer->count == circular_buffer->size) {
            circular_buffer->start = (circular_buffer->start + 1) % circular_buffer->size; // Sobrescreve os dados mais antigos
        } else {
            circular_buffer->count++;
        }
    }

    // Exibe uma mensagem com o número de frames e o tamanho do buffer
    printf("Um novo buffer de %zu frames foi adicionado ao buffer circular de tamanho %zu.\n", size, circular_buffer->size);

    return 0;
}

// Função para ler do buffer circular
int read_circular_buffer(CircularBuffer *circular_buffer, short *data, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) {
        if (circular_buffer->count == 0) {
            return -1; // Buffer vazio
        }
        data[i] = circular_buffer->buffer[circular_buffer->start];
        circular_buffer->start = (circular_buffer->start + 1) % circular_buffer->size;
        circular_buffer->count--;
    }
    return 0;
}