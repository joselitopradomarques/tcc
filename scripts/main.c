#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "capture.h"

// Declare a constante externa UPDATE_BUFFER_SIZE
extern const size_t UPDATE_BUFFER_SIZE;

typedef struct {
    snd_pcm_t *pcm_handle;
    CircularBuffer *write_buffer;
    CircularBuffer *read_buffer;
} CaptureThreadArgs;

// Função para capturar áudio em uma thread
void *capture_audio_thread(void *args) {
    CaptureThreadArgs *capture_args = (CaptureThreadArgs *)args;
    snd_pcm_t *pcm_handle = capture_args->pcm_handle;
    CircularBuffer *write_buffer = capture_args->write_buffer;
    CircularBuffer *read_buffer = capture_args->read_buffer;

    capture_audio_loop(pcm_handle, write_buffer, read_buffer);

    return NULL;
}

int main() {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    CircularBuffer write_buffer, read_buffer;
    size_t circular_buffer_size = UPDATE_BUFFER_SIZE * 1024;  // Tamanho do buffer circular baseado no UPDATE_BUFFER_SIZE

    // Inicializa buffers circulares
    if (init_circular_buffer(&write_buffer, circular_buffer_size) < 0 || init_circular_buffer(&read_buffer, circular_buffer_size) < 0) {
        fprintf(stderr, "Erro ao inicializar buffer circular\n");
        return -1;
    }

    // Configura captura de áudio
    if (configure_capture(&pcm_handle, &params) < 0) {
        fprintf(stderr, "Erro ao configurar captura de áudio\n");
        return -1;
    }

    // Estrutura para passar parâmetros para a thread
    CaptureThreadArgs capture_args;
    capture_args.pcm_handle = pcm_handle;
    capture_args.write_buffer = &write_buffer;
    capture_args.read_buffer = &read_buffer;

    // Criação da thread de captura de áudio
    pthread_t capture_thread;
    if (pthread_create(&capture_thread, NULL, capture_audio_thread, &capture_args) != 0) {
        fprintf(stderr, "Erro ao criar a thread de captura de áudio\n");
        return -1;
    }

    // Aguardar a thread de captura terminar
    if (pthread_join(capture_thread, NULL) != 0) {
        fprintf(stderr, "Erro ao esperar a thread de captura de áudio\n");
        return -1;
    }

    // Fecha captura e libera buffers
    close_capture(pcm_handle, &write_buffer, &read_buffer);

    return 0;
}
